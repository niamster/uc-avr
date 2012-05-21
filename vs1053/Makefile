GCC := avr-gcc
OBJCOPY := avr-objcopy
SIZE := avr-size

F_CPU := 16000000UL
BAUD  := 38400

CFLAGS  :=
#CFLAGS  += -mint8
CFLAGS  += -O2
CFLAGS  += -DF_CPU=$(F_CPU) -DBAUD=$(BAUD)
CFLAGS  += -ffunction-sections -fdata-sections
CFLAGS  += -I./

LDFLAGS := -gc-sections

EXT_LIB := /projects/avr

UART    := $(EXT_LIB)/uart
SH      := $(EXT_LIB)/sh
SPI     := $(EXT_LIB)/spi

CFLAGS  += -I$(EXT_LIB)
CFLAGS  += -I$(EXT_LIB)
CFLAGS  += -I$(EXT_LIB)

VPATH = vs1053:$(UART):$(SH):$(SPI):

OBJDIR ?= obj
OUTDIR ?= out
SRC = main.c vs1053.c spi.c uart.c sh.c
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
