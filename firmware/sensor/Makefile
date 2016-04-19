all: firmware.hex firmware-debug.hex

MCU=attiny25

%.elf: %.cpp
	avr-g++ -Wall -Wextra -mmcu=${MCU} $< -o $@

%-debug.elf: %.cpp
	avr-g++ -Wall -Wextra -mmcu=${MCU} -DDEBUG_MODE $< -o $@

%.hex: %.elf
	avr-objcopy -O ihex $< $@

.PHONY: all
