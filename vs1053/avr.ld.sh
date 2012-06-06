#!/bin/bash

MCU=$1

SCR=$(avr-gcc -Wl,-verbose -mmcu=$MCU 2>/dev/null | gawk '{s=$1 " " $2 " " $3; if (s == "opened script file") {print $4} }')

sed -f avr.ld.sed $SCR > avr.ld

echo avr.ld