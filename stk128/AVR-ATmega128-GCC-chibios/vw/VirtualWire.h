// VirtualWire.h
//
// Virtual Wire implementation for Arduino
// See the README file in this directory fdor documentation
//
// Author: Mike McCauley (mikem@open.com.au)
// Copyright (C) 2008 Mike McCauley
// $Id: VirtualWire.h,v 1.4 2012/01/10 22:21:03 mikem Exp mikem $

#ifndef VirtualWire_h
#define VirtualWire_h

#include <stdlib.h>

// Maximum number of bytes in a message, counting the byte count and FCS
#define VW_MAX_MESSAGE_LEN 30

// The maximum payload length
#define VW_MAX_PAYLOAD VW_MAX_MESSAGE_LEN-3

// The size of the receiver ramp. Ramp wraps modulu this number
#define VW_RX_RAMP_LEN 160

// Number of samples per bit
#define VW_RX_SAMPLES_PER_BIT 8

// Ramp adjustment parameters
// Standard is if a transition occurs before VW_RAMP_TRANSITION (80) in the ramp,
// the ramp is retarded by adding VW_RAMP_INC_RETARD (11)
// else by adding VW_RAMP_INC_ADVANCE (29)
// If there is no transition it is adjusted by VW_RAMP_INC (20)
#define VW_RAMP_INC (VW_RX_RAMP_LEN/VW_RX_SAMPLES_PER_BIT)
#define VW_RAMP_TRANSITION VW_RX_RAMP_LEN/2
#define VW_RAMP_ADJUST 9
#define VW_RAMP_INC_RETARD (VW_RAMP_INC-VW_RAMP_ADJUST)
#define VW_RAMP_INC_ADVANCE (VW_RAMP_INC+VW_RAMP_ADJUST)

// Outgoing message bits grouped as 6-bit words
// 36 alternating 1/0 bits, followed by 12 bits of start symbol
// Followed immediately by the 4-6 bit encoded byte count,
// message buffer and 2 byte FCS
// Each byte from the byte count on is translated into 2x6-bit words
// Caution, each symbol is transmitted LSBit first,
// but each byte is transmitted high nybble first
#define VW_HEADER_LEN 8

// Initialise the VirtualWire software, to operate at speed bits per second
// Call this one in your setup() after any vw_set_* calls
// Must call vw_rx_start() before you will get any messages
void vw_setup(uint16_t speed);

// Start the Phase Locked Loop listening to the receiver
// Must do this before you can receive any messages
// When a message is available (good checksum or not), vw_have_message();
// will return true.
void vw_rx_start(void);

// Stop the Phase Locked Loop listening to the receiver
// No messages will be received until vw_rx_start() is called again
// Saves interrupt processing cycles
void vw_rx_stop(void);

// Block until a message is available
void vw_wait_rx(void);

// Returns true if an unread message is available
uint8_t vw_have_message(void);

// If a message is available (good checksum or not), copies
// up to *len octets to buf.
// Returns true if there was a message and the checksum was good
uint8_t vw_get_message(uint8_t* buf, uint8_t* len);

#endif
