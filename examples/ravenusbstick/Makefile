
DEVICE=at90usb1287
AVRDUDE=avrdude -p ${DEVICE} -P usb
DFUPROGRAMMER=dfu-programmer

AVRDUDEDEVICE=usb1287
AVRDUDEPROGRAMMER=avr109
AVRDUDEPORT=/dev/tty.usbmodem*
AVRDUDEPROGRAMMER=dragon_jtag
AVRDUDEPORT=usb
AVRDUDE=avrdude -c ${AVRDUDEPROGRAMMER} -p ${AVRDUDEDEVICE} -P ${AVRDUDEPORT}

all:
	make TARGET=avr-ravenusb -f Makefile.ravenusbstick ravenusbstick.elf
	avr-objcopy -O ihex -R .signature -R .fuse -R .eeprom ravenusbstick.elf ravenusbstick.hex
	avr-objcopy -O ihex -j .eeprom ravenusbstick.elf ravenusbstick-eeprom.hex
	avr-size ravenusbstick.elf

dfu-burn: all ravenusbstick.hex
	$(DFUPROGRAMMER) ${DEVICE} flash --debug 100 ravenusbstick.hex

dfu-erase:
	$(DFUPROGRAMMER) ${DEVICE} erase --debug 100

dfu-start:
	$(DFUPROGRAMMER) ${DEVICE} start

burn: all
	$(AVRDUDE) -U flash:w:ravenusbstick.hex

burn-eeprom: all
	$(AVRDUDE) -U eeprom:w:ravenusbstick-eeprom.hex

hextar:
	tar cvzf ravenusbstick.tar.gz ravenusbstick.hex ravenusbstick-eeprom.hex

clean:
	make -f Makefile.ravenusbstick clean
	rm -rf obj_avr-ravenusb
	rm symbols.c symbols.h ravenusbstick.elf ravenusbstick.hex
