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
    VS1053_SCI_REG_MODE_CANCEL,
    VS1053_SCI_REG_MODE_EARSPEAKER_LO,
    VS1053_SCI_REG_MODE_TESTS,
    VS1053_SCI_REG_MODE_STREAM,
    VS1053_SCI_REG_MODE_EARSPEAKER_HI,
    VS1053_SCI_REG_MODE_DACT,
    VS1053_SCI_REG_MODE_SDIORD,
    VS1053_SCI_REG_MODE_SDISHARE,
    VS1053_SCI_REG_MODE_SDINEW,
    VS1053_SCI_REG_MODE_ADPCM,
    VS1053_SCI_REG_MODE_RESERVED,
    VS1053_SCI_REG_MODE_LINE1,
    VS1053_SCI_REG_MODE_CLK_RANGE,
};

/* dBm below 0 */
typedef uint16_t idBm_t;

/** Callback function used to provide audio data
 *  data and len refer to a portion of audio data
 *  priv is opaque pointer to calee private data
 *  return 1 if that was the last portion of data to play, 0 otherwise
 */
typedef uint8_t (*vs1053_audio_feeder_t)(uint8_t **data, uint16_t *len, void *priv);

void vs1053_setup(void);

void vs1053_write_register(vs1053_sci_reg_t reg, uint16_t value);
void vs1053_read_register(vs1053_sci_reg_t reg, uint16_t *value);

void vs1053_play(vs1053_audio_feeder_t feeder, void *priv);
void vs1053_play_ram(const uint8_t *data, uint16_t len);
void vs1053_play_pgm(const uint8_t *data, uint16_t len);

void vs1053_set_volume(idBm_t left, idBm_t right);

void vs1053_play_sine(uint8_t pitch);

#endif
