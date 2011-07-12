GCC := avr-gcc
OBJCOPY := avr-objcopy

#CFLAGS := -mint8
CFLAGS  := -O2 -mtiny-stack
LDFLAGS :=

main:
	$(GCC) -g -mmcu=atmega128 $(CFLAGS) -c main.c -o main.o
	$(GCC) -g -mmcu=atmega128 main.o -o main.elf
	$(OBJCOPY) -j .text -j .data -O ihex main.elf main.hex

flash: main
	avrdude -p m128 -c usbasp -U flash:w:main.hex

clean:
	rm -f *.o *.hex *.elf