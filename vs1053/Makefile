GCC := avr-gcc
OBJCOPY := avr-objcopy
SIZE := avr-size

F_CPU := 16000000UL
BAUD  := 38400

CFLAGS  := -Wall -Wno-pointer-sign -Wno-unused-function
#CFLAGS  += -mint8
CFLAGS  += -O2
CFLAGS  += -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)
CFLAGS  += -DUART_RX_INTERRUPT
CFLAGS  += -DVS1053_XCS_SHARED -DVS1053_SPI_TRANSACTIONS -DVS1053_F_CLKI_KHZ=43008UL -DVS1053_F_XTAL_KHZ=12288UL
# CFLAGS  += -DVS1053_SPI_HIGHEST_SPEED
CFLAGS  += -ffunction-sections -fdata-sections
CFLAGS  += -D__OPTIMIZE__
CFLAGS  += -DWCLOCK_FREQUENCY=1000UL
CFLAGS  += -I./

LDFLAGS := -Wl,-gc-sections -Tavr5.x

EXT_LIB := /projects/avr

CFLAGS  += -I$(EXT_LIB)

VPATH   := vs1053:

OBJDIR  ?= obj
OUTDIR  ?= out

SRC	    := main.c vs1053.c

include ../uart/uart.mk
include ../sh/sh.mk
include ../spi/spi.mk
include ../wclock/wclock.mk
include ../mi/mi.mk

OBJ = $(SRC:%.c=$(OBJDIR)/%.o)

# Create object files directory
$(shell mkdir $(OBJDIR) 2>/dev/null)
$(shell mkdir $(OUTDIR) 2>/dev/null)

main: $(OBJ)
	$(GCC) -g -mmcu=atmega16 $(LDFLAGS) $^ -o $(OUTDIR)/main.elf
	$(OBJCOPY) -j .text -j .data -O ihex $(OUTDIR)/main.elf $(OUTDIR)/main.hex
	$(SIZE) $(OUTDIR)/main.elf

$(OBJDIR)/%.o: %.c
	$(GCC) -g -mmcu=atmega16 $(CFLAGS) -c $< -o $@

program: main
	avrdude -p m16 -c usbasp -U flash:w:$(OUTDIR)/main.hex

fuses:
	avrdude -p m16 -c usbasp -U lfuse:w:0xff:m -U hfuse:w:0xc9:m

clean:
	rm -rf $(OBJDIR) $(OUTDIR)
	mkdir $(OBJDIR)
	mkdir $(OUTDIR)
