#ifndef _VS1053_H_
#define _VS1053_H_

typedef enum vs1053_sci_reg {
    VS1053_SCI_REG_MODE,
    VS1053_SCI_REG_STATUS,
    VS1053_SCI_REG_BASS,
    VS1053_SCI_REG_CLOCKF,
    VS1053_SCI_REG_DECODE_TIME,
    VS1053_SCI_REG_AUDATA,
    VS1053_SCI_REG_WRAM,
    VS1053_SCI_REG_WRAMADDR,
    VS1053_SCI_REG_HDAT0,
    VS1053_SCI_REG_HDAT1,
    VS1053_SCI_REG_AIADDR,
    VS1053_SCI_REG_VOL,
    VS1053_SCI_REG_AICTRL0,
    VS1053_SCI_REG_AICTRL1,
    VS1053_SCI_REG_AICTRL2,
    VS1053_SCI_REG_AICTRL3,

    VS1053_SCI_REG_QTY
} vs1053_sci_reg_t;

// SCI MODE REG bits
enum vs1053_sci_reg_mode {
    VS1053_SCI_REG_MODE_DIFF,
    VS1053_SCI_REG_MODE_LAYER12,
    VS1053_SCI_REG_MODE_RESET,
    VS1053_SCI_REG_MODE_OUTOFWAV,
    VS1053_SCI_REG_MODE_EARSPEAKER_LO,
    VS1053_SCI_REG_MODE_TESTS,
    VS1053_SCI_REG_MODE_STREAM,
    VS1053_SCI_REG_MODE_EARSPEAKER_HI,
    VS1053_SCI_REG_MODE_DACT,
    VS1053_SCI_REG_MODE_SDIORD,
    VS1053_SCI_REG_MODE_SDISHARE,
    VS1053_SCI_REG_MODE_SDINEW,
    VS1053_SCI_REG_MODE_ADPCM,
    VS1053_SCI_REG_MODE_ADCPM_HP,
    VS1053_SCI_REG_MODE_LINE_IN,
    VS1053_SCI_REG_MODE_CLK_RANGE,
};

void vs1053_setup(void);

void vs1053_write_register(vs1053_sci_reg_t reg, unsigned int value);
void vs1053_read_register(vs1053_sci_reg_t reg, unsigned int *value);

void vs1053_play(const unsigned char *data, unsigned int len);
void vs1053_play_progmem(const unsigned char *data, unsigned int len);

void vs1053_play_sine(unsigned char pitch);

#endif
