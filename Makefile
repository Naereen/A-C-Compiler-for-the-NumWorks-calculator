#
# GNU Makefile for the Tiny C Compiler (NWA) app for the Numwoks calculators
#
# See https://github.com/Naereen/A-C-Compiler-for-the-NumWorks-calculator
# See https://yaya-cout.github.io/Nwagyu/guide/help/how-to-install.html
#

# Q ?= @
Q ?=
CC = arm-none-eabi-gcc
NWLINK = npx --yes -- nwlink
LINK_GC = 1
LTO = 1

# objs = $(addprefix output/tinycc.git/,\
#   libtcc.o \
# )

#   eadk_lib.o
objs += $(addprefix output/,\
  storage.o \
  tcc_stubs.o \
  crt_stubs.o \
  icon.o \
  main.o \
)

# Path to your libtcc library
TCC_LIB_DIR := ./src/tinycc.git/

CFLAGS = -std=c99
CFLAGS += $(shell $(NWLINK) eadk-cflags-device)
CFLAGS += -Os -Wall -Wextra -Wvla
# CFLAGS += -ggdb

LDFLAGS = -Wl,--relocatable
LDFLAGS += $(shell $(NWLINK) eadk-ldflags-device)

# Include the TCC library directory
CFLAGS += -I$(TCC_LIB_DIR)
LDFLAGS += -L$(TCC_LIB_DIR)

# This is the embedded *runtime* library, NOT the one we want!
# CLIBS += -l:arm-eabi-libtcc1.a
# LDLIBS += -l:arm-eabi-libtcc1.a

# This should be the good library to include!
CLIBS += -l:arm-eabihf-libtcc.a
LDLIBS += -l:arm-eabihf-libtcc.a
LDLIBS += -l:arm-eabihf-libtcc.a

LDFLAGS += -nostartfiles

# LDFLAGS += --specs=nano.specs # Alternatively, use nano C lib
LDFLAGS += --specs=nosys.specs # Alternatively, use full-fledged newlib

ifeq ($(LINK_GC),1)
CFLAGS += -fdata-sections -ffunction-sections
LDFLAGS += -Wl,-e,main -Wl,-u,eadk_app_name -Wl,-u,eadk_app_icon -Wl,-u,eadk_api_level
LDFLAGS += -Wl,--gc-sections
endif

ifeq ($(LTO),1)
CFLAGS += -flto -fno-fat-lto-objects
CFLAGS += -fwhole-program
CFLAGS += -fvisibility=internal
LDFLAGS += -flinker-output=nolto-rel
endif

.PHONY: build
build: output/tiny-c-compiler.nwa
	ls -larth output/tiny-c-compiler.nwa
	du -b output/tiny-c-compiler.nwa
	file output/tiny-c-compiler.nwa

.PHONY: check
check: output/tiny-c-compiler.bin
	ls -larth output/tiny-c-compiler.bin
	du -b output/tiny-c-compiler.bin
	file output/tiny-c-compiler.bin

.PHONY: elf
elf: output/tiny-c-compiler.elf
	ls -larth output/tiny-c-compiler.elf
	du -b output/tiny-c-compiler.elf
	file output/tiny-c-compiler.elf

.PHONY: run
run: output/tiny-c-compiler.nwa src/test.c
	@echo "INSTALL $<"
# $(Q) $(NWLINK) install-nwa --external-data src/test.c $<
	$(Q) $(NWLINK) install-nwa $<

output/%.bin: output/%.nwa src/test.c
	@echo "BIN     $@"
# $(Q) $(NWLINK) nwa-bin --external-data src/test.c $< $@
	$(Q) $(NWLINK) nwa-bin $< $@

output/%.elf: output/%.nwa src/test.c
	@echo "ELF     $@"
# $(Q) $(NWLINK) nwa-elf --external-data src/test.c $< $@
	$(Q) $(NWLINK) nwa-elf $< $@

output/tiny-c-compiler.nwa: $(objs)
	@echo "LD      $@"
	$(Q) $(CC) $(CFLAGS) $(LDFLAGS) $^ -o $@ -lm $(LDLIBS) -lCMSIS_CORE

output/%.o: src/%.c
	@mkdir -p $(@D)
	@echo "CC      $^"
	$(Q) $(CC) $(CFLAGS) -c $^ -o $@ -lm $(CLIBS)

output/icon.o: src/icon.png
	@echo "ICON    $<"
	$(Q) $(NWLINK) png-icon-o $< $@

.PHONY: clean
clean:
	@echo "CLEAN"
	$(Q) rm -rf output
