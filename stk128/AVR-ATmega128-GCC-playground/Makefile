GCC := avr-gcc
OBJCOPY := avr-objcopy

#CFLAGS := -mint8
CFLAGS  := -O2 -mtiny-stack
LDFLAGS :=

F_CPU := 7372800UL

main:
	$(GCC) -g -mmcu=atmega128 $(CFLAGS) -DF_CPU=$(F_CPU) -c main.c -o main.o
	$(GCC) -g -mmcu=atmega128 $(CFLAGS) -DF_CPU=$(F_CPU) -c uart.c -o uart.o
	$(GCC) -g -mmcu=atmega128 main.o uart.o -o main.elf
	$(OBJCOPY) -j .text -j .data -O ihex main.elf main.hex

flash: main
	avrdude -p m128 -c usbasp -U flash:w:main.hex

clean:
	rm -f *.o *.hex *.elf