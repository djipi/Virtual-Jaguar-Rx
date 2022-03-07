#
# Makefile for Virtual Jaguar core library
#
# by James Hammons
# Modified by Jean-Paul Mari
#
# This software is licensed under the GPL v3 or any later version. See the
# file LICENSE file for details. ;-)
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
QT_CFLAGS = -fPIC -I/usr/include/qt5 -I/usr/include/qt5/QtOpenGL -I/usr/include/qt5/QtWidgets -I/usr/include/qt5/QtGui -I/usr/include/qt5/QtCore
DEFINES = -D$(SYSTYPE)
GCC_DEPS = -MMD

INCS := -I./src

OBJS := \
	obj/core/AJ2/blitter.o  \
	obj/core/blitter.o      \
	obj/core/cdintf.o       \
	obj/core/cdrom.o        \
	obj/core/dac.o          \
	obj/core/dsp.o          \
	obj/core/eeprom.o       \
	obj/core/event.o        \
	obj/core/filedb.o       \
	obj/core/gpu.o          \
	obj/core/jagbios.o      \
	obj/core/jagbios2.o     \
	obj/core/jagcdbios.o    \
	obj/core/jagdevcdbios.o \
	obj/core/jagstub1bios.o \
	obj/core/jagstub2bios.o \
	obj/core/jagdasm.o      \
	obj/core/jaguar.o       \
	obj/core/jerry.o        \
	obj/core/joystick.o     \
	obj/core/memory.o       \
	obj/core/memtrack.o     \
	obj/core/mmu.o          \
	obj/core/modelsBIOS.o   \
	obj/core/op.o           \
	obj/core/state.o        \
	obj/core/tom.o          \
	obj/core/universalhdr.o \
	obj/core/wavetable.o

# Targets for convenience sake, not "real" targets
.PHONY: clean

all: obj objcore objcoreaj2 obj/libjaguarcore.a
	@echo "Done!"

clean:
	@rm -f $(OBJS)

obj:
	@mkdir -p obj
objcore:
	@mkdir -p obj/core
objcoreaj2:
	@mkdir -p obj/core/AJ2

# Library rules (might not be cross-platform compatible)
obj/libjaguarcore.a: $(OBJS) 
	$(Q)$(AR) $(ARFLAGS) obj/libjaguarcore.a $(OBJS)

# Main source compilation (implicit rules)...
obj/core/%.o: src/%.cpp
	@echo -e "\033[01;33m***\033[00;32m Compiling $<...\033[00m"
	$(Q)$(CC) $(GCC_DEPS) $(CXXFLAGS) $(SDL_CFLAGS) $(QT_CFLAGS) $(DEFINES) $(INCS) -c $< -o $@

-include obj/core/*.d obj/core/AJ2/*.d
