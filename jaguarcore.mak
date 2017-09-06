#
# Makefile for Virtual Jaguar core library
#
# by James Hammons
#
# This software is licensed under the GPL v3 or any later version. See the
# file GPLv3 for details. ;-)
#

ifeq ("$(V)","1")
Q :=
else
Q := @
endif

# Cross compilation with MXE
#CROSS = i686-pc-mingw32-

SYSTYPE    := __GCCUNIX__

ifneq "$(CROSS)" ""
SYSTYPE    := __GCCWIN32__
else
OSTYPE := $(shell uname -o)
ifeq "$(OSTYPE)" "Msys"
SYSTYPE    := __GCCWIN32__
endif
endif

# Set vars for libcdio
ifneq "$(shell pkg-config --silence-errors --libs libcdio)" ""
HAVECDIO := -DHAVE_LIB_CDIO
CDIOLIB  := -lcdio
else
HAVECDIO :=
CDIOLIB  :=
endif

CC      := $(CROSS)gcc
LD      := $(CROSS)gcc
AR      := $(CROSS)ar
ARFLAGS := -rs

SDL_CFLAGS = `$(CROSS)sdl-config --cflags`
DEFINES = -D$(SYSTYPE)
GCC_DEPS = -MMD

INCS := -I./src

OBJS := \
	obj/blitter.o      \
	obj/cdintf.o       \
	obj/cdrom.o        \
	obj/crc32.o        \
	obj/dac.o          \
	obj/dsp.o          \
	obj/eeprom.o       \
	obj/event.o        \
	obj/file.o         \
	obj/filedb.o       \
	obj/gpu.o          \
	obj/jagbios.o      \
	obj/jagbios2.o     \
	obj/jagcdbios.o    \
	obj/jagdevcdbios.o \
	obj/jagstub1bios.o \
	obj/jagstub2bios.o \
	obj/jagdasm.o      \
	obj/jaguar.o       \
	obj/jerry.o        \
	obj/joystick.o     \
	obj/log.o          \
	obj/memory.o       \
	obj/memtrack.o     \
	obj/mmu.o          \
	obj/op.o           \
	obj/settings.o     \
	obj/state.o        \
	obj/tom.o          \
	obj/universalhdr.o \
	obj/unzip.o        \
	obj/wavetable.o

# Targets for convenience sake, not "real" targets
.PHONY: clean

all: obj obj/libjaguarcore.a
	@echo "Done!"

obj:
	@mkdir obj

# Library rules (might not be cross-platform compatible)
obj/libjaguarcore.a: $(OBJS) 
	$(Q)$(AR) $(ARFLAGS) obj/libjaguarcore.a $(OBJS)

# Main source compilation (implicit rules)...

obj/%.o: src/%.cpp
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	$(Q)$(CC) $(GCC_DEPS) $(CXXFLAGS) $(SDL_CFLAGS) $(DEFINES) $(INCS) -c $< -o $@

-include obj/*.d
