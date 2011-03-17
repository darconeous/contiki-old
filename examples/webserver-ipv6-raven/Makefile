ifndef WEBDIR
 OUTFILE=webserver6
else
 OUTFILE=webserver6-$(WEBDIR)
endif

DEVICE=m1284p
AVRDUDE=avrdude -V -c usbtiny -p ${DEVICE} -P usb


all:
	make -f Makefile.webserver TARGET=avr-raven webserver6.elf
#can't just rename, webserver6 may be open in debugger
	@if [ $(OUTFILE) != "webserver6" ]; then cp webserver6.elf $(OUTFILE).elf;fi
	avr-objcopy -O ihex -R .eeprom -R .fuse -R .signature $(OUTFILE).elf $(OUTFILE).hex
	avr-objcopy -O ihex -j .eeprom  $(OUTFILE).elf $(OUTFILE)-eeprom.hex
	avr-size  $(OUTFILE).elf

burn: all
	$(AVRDUDE) -U flash:w:webserver6.hex

burn-eeprom: all
	$(AVRDUDE) -U eeprom:w:webserver6-eeprom.hex


clean:
	make -f Makefile.webserver TARGET=avr-raven clean
# Delete httpd-fsdata.c only if your toolchain includes perl
#	rm ../../platform/avr-raven/apps/raven-webserver/httpd-fsdata.c
	rm symbols.c symbols.h webserver6.elf webserver6.hex
	rm -rf obj_avr-raven
