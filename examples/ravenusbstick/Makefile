all:
	${MAKE} TARGET=avr-ravenusb NOAVRSIZE=1 -f Makefile.ravenusbstick ravenusbstick.elf
	avr-objcopy -O ihex -R .signature -R .fuse -R .eeprom ravenusbstick.elf ravenusbstick.hex
	avr-objcopy -O ihex -j .eeprom ravenusbstick.elf ravenusbstick-eeprom.hex
	avr-size ravenusbstick.elf

clean:
	${MAKE} -f Makefile.ravenusbstick clean
	rm -rf obj_avr-ravenusb
	rm symbols.c symbols.h ravenusbstick.elf ravenusbstick.hex
