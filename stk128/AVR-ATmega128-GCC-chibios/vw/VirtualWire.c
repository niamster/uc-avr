// VirtualWire.cpp
//
// Virtual Wire implementation for Arduino
// See the README file in this directory fdor documentation
//
// Changes:
// 1.5 2008-05-25: fixed a bug that could prevent messages with certain
//  bytes sequences being received (0 message start detected)
// 1.6 2011-09-10: Patch from David Bath to prevent unconditional reenabling of the receiver
//  at end of transmission.
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2008 Mike McCauley
// $Id: VirtualWire.cpp,v 1.6 2012/01/10 22:21:03 mikem Exp mikem $

#include <util/crc16.h>
#include <string.h>

#include "ch.h"
#include "hal.h"

#include "vw/VirtualWire.h"

// Current receiver sample
static uint8_t vw_rx_sample = 0;

// Last receiver sample
static uint8_t vw_rx_last_sample = 0;

// PLL ramp, varies between 0 and VW_RX_RAMP_LEN-1 (159) over
// VW_RX_SAMPLES_PER_BIT (8) samples per nominal bit time.
// When the PLL is synchronised, bit transitions happen at about the
// 0 mark.
static uint8_t vw_rx_pll_ramp = 0;

// This is the integrate and dump integral. If there are <5 0 samples in the PLL cycle
// the bit is declared a 0, else a 1
static uint8_t vw_rx_integrator = 0;

// Flag indictate if we have seen the start symbol of a new message and are
// in the processes of reading and decoding it
static uint8_t vw_rx_active = 0;

// Flag to indicate that a new message is available
static volatile uint8_t vw_rx_done = 0;

// Flag to indicate the receiver PLL is to run
static uint8_t vw_rx_enabled = 0;

// Last 12 bits received, so we can look for the start symbol
static uint16_t vw_rx_bits = 0;

// How many bits of message we have received. Ranges from 0 to 12
static uint8_t vw_rx_bit_count = 0;

// The incoming message buffer
static uint8_t vw_rx_buf[VW_MAX_MESSAGE_LEN];

// The incoming message expected length
static uint8_t vw_rx_count = 0;

// The incoming message buffer length received so far
static volatile uint8_t vw_rx_len = 0;

// Number of bad messages received and dropped due to bad lengths
static uint8_t vw_rx_bad = 0;

// Number of good messages received
static uint8_t vw_rx_good = 0;

// 4 bit to 6 bit symbol converter table
// Used to convert the high and low nybbles of the transmitted data
// into 6 bit symbols for transmission. Each 6-bit symbol has 3 1s and 3 0s
// with at most 2 consecutive identical bits
static const uint8_t symbols[] = {
    0xd,  0xe,  0x13, 0x15, 0x16, 0x19, 0x1a, 0x1c,
    0x23, 0x25, 0x26, 0x29, 0x2a, 0x2c, 0x32, 0x34
};

// Compute CRC over count bytes.
// This should only be ever called at user level, not interrupt level
static uint16_t vw_crc(uint8_t *ptr, uint8_t count)
{
    uint16_t crc = 0xffff;

    while (count-- > 0)
        crc = _crc_ccitt_update(crc, *ptr++);
    return crc;
}

// Convert a 6 bit encoded symbol into its 4 bit decoded equivalent
static uint8_t vw_symbol_6to4(uint8_t symbol)
{
    uint8_t i;

    // Linear search :-( Could have a 64 byte reverse lookup table?
    for (i = 0; i < 16; i++)
        if (symbol == symbols[i])
            return i;

    return 0; // Not found
}

