GCC := avr-gcc
OBJCOPY := avr-objcopy

#CFLAGS := -mint8
CFLAGS  := -O2 -mtiny-stack
LDFLAGS :=

F_CPU := 16000000UL

main:
	$(GCC) -g -mmcu=atmega16 $(CFLAGS) -DF_CPU=$(F_CPU) -c main.c -o main.o
	$(GCC) -g -mmcu=atmega16 $(CFLAGS) -DF_CPU=$(F_CPU) -c uart.c -o uart.o
	$(GCC) -g -mmcu=atmega16 main.o uart.o -o main.elf
	$(OBJCOPY) -j .text -j .data -O ihex main.elf main.hex

flash: main
	avrdude -p m16 -c usbasp -U flash:w:main.hex

clean:
	rm -f *.o *.hex *.elf
