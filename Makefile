# Makefile for Virtual Jaguar
#
# by James Hammons
# (C) 2011 Underground Software
#
# Note that we control the version information here--uncomment only one set of
# echo's from the "prepare" recipe. :-)
#

FIND = find

ifeq ("$(V)","1")
Q :=
else
Q := @
endif

# Gah
OSTYPE := $(shell uname -a)

# Should catch both 'darwin' and 'darwin7.0'
ifeq "$(findstring Darwin,$(OSTYPE))" "Darwin"
QMAKE_EXTRA := -spec macx-g++
endif

# Set basic flags, these can be overridden from the environment
CFLAGS = -O2
CXXFLAGS = -O2

# Add CPPFLAGS
CFLAGS += $(CPPFLAGS)
CXXFLAGS += $(CPPFLAGS)

# Without these flags, Virtual Jaguar will run very slow.
CFLAGS += -ffast-math -fomit-frame-pointer
CXXFLAGS += -ffast-math -fomit-frame-pointer

# Flags to pass on to qmake...
QMAKE_EXTRA += "QMAKE_CFLAGS_RELEASE=$(CFLAGS)"
QMAKE_EXTRA += "QMAKE_CXXFLAGS_RELEASE=$(CXXFLAGS)"
QMAKE_EXTRA += "QMAKE_LFLAGS_RELEASE=$(LDFLAGS)"

QMAKE_EXTRA += "QMAKE_CFLAGS_DEBUG=$(CFLAGS)"
QMAKE_EXTRA += "QMAKE_CXXFLAGS_DEBUG=$(CXXFLAGS)"
QMAKE_EXTRA += "QMAKE_LFLAGS_DEBUG=$(LDFLAGS)"


all: prepare virtualjaguar
	@echo -e "\033[01;33m***\033[00;32m Success!\033[00m"

obj:
	@mkdir obj
	@mkdir obj/rcc
	@mkdir obj/moc

prepare: obj
	@echo -e "\033[01;33m***\033[00;32m Preparing to compile Virtual Jaguar...\033[00m"
	@echo "#define VJ_RELEASE_VERSION \"v2.1.3\"" > src/version.h
	@echo "#define VJ_RELEASE_SUBVERSION \"Final\"" >> src/version.h
	@echo "#define VJ_REMOVE_DEV_CODE" >> src/version.h
#	@echo "#define VJ_RELEASE_VERSION \"GIT `git log -1 --pretty=format:%ci | cut -d ' ' -f 1 | tr -d -`\"" > src/version.h
#	@echo "#define VJ_RELEASE_SUBVERSION \"2.1.4 Prerelease\"" >> src/version.h

virtualjaguar: sources libs makefile-qt
	@echo -e "\033[01;33m***\033[00;32m Making Virtual Jaguar GUI...\033[00m"
	$(Q)$(MAKE) -f makefile-qt CROSS=$(CROSS) V="$(V)"

makefile-qt: virtualjaguar.pro
	@echo -e "\033[01;33m***\033[00;32m Creating Qt makefile...\033[00m"
	$(Q)$(CROSS)qmake $(QMAKE_EXTRA) virtualjaguar.pro -o makefile-qt

libs: obj/libm68k.a obj/libjaguarcore.a
	@echo -e "\033[01;33m***\033[00;32m Libraries successfully made.\033[00m"

obj/libm68k.a: src/m68000/Makefile sources
	@echo -e "\033[01;33m***\033[00;32m Making Customized UAE 68K Core...\033[00m"
	$(Q)$(MAKE) -C src/m68000 CROSS=$(CROSS) CFLAGS="$(CFLAGS)" V="$(V)"
	$(Q)cp src/m68000/obj/libm68k.a obj/

obj/libjaguarcore.a: jaguarcore.mak sources
	@echo -e "\033[01;33m***\033[00;32m Making Virtual Jaguar core...\033[00m"
	$(Q)$(MAKE) -f jaguarcore.mak CROSS=$(CROSS) CFLAGS="$(CFLAGS)" CXXFLAGS="$(CXXFLAGS)" V="$(V)"

sources: src/*.h src/*.cpp src/m68000/*.c src/m68000/*.h

clean:
	@echo -ne "\033[01;33m***\033[00;32m Cleaning out the garbage...\033[00m"
	@-rm -rf ./obj
	@-rm -rf ./src/m68000/obj
	@-rm -rf makefile-qt
	@-rm -rf virtualjaguar
	@-$(FIND) . -name "*~" -exec rm -f {} \;
	@echo "done!"

statistics:
	@echo -n "Lines in source files: "
	@-$(FIND) ./src -name "*.cpp" | xargs cat | wc -l
	@echo -n "Lines in header files: "
	@-$(FIND) ./src -name "*.h" | xargs cat | wc -l

dist:	clean