// Called 8 times per bit period
// Phase locked loop tries to synchronise with the transmitter so that bit
// transitions occur at about the time vw_rx_pll_ramp is 0;
// Then the average is computed over each bit period to deduce the bit value
static void vw_pll(void)
{
    // Integrate each sample
    if (vw_rx_sample)
        vw_rx_integrator++;

    if (vw_rx_sample != vw_rx_last_sample) {
        // Transition, advance if ramp > 80, retard if < 80
        vw_rx_pll_ramp += ((vw_rx_pll_ramp < VW_RAMP_TRANSITION)
                ? VW_RAMP_INC_RETARD
                : VW_RAMP_INC_ADVANCE);
        vw_rx_last_sample = vw_rx_sample;
    } else {
        // No transition
        // Advance ramp by standard 20 (== 160/8 samples)
        vw_rx_pll_ramp += VW_RAMP_INC;
    }

    if (vw_rx_pll_ramp >= VW_RX_RAMP_LEN) {
        // Add this to the 12th bit of vw_rx_bits, LSB first
        // The last 12 bits are kept
        vw_rx_bits >>= 1;

        // Check the integrator to see how many samples in this cycle were high.
        // If < 5 out of 8, then its declared a 0 bit, else a 1;
        if (vw_rx_integrator >= 5)
            vw_rx_bits |= 0x800;

        vw_rx_pll_ramp -= VW_RX_RAMP_LEN;
        vw_rx_integrator = 0; // Clear the integral for the next cycle

        if (vw_rx_active) {
            // We have the start symbol and now we are collecting message bits,
            // 6 per symbol, each which has to be decoded to 4 bits
            if (++vw_rx_bit_count >= 12) {
                // Have 12 bits of encoded message == 1 byte encoded
                // Decode as 2 lots of 6 bits into 2 lots of 4 bits
                // The 6 lsbits are the high nybble
                uint8_t this_byte =
                    (vw_symbol_6to4(vw_rx_bits & 0x3f)) << 4
                    | vw_symbol_6to4(vw_rx_bits >> 6);

                // The first decoded byte is the byte count of the following message
                // the count includes the byte count and the 2 trailing FCS bytes
                // REVISIT: may also include the ACK flag at 0x40
                if (vw_rx_len == 0) {
                    // The first byte is the byte count
                    // Check it for sensibility. It cant be less than 4, since it
                    // includes the bytes count itself and the 2 byte FCS
                    vw_rx_count = this_byte;
                    if (vw_rx_count < 4 || vw_rx_count > VW_MAX_MESSAGE_LEN) {
                        // Stupid message length, drop the whole thing
                        vw_rx_active = 0;
                        vw_rx_bad++;
                        return;
                    }
                }
                vw_rx_buf[vw_rx_len++] = this_byte;

                if (vw_rx_len >= vw_rx_count) {
                    // Got all the bytes now
                    vw_rx_active = 0;
                    vw_rx_good++;
                    vw_rx_done = 1; // Better come get it before the next one starts
                }
                vw_rx_bit_count = 0;
            }
        } else if (vw_rx_bits == 0xb38) { // Not in a message, see if we have a start symbol
            // Have start symbol, start collecting message
            vw_rx_active = 1;
            vw_rx_bit_count = 0;
            vw_rx_len = 0;
            vw_rx_done = 0; // Too bad if you missed the last message
        }
    }
}

// Speed is in bits per sec RF rate
void vw_setup(uint16_t speed)
{
    // Calculate the OCR1A overflow count based on the required bit speed
    // and CPU clock rate
    uint16_t ocr1a = (F_CPU / 8UL) / speed;

    // Set up timer1 for a tick every 62.50 microseconds
    // for 2000 bits per sec
    TCCR1A = 0;
    TCCR1B = _BV(WGM12) | _BV(CS10);
    // Caution: special procedures for setting 16 bit regs
    OCR1A = ocr1a;
    // Enable interrupt
#ifdef TIMSK1
    // atmega168
    TIMSK1 |= _BV(OCIE1A);
#else
    // others
    TIMSK |= _BV(OCIE1A);
#endif
}

// Enable the receiver. When a message becomes available, vw_rx_done flag
// is set, and vw_wait_rx() will return.
void vw_rx_start(void)
{
    if (!vw_rx_enabled) {
        vw_rx_enabled = 1;
        vw_rx_active = 0; // Never restart a partial message
    }
}

// Disable the receiver
void vw_rx_stop(void)
{
    vw_rx_enabled = 0;
}

// Wait for the receiver to get a message
// Busy-wait loop until the ISR says a message is available
// can then call vw_get_message()
void vw_wait_rx(void)
{
    while (!vw_rx_done);
}

/* Do this in realtime, no time for chibiOS in this handler ... */

// This is the interrupt service routine called when timer1 overflows
// Its job is to output the next bit from the transmitter (every 8 calls)
// and to call the PLL code if the receiver is enabled
//ISR(SIG_OUTPUT_COMPARE1A)
/* CH_IRQ_HANDLER(TIMER1_COMPA_vect) */
SIGNAL(TIMER1_COMPA_vect)
{
    /* CH_IRQ_PROLOGUE(); */

    vw_rx_sample = (VW_DIN_PORT >> VW_DIN_BIT)&0x1;

    if (vw_rx_enabled)
        vw_pll();

    /* CH_IRQ_EPILOGUE(); */
}

// Return 1 if there is a message available
uint8_t vw_have_message(void)
{
    return vw_rx_done;
}

// Get the last message received (without byte count or FCS)
// Copy at most *len bytes, set *len to the actual number copied
// Return 1 if there is a message and the FCS is OK
uint8_t vw_get_message(uint8_t* buf, uint8_t* len)
{
    uint8_t rxlen;

    // Message available?
    if (!vw_rx_done)
        return 0;

    // Wait until vw_rx_done is set before reading vw_rx_len
    // then remove bytecount and FCS
    rxlen = vw_rx_len - 3;

    // Copy message (good or bad)
    if (*len > rxlen)
        *len = rxlen;
    memcpy(buf, vw_rx_buf + 1, *len);

    vw_rx_done = 0; // OK, got that message thanks

    // Check the FCS, return goodness
    return (vw_crc(vw_rx_buf, vw_rx_len) == 0xf0b8); // FCS OK?
}
