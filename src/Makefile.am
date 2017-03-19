#
# This file is part of Antz.
# Copyright (C) 2017  Albert Farres
#
# Antz is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Antz is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Antz.  If not, see <http://www.gnu.org/licenses/>.
#
AUTOMAKE_OPTIONS = foreign

PROGRAM        := antz
CXXSOURCES     := driver.cpp
CSOURCES       :=

OBJECTS        := $(patsubst %.cpp,%.o,$(CXXSOURCES))
OBJECTS        += $(patsubst %.c,%.o,$(CSOURCES))

#all: $(PROGRAM).elf lst text eeprom
all-am: $(PROGRAM).hex

if WITH_AVRDUDE
#AVRDUDE_OPTS  = -U lfuse:w:0xe2:m # program CKDIV8
#AVRDUDE_OPTS  = -U lfuse:w:0x62:m # unprogram CKDIV8
upload: $(PROGRAM).hex
	$(AVRDUDE) $(AVRDUDE_PROGRAMMER) -p $(AVRDUDE_MCU) -U flash:w:$^ $(AVRDUDE_OPTS)
endif

DEPDIR   = .deps
$(eval include $(srcdir)/deps.mk)

$(DEPDIR)/%.d: %.cpp %.hpp
	$(CXX) $(CPPFLAGS) -DNDEBUG $(CXXFLAGS) -mmcu=$(MCU_TARGET) -MM -MF $@ $<
$(DEPDIR)/%.d: %.cpp
	$(CXX) $(CPPFLAGS) -DNDEBUG $(CXXFLAGS) -mmcu=$(MCU_TARGET) -MM -MF $@ $<
$(DEPDIR)/%.d: %.c %.h
	$(CC) $(CPPFLAGS) -DNDEBUG $(CFLAGS) -mmcu=$(MCU_TARGET) -MM -MF $@ $<
$(DEPDIR)/%.d: %.c
	$(CC) $(CPPFLAGS) -DNDEBUG $(CFLAGS) -mmcu=$(MCU_TARGET) -MM -MF $@ $<


-include $(DEPS)
%.o: %.cpp $(DEPDIR)/%.d
	$(CXX) $(CPPFLAGS) -DNDEBUG $(CXXFLAGS) -ffunction-sections -fdata-sections -mmcu=$(MCU_TARGET) -c -o $@ $<
%.o: %.c $(DEPDIR)/%.d
	$(CC) $(CPPFLAGS) -DNDEBUG $(CFLAGS) -ffunction-sections -fdata-sections -mmcu=$(MCU_TARGET) -c -o $@ $<

$(PROGRAM).elf: $(OBJECTS)
	$(CXX) $(CXXFLAGS) $(LDFLAGS) -mmcu=$(MCU_TARGET) -Wl,--gc-sections,-Map,$(PROGRAM).map -o $@ $^ $(LIBS)

clean:
	@rm -vrf .deps
	@rm -vrf *.o $(PROGRAM).elf *.eps *.png *.pdf *.bak
	@rm -vrf *.lst *.map $(EXTRA_CLEAN_FILES)

lst:  $(PROGRAM).lst

%.lst: %.elf
	$(OBJDUMP) -h -S $< > $@

# Rules for building the .text rom images

text: hex bin srec

hex:  $(PROGRAM).hex
bin:  $(PROGRAM).bin
srec: $(PROGRAM).srec

%.hex: %.elf
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

%.srec: %.elf
	$(OBJCOPY) -j .text -j .data -O srec $< $@

%.bin: %.elf
	$(OBJCOPY) -j .text -j .data -O binary $< $@

# Rules for building the .eeprom rom images

eeprom: ehex ebin esrec

ehex:  $(PROGRAM)_eeprom.hex
ebin:  $(PROGRAM)_eeprom.bin
esrec: $(PROGRAM)_eeprom.srec

%_eeprom.hex: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O ihex $< $@ \
	  || { echo empty $@ not generated; exit 0; }

%_eeprom.srec: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O srec $< $@ \
	  || { echo empty $@ not generated; exit 0; }

%_eeprom.bin: %.elf
	$(OBJCOPY) -j .eeprom --change-section-lma .eeprom=0 -O binary $< $@ \
	  || { echo empty $@ not generated; exit 0; }

EXTRA_CLEAN_FILES       = *.hex *.bin *.srec

