cmake_minimum_required(VERSION 3.16)
project(VirtualJaguar VERSION 2.1.3 LANGUAGES C CXX)

# Set C & C++ standard
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
if(NOT MSVC)
	set(CMAKE_CXX_STANDARD 14)
else()
	set(CMAKE_CXX_STANDARD 17)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /Zc:__cplusplus")
endif()

#
message(STATUS "--------------------------------------------------")
message(STATUS "C compiler      : ${CMAKE_C_COMPILER} (${CMAKE_C_COMPILER_ID} v${CMAKE_C_COMPILER_VERSION})")
message(STATUS "C++ compiler    : ${CMAKE_CXX_COMPILER} (${CMAKE_CXX_COMPILER_ID} v${CMAKE_CXX_COMPILER_VERSION})")
message(STATUS "CMake generator : ${CMAKE_GENERATOR}")
message(STATUS "Architecture    : ${CMAKE_SIZEOF_VOID_P} bytes (8=64bits, 4=32bits)")
message(STATUS "--------------------------------------------------")

# Options
option(GDBSTUB_ENABLE "Enable GDB stub" OFF)
option(HAVE_LIB_CDIO "Use CD library" OFF)
option(LIBDWARF_ENABLE "Enable DWARF format" ON)
option(LIBELF_ENABLE "Enable ELF format" ON)
option(LUA_ENABLE "Enable LUA language" OFF)
option(TRACY_ENABLE "Enable Tracy profiler" OFF)

# Options for makefile
set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Show full compilation commands")

# Find required packages
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)
if(NOT MSVC)
	find_package(PkgConfig REQUIRED)
endif()

# Qt5
if(MSVC)
	if(DEFINED QTDIR)
		list(APPEND CMAKE_PREFIX_PATH "${QTDIR}")
	elseif(DEFINED ENV{QTDIR})
		list(APPEND CMAKE_PREFIX_PATH "$ENV{QTDIR}")	
	endif()
endif()
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui OpenGL)
if(Qt5_FOUND)
    message(STATUS "Found Qt5: ${Qt5_VERSION} (${Qt5_DIR})")
    message(STATUS "  Qt5Core: ${Qt5Core_VERSION}")
    message(STATUS "  Qt5Widgets: ${Qt5Widgets_VERSION}")
    message(STATUS "  Qt5Gui: ${Qt5Gui_VERSION}")
    message(STATUS "  Qt5OpenGL: ${Qt5OpenGL_VERSION}")
endif()

# gdbstub
if(GDBSTUB_ENABLE)
	if(MSVC)
		if(DEFINED GDBSTUBDIR)
			list(APPEND CMAKE_PREFIX_PATH "${GDBSTUBDIR}")
#			set(GDBSTUB_LIBRARIES ${GDBSTUBDIR}/lib/gdbstub.lib)
#			set(GDBSTUB_INCLUDE_DIRS ${GDBSTUBDIR}/include)
#			message(STATUS "Found gdbstub)
		elseif(DEFINED ENV{GDBSTUBDIR})
			list(APPEND CMAKE_PREFIX_PATH "$ENV{GDBSTUBDIR}")
#			set(GDBSTUB_LIBRARIES $ENV{GDBSTUBDIR}/lib/gdbstub.lib)
#			set(GDBSTUB_INCLUDE_DIRS $ENV{GDBSTUBDIR}/include)
#			message(STATUS "Found gdbstub)
#		else()
#			set(GDBSTUB_ENABLE OFF)
#			message(STATUS "gdbstub not found")
		endif()
	endif()
#	else()
		find_package(gdbstub QUIET CONFIG)
		if(NOT gdbstub_FOUND)
			set(GDBSTUB_ENABLE OFF)
			message(STATUS "gdbstub not found")
		else()
#			get_target_property(GDBSTUB_INCLUDE_DIRS gdbstub::gdbstub INTERFACE_INCLUDE_DIRECTORIES)
#			get_target_property(GDBSTUB_LIB_PATH gdbstub::gdbstub IMPORTED_LOCATION)
#			if(NOT GDBSTUB_LIB_PATH)
#				get_target_property(GDBSTUB_LIB_PATH gdbstub::gdbstub IMPORTED_LOCATION_RELEASE)
#			endif()
#			if(NOT GDBSTUB_LIB_PATH)	
#				get_target_property(GDBSTUB_LIB_PATH gdbstub::gdbstub IMPORTED_LOCATION_DEBUG)
#			endif()
#			if(NOT GDBSTUB_LIB_PATH)
#				get_target_property(GDBSTUB_LIB_PATH gdbstub::gdbstub IMPORTED_LOCATION_NOCONFIG)
#			endif()
#			get_target_property(GDBSTUB_DEPS gdbstub::gdbstub INTERFACE_LINK_LIBRARIES)
#			set(GDBSTUB_LIBRARIES "${GDBSTUB_LIB_PATH};${GDBSTUB_DEPS}")
			message(STATUS "Found gdbstub: ${gdbstub_VERSION}")
		endif()
#	endif()
	#message(STATUS "Found gdbstub include: ${GDBSTUB_INCLUDE_DIRS}")
	#message(STATUS "Found gdbstub library: ${GDBSTUB_LIBRARIES}")
else()
	message(STATUS "gdbstub not required")
endif()

# SDL2
if(MSVC)
	if(DEFINED SDL2DIR)
		list(APPEND CMAKE_PREFIX_PATH "${SDL2DIR}")
		cmake_path(SET SDL2_DLL "${SDL2DIR}/lib/x64/SDL2.dll")
		cmake_path(SET SDL2_README_SDL "${SDL2DIR}/README-SDL.txt")
	elseif(DEFINED ENV{SDL2DIR})
		list(APPEND CMAKE_PREFIX_PATH "$ENV{SDL2DIR}")
		cmake_path(SET SDL2_DLL "$ENV{SDL2DIR}/lib/x64/SDL2.dll")
		cmake_path(SET SDL2_README_SDL "$ENV{SDL2DIR}/README-SDL.txt")
	endif()
elseif(MINGW)
	get_filename_component(_mingw_bin_dir "${CMAKE_CXX_COMPILER}" DIRECTORY)
	# There is no README-SDL.txt in the MINGW SDL2 package
	cmake_path(SET SDL2_DLL "${_mingw_bin_dir}/SDL2.dll")	
endif()
find_package(SDL2 REQUIRED)
set(SDL2MAIN_AVAILABLE FALSE)

# OpenGL
find_package(OpenGL REQUIRED)

# ZLIB
if(MSVC)
	if(DEFINED ZLIBDIR)
		list(APPEND CMAKE_PREFIX_PATH "${ZLIBDIR}")
		cmake_path(SET ZLIB_LIBRARY "${ZLIBDIR}/lib/zs.lib")
		#set(ZLIB_LIBRARIES ${ZLIBDIR}/lib/zlibstatic.lib)
		#set(ZLIB_INCLUDE_DIR ${ZLIBDIR}/include)
		#message(STATUS "Found zlib")
	elseif(DEFINED ENV{ZLIBDIR})
		list(APPEND CMAKE_PREFIX_PATH "$ENV{ZLIBDIR}")
		cmake_path(SET ZLIB_LIBRARY "$ENV{ZLIBDIR}/lib/zs.lib")
		#set(ZLIB_LIBRARIES $ENV{ZLIBDIR}/lib/zlibstatic.lib)
		#set(ZLIB_INCLUDE_DIR $ENV{ZLIBDIR}/include)
		#message(STATUS "Found zlib")
	endif()
	#message(STATUS "Found zlib include: ${ZLIB_INCLUDE_DIR}")
	#message(STATUS "Found zlib library: ${ZLIB_LIBRARIES}")
#else()
endif()
#	set(ZLIB_USE_STATIC_LIBS ON)
	find_package(ZLIB REQUIRED)
#endif()

# libcdio
if(HAVE_LIB_CDIO)
    pkg_check_modules(libcdio QUIET libcdio)
    if(NOT libcdio_FOUND)
        set(HAVE_LIB_CDIO OFF)
		message(STATUS "libcdio not found")
    else()
		if(NOT TARGET libcdio::libcdio)
			add_library(libcdio::libcdio INTERFACE IMPORTED)
			set_target_properties(libcdio::libcdio PROPERTIES
			INTERFACE_INCLUDE_DIRECTORIES "${libcdio_INCLUDE_DIRS}"
			INTERFACE_LINK_LIBRARIES "${libcdio_LIBRARIES}"
			)
		endif()
		message(STATUS "Found libcdio: ${libcdio_VERSION}")
		message(STATUS "Found libcdio include: ${libcdio_INCLUDE_DIRS}")
		message(STATUS "Found libcdio library: ${libcdio_LIBRARIES}")
    endif()
else()
	message(STATUS "libcdio not required")
endif()

# libdwarf
if(LIBDWARF_ENABLE)
	if(MSVC)
		if(DEFINED DWARFDIR)
			cmake_path(SET LIBDWARF_LIBRARIES ${DWARFDIR}/lib/dwarf.lib)
			cmake_path(SET LIBDWARF_INCLUDE_DIRS ${DWARFDIR}/include)
			#message(STATUS "Found libdwarf")
		elseif(DEFINED ENV{DWARFDIR})
			cmake_path(SET LIBDWARF_LIBRARIES $ENV{DWARFDIR}/lib/dwarf.lib)
			cmake_path(SET LIBDWARF_INCLUDE_DIRS $ENV{DWARFDIR}/include)
			#message(STATUS "Found libdwarf")
		endif()
		message(STATUS "Found libdwarf include: ${LIBDWARF_INCLUDE_DIRS}")
		message(STATUS "Found libdwarf library: ${LIBDWARF_LIBRARIES}")
	else()
		find_package(libdwarf QUIET)
		if(NOT libdwarf_FOUND)
			set(LIBDWARF_ENABLE OFF)
			message(STATUS "libdwarf not found")
		else()
			get_target_property(LIBDWARF_INCLUDE_DIRS libdwarf::dwarf-static INTERFACE_INCLUDE_DIRECTORIES)
			get_target_property(LIBDWARF_LIBRARIES libdwarf::dwarf-static INTERFACE_LINK_LIBRARIES)
			#message(STATUS "Found libdwarf library: ${LIBDWARF_LIBRARIES}")
			#message(STATUS "Found libdwarf headers: ${LIBDWARF_INCLUDE_DIRS}")
			#message(STATUS "Found libdwarf: ${libdwarf_VERSION}")
			message(STATUS "Found libdwarf")
		endif()
    endif()
else()
	message(STATUS "libdwarf not required")
endif()

# libelf
if(LIBELF_ENABLE)
	if(MSVC)
		if(DEFINED LIBELFDIR)
			cmake_path(SET LIBELF_LIBRARIES ${LIBELFDIR}/lib/libelf.lib)
			cmake_path(SET LIBELF_INCLUDE_DIRS ${LIBELFDIR}/include)
		elseif(DEFINED ENV{LIBELFDIR})
			cmake_path(SET LIBELF_LIBRARIES $ENV{LIBELFDIR}/lib/libelf.lib)
			cmake_path(SET LIBELF_INCLUDE_DIRS $ENV{LIBELFDIR}/include)
		endif()
		message(STATUS "Found libelf include: ${LIBELF_INCLUDE_DIRS}")
		message(STATUS "Found libelf library: ${LIBELF_LIBRARIES}")
	else()
		pkg_check_modules(LIBELF QUIET libelf)
		if(NOT LIBELF_FOUND)
			set(LIBELF_ENABLE OFF)
			message(STATUS "libelf not found")
		else()
			add_library(libelf::libelf INTERFACE IMPORTED)
			set_target_properties(libelf::libelf PROPERTIES
				INTERFACE_INCLUDE_DIRECTORIES "${LIBELF_INCLUDE_DIRS}"
				INTERFACE_LINK_LIBRARIES "${LIBELF_LIBRARIES}"
			)
			#message(STATUS "Found libelf library: ${LIBELF_LIBRARIES}")
			#message(STATUS "Found libelf headers: ${LIBELF_INCLUDE_DIRS}")
			message(STATUS "Found libelf: ${LIBELF_VERSION}")
		endif()
	endif()
else()
	message(STATUS "libelf not required")
endif()

# Lua
if(LUA_ENABLE)
	if(MSVC)
		if(DEFINED LUADIR)
			cmake_path(SET LUA_LIBRARIES ${LUADIR}/lua54.lib)
			cmake_path(SET LUA_DLL ${LUADIR}/lua54.dll)
			cmake_path(SET LUA_INCLUDE_DIR ${LUADIR}/include)
		elseif(DEFINED ENV{LUADIR})
			cmake_path(SET LUA_LIBRARIES $ENV{LUADIR}/lua54.lib)
			cmake_path(SET LUA_DLL $ENV{LUADIR}/lua54.dll)
			cmake_path(SET LUA_INCLUDE_DIR $ENV{LUADIR}/include)
		endif()
		message(STATUS "Found lua include: ${LUA_INCLUDE_DIR}")
		message(STATUS "Found lua library: ${LUA_LIBRARIES}")
	else()
		find_package(Lua 5.3 QUIET)
	endif()
else()
	message(STATUS "Lua not required")	
endif()

# Tracy profiler
if(TRACY_ENABLE)
	if(MSVC)
		if(DEFINED TRACYPROFILERDIR)
			list(APPEND CMAKE_PREFIX_PATH "${TRACYPROFILERDIR}")
#			set(LIBTRACY_LIBRARIES ${TRACYPROFILERDIR}/lib/TracyClient.lib)
#			set(LIBTRACY_INCLUDE_DIRS ${TRACYPROFILERDIR}/include)
		elseif(DEFINED ENV{TRACYPROFILERDIR})
			list(APPEND CMAKE_PREFIX_PATH "$ENV{TRACYPROFILERDIR}")
#			set(LIBTRACY_LIBRARIES $ENV{TRACYPROFILERDIR}/lib/TracyClient.lib)
#			set(LIBTRACY_INCLUDE_DIRS $ENV{TRACYPROFILERDIR}/include)
#		else()
#			set(TRACY_ENABLE OFF)
#			message(STATUS "Tracy profiler not found")
		endif()
	endif()
#	else()
		find_package(tracy QUIET)
		if(NOT tracy_FOUND)
			set(TRACY_ENABLE OFF)
			message(STATUS "Tracy profiler not found")
		else()
			get_target_property(LIBTRACY_INCLUDE_DIRS Tracy::TracyClient INTERFACE_INCLUDE_DIRECTORIES)
			#get_target_property(LIBTRACY_LIBRARIES Tracy::TracyClient INTERFACE_LINK_LIBRARIES)
			#message(STATUS "Found tracy profiler library: ${LIBTRACY_LIBRARIES}")
			#message(STATUS "Found tracy profiler headers: ${LIBTRACY_INCLUDE_DIRS}")
			message(STATUS "Found Tracy profiler")
			if(NOT LIBTRACY_PUBLIC_DIR)
				message(FATAL_ERROR "LIBTRACY_INCLUDE_DIRS must link to the tracy/public directory")
			elseif(NOT EXISTS "${LIBTRACY_PUBLIC_DIR}")
				message(FATAL_ERROR "${LIBTRACY_PUBLIC_DIR} doesn't contain the required tracy/public source code")
			endif()
		endif()
#	endif()
else()
	message(STATUS "Tracy profiler not required")
endif()

message(STATUS "--------------------------------------------------")

# Source files - Core emulator
set(CORE_SOURCES
    src/blitter.cpp
    src/cdintf.cpp
    src/cdrom.cpp
    src/crc32.cpp
    src/dac.cpp
    src/dsp.cpp
    src/eeprom.cpp
    src/event.cpp
    src/file.cpp
    src/filedb.cpp
    src/gpu.cpp
    src/jagbios.cpp
    src/jagbios2.cpp
    src/jagcdbios.cpp
    src/jagdasm.cpp
    src/jagdevcdbios.cpp
    src/jagstub1bios.cpp
    src/jagstub2bios.cpp
    src/jaguar.cpp
    src/jerry.cpp
    src/joystick.cpp
    src/LEB128.cpp
    src/log.cpp
    src/memory.cpp
    src/memtrack.cpp
    src/mmu.cpp
    src/modelsBIOS.cpp
    src/op.cpp
    src/profiler.cpp
    src/remote.cpp
    src/settings.cpp
    src/state.cpp
    src/tom.cpp
    src/universalhdr.cpp
    src/unzip.cpp
    src/wavetable.cpp
)

# M68000 core sources
set(M68K_SOURCES
    src/m68000/obj/cpudefs.c
    src/m68000/obj/cpuemu.c
    src/m68000/obj/cpustbl.c
    src/m68000/cpuextra.c
    src/m68000/readcpu.c
    src/m68000/m68kinterface.c
    src/m68000/m68kdasm.c
)

# Profiler sources
set(PROFILER_SOURCES
    src/profiler/ctrlprofilerwin.cpp
    src/profiler/baseprofiler.h
    src/profiler/dummyprofiler.h
    src/profiler/vjrxprofiler.cpp
    src/profiler/vjrxprofiler.h
    src/profiler/vjrxprofilerwin.cpp
    src/profiler/vjrxprofilerwin.h
)
# Tracy profiler sources
if(TRACY_ENABLE)
    list(APPEND PROFILER_SOURCES
        src/profiler/tracyprofiler.cpp
        src/profiler/tracyprofiler.h
		${LIBTRACY_PUBLIC_DIR}/TracyClient.cpp
    )
endif()

# Remote sources
set(REMOTE_SOURCES
    src/remote/baseremote.h
    src/remote/dummyremote.h
)
# GDB remote sources
if(GDBSTUB_ENABLE)
    list(APPEND REMOTE_SOURCES
        src/remote/gdbremote.cpp
        src/remote/gdbremote.h
    )
endif()

# GUI sources
set(GUI_SOURCES
    src/gui/app.cpp
    src/gui/mainwin.cpp
    src/gui/glwidget.cpp
    src/gui/about.cpp
    src/gui/configdialog.cpp
    src/gui/filepicker.cpp
    src/gui/filelistmodel.cpp
    src/gui/filethread.cpp
    src/gui/gamepad.cpp
    src/gui/help.cpp
    src/gui/imagedelegate.cpp
    src/gui/keygrabber.cpp
    src/gui/profile.cpp
    src/gui/emustatus.cpp
    src/gui/stdConsole.cpp
    src/gui/generaltab.cpp
    src/gui/controllertab.cpp
    src/gui/controllerwidget.cpp
    src/gui/keybindingstab.cpp
    src/gui/modelsbiostab.cpp
    src/gui/alpinetab.cpp
    src/gui/exceptionstab.cpp
)

# Alpine/Debug browser sources
set(BROWSER_SOURCES
    src/gui/debug/cpubrowser.cpp
    src/gui/debug/memorybrowser.cpp
    src/gui/debug/stackbrowser.cpp
    src/gui/debug/opbrowser.cpp
    src/gui/debug/m68kdasmbrowser.cpp
    src/gui/debug/riscdasmbrowser.cpp
    src/gui/debug/hwregsbrowser.cpp
    src/gui/debug/hwregsblitterbrowser.cpp
    src/gui/debug/romcartbrowser.cpp
)

# Debugger sources
set(DEBUGGER_SOURCES
    src/debugger/DBGManager.cpp
    src/debugger/DWARFManager.cpp
    src/debugger/ELFManager.cpp
    src/debugger/HWLABELManager.cpp
    src/debugger/VideoWin.cpp
    src/debugger/DasmWin.cpp
    src/debugger/m68kDasmWin.cpp
    src/debugger/GPUDasmWin.cpp
    src/debugger/DSPDasmWin.cpp
    src/debugger/SourcesWin.cpp
    src/debugger/SourceCWin.cpp
    src/debugger/FilesrcListWin.cpp
    src/debugger/BreakpointsWin.cpp
    src/debugger/NewBreakpointWin.cpp
    src/debugger/SaveDumpAsWin.cpp
    src/debugger/CartFilesListWin.cpp
    src/debugger/allwatchbrowser.cpp
    src/debugger/localbrowser.cpp
    src/debugger/heapallocatorbrowser.cpp
    src/debugger/memory1browser.cpp
    src/debugger/exceptionvectortablebrowser.cpp
    src/debugger/callstackbrowser.cpp
    src/debugger/debuggertab.cpp
)

# Headers that need MOC
set(MOC_HEADERS
    src/gui/mainwin.h
    src/gui/glwidget.h
    src/gui/configdialog.h
    src/gui/filepicker.h
    src/gui/filethread.h
    src/gui/keygrabber.h
    src/gui/emustatus.h
    src/gui/stdConsole.h
    src/gui/generaltab.h
    src/gui/controllertab.h
    src/gui/controllerwidget.h
    src/gui/keybindingstab.h
    src/gui/modelsbiostab.h
    src/gui/alpinetab.h
    src/gui/exceptionstab.h
    src/gui/debug/cpubrowser.h
    src/gui/debug/memorybrowser.h
    src/gui/debug/stackbrowser.h
    src/gui/debug/opbrowser.h
    src/gui/debug/m68kdasmbrowser.h
    src/gui/debug/riscdasmbrowser.h
    src/gui/debug/hwregsbrowser.h
    src/gui/debug/hwregsblitterbrowser.h
    src/gui/debug/romcartbrowser.h
    src/debugger/VideoWin.h
    src/debugger/DasmWin.h
    src/debugger/m68kDasmWin.h
    src/debugger/GPUDasmWin.h
    src/debugger/DSPDasmWin.h
    src/debugger/SourcesWin.h
    src/debugger/SourceCWin.h
    src/debugger/FilesrcListWin.h
    src/debugger/BreakpointsWin.h
    src/debugger/NewBreakpointWin.h
    src/debugger/SaveDumpAsWin.h
    src/debugger/CartFilesListWin.h
    src/debugger/allwatchbrowser.h
    src/debugger/localbrowser.h
    src/debugger/heapallocatorbrowser.h
    src/debugger/memory1browser.h
    src/debugger/exceptionvectortablebrowser.h
    src/debugger/callstackbrowser.h
    src/debugger/debuggertab.h
    src/profiler/ctrlprofilerwin.h
)

# Qt resource file
set(RESOURCES
    src/gui/virtualjaguar.qrc
)

# All sources combined
set(ALL_SOURCES
    ${CORE_SOURCES}
    ${M68K_SOURCES}
    ${PROFILER_SOURCES}
    ${GUI_SOURCES}
    ${BROWSER_SOURCES}
    ${DEBUGGER_SOURCES}
	${REMOTE_SOURCES}
    ${RESOURCES}
)

# Create version header
file(WRITE ${CMAKE_BINARY_DIR}/version.h
"#ifndef __VERSION_H__
#define __VERSION_H__
#define VJ_RELEASE_VERSION \"v2.1.3\"
#define VJ_RELEASE_SUBVERSION \"Final\"
#define VJ_REMOVE_DEV_CODE
#endif
")

# Platform-specific compile options
if(MSVC)
	add_compile_options(/Zc:__cplusplus)
endif()

# Create executable
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${ALL_SOURCES} ${CMAKE_SOURCE_DIR}/res/vj.rc)

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
#	DISABLE_PRECOMPILED_HEADERS ON
    OUTPUT_NAME "virtualjaguar"
)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/m68000
    ${CMAKE_SOURCE_DIR}/src/gui
    ${CMAKE_SOURCE_DIR}/src/debugger
    ${CMAKE_SOURCE_DIR}/src/profiler
    ${CMAKE_BINARY_DIR}
#    ${SDL2_INCLUDE_DIR}
#    ${LUA_INCLUDE_DIR}
    ${LIBELF_INCLUDE_DIRS}
    ${LIBDWARF_INCLUDE_DIRS}
#	${LIBCDIO_INCLUDE_DIRS}
#	${LIBTRACY_INCLUDE_DIRS}
#	${GDBSTUB_INCLUDE_DIRS}
)
#if(MSVC)
#	target_include_directories(${PROJECT_NAME} PRIVATE ${ZLIB_INCLUDE_DIR})
#endif()
if(LUA_ENABLE)
	target_include_directories(${PROJECT_NAME} PRIVATE ${LUA_INCLUDE_DIR})
endif()
if(TRACY_ENABLE)
	target_include_directories(${PROJECT_NAME} PRIVATE ${LIBTRACY_INCLUDE_DIRS})
endif()

# Qt5 compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_OPENGL_LIB
    QT_CORE_LIB
    QT_GUI_LIB
    QT_WIDGETS_LIB
)

# SDL2 definitions
#target_compile_definitions(${PROJECT_NAME} PRIVATE SDL_MAIN_HANDLED)

# General definitions
foreach(macro GDBSTUB_ENABLE HAVE_LIB_CDIO LIBDWARF_ENABLE LIBELF_ENABLE TRACY_ENABLE LUA_ENABLE)
    if(${macro})
        target_compile_definitions(${PROJECT_NAME} PRIVATE ${macro})
    endif()
endforeach()
# Tracy profiler definitions
if(TRACY_ENABLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_EMULATED_CLOCK TRACY_ON_DEMAND)
endif()

# Platform-specific definitions
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _MSC_VER)
	add_compile_definitions(_ALLOW_COMPILER_AND_STL_VERSION_MISMATCH)
elseif(MINGW)
    # Definitions for MinGW
elseif(UNIX AND NOT APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE __GCCUNIX__)
endif()

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    Qt5::Core
    Qt5::Widgets
    Qt5::Gui
    Qt5::OpenGL
    SDL2::SDL2
    ${OPENGL_LIBRARIES}
#    ${ZLIB_LIBRARIES}
	ZLIB::ZLIB
#    ${LUA_LIBRARIES}
    ${LIBELF_LIBRARIES}
    ${LIBDWARF_LIBRARIES}
#	${LIBCDIO_LIBRARIES}
#	${LIBTRACY_LIBRARIES}
#	${GDBSTUB_LIBRARIES}
)
if(LUA_ENABLE)
	target_link_libraries(${PROJECT_NAME} PRIVATE ${LUA_LIBRARIES})
endif()
if(TRACY_ENABLE)
	if(WIN32)
		target_link_libraries(${PROJECT_NAME} PRIVATE dbghelp secur32 ws2_32)
	endif()
#	target_link_libraries(${PROJECT_NAME} PRIVATE Tracy::TracyClient)
endif()
if(GDBSTUB_ENABLE)
	target_link_libraries(${PROJECT_NAME} PRIVATE gdbstub::gdbstub)
endif()
if(HAVE_LIB_CDIO)
#	if(TARGET libcdio::libcdio)
#		target_link_libraries(${PROJECT_NAME} PRIVATE libcdio::libcdio)
#	else()
		target_link_libraries(${PROJECT_NAME} PRIVATE ${libcdio_LIBRARIES})
#	endif()
endif()

# Platform-specific libraries
if(UNIX AND NOT APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        pthread
        dl
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        winmm
    )
endif()

# Link output console options
if(WIN32)
    if(MSVC)
        set_target_properties(${PROJECT_NAME} PROPERTIES LINK_FLAGS "/SUBSYSTEM:CONSOLE")
    else()
        target_link_options(${PROJECT_NAME} PRIVATE "-mconsole")
    endif()
endif()

# Installation
if(WIN32)
	 # Install the main executable to the install prefix root
    install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION .)
	
	# Locate windeployqt using the Qt5 qmake target property
    get_target_property(_qt_qmake_exe Qt5::qmake LOCATION)
    get_filename_component(_qt_bin_dir "${_qt_qmake_exe}" DIRECTORY)
	if(MSVC)
		find_program(WINDEPLOYQT_EXECUTABLE windeployqt HINTS "${_qt_bin_dir}")
	else()
		find_program(WINDEPLOYQT_EXECUTABLE windeployqt-qt5 HINTS "${_qt_bin_dir}")
	endif()

	# Qt critical information
	message(STATUS "Found Qt qmake: " ${_qt_qmake_exe})	
	message(STATUS "Found Qt binary: " ${_qt_bin_dir})	
	message(STATUS "Found Qt deployment: " ${WINDEPLOYQT_EXECUTABLE})	

    if(WINDEPLOYQT_EXECUTABLE)	
        install(CODE "
			message(STATUS \"Qt5 dependencies installation\")			
			# Automatically deploy Qt dependencies during 'make install'
            execute_process(
                COMMAND \"${WINDEPLOYQT_EXECUTABLE}\" 
                    --no-translations 
                    --no-system-d3d-compiler 
                    --no-opengl-sw 
                    --no-angle
                    --no-svg 
                    --no-webkit 
                    --no-xml 
                    --no-network 
                    --no-sql
                    \"\${CMAKE_INSTALL_PREFIX}/${PROJECT_NAME}.exe\"
                #RESULT_VARIABLE _windeployqt_result
                #ERROR_VARIABLE _windeployqt_error
				OUTPUT_QUIET
            )
			
			#if(NOT _windeployqt_result EQUAL 0)
            #    message(WARNING \"windeployqt failed with return code: \${_windeployqt_result}\")
            #endif()
			
			# Clean up unnecessary image format plugins
			file(GLOB ALL_IMAGE_PLUGINS \"\${CMAKE_INSTALL_PREFIX}/imageformats/*.dll\")
            foreach(PLUGIN_FILE \${ALL_IMAGE_PLUGINS})
                if(NOT PLUGIN_FILE MATCHES \"qjpeg.dll\")
                    file(REMOVE \${PLUGIN_FILE})
                endif()
            endforeach()
			
			file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/styles\")
			file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/libEGL.dll\")
			file(REMOVE_RECURSE \"\${CMAKE_INSTALL_PREFIX}/libGLESv2.dll\")
				
			# Copy optional third-party DLLs
			if(EXISTS \"${LUA_DLL}\")
				file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}\" TYPE FILE FILES \"${LUA_DLL}\")
			endif()
			if(EXISTS \"${SDL2_DLL}\")
				file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}\" TYPE FILE FILES \"${SDL2_DLL}\")
			endif()
			if(EXISTS \"${SDL2_README_SDL}\")
				file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}\" TYPE FILE FILES \"${SDL2_README_SDL}\")
			endif()
	
			if(\"${MINGW}\")
				# Detect compiler directory to locate the MinGW /bin folder
				get_filename_component(MINGW_BIN_DIR \"${CMAKE_CXX_COMPILER}\" DIRECTORY)
				
				# MinGW critical information
				message(STATUS \"Found MinGW binary: \" \"\${MINGW_BIN_DIR}\")	
        
				# List of core standard runtime DLLs for a MinGW64 C++ binary
				set(MINGW_DLLS
					\"${MINGW_BIN_DIR}/libgcc_s_seh-1.dll\"
					\"\${MINGW_BIN_DIR}/libstdc++-6.dll\"
					\"\${MINGW_BIN_DIR}/libwinpthread-1.dll\"
					\"\${MINGW_BIN_DIR}/zlib1.dll\"
					\"\${MINGW_BIN_DIR}/libdouble-conversion.dll\"
					\"\${MINGW_BIN_DIR}/libicuin78.dll\"
					\"\${MINGW_BIN_DIR}/libicuuc78.dll\"
					\"\${MINGW_BIN_DIR}/libharfbuzz-0.dll\"
					\"\${MINGW_BIN_DIR}/libpcre2-16-0.dll\"
					\"\${MINGW_BIN_DIR}/libzstd.dll\"
					\"\${MINGW_BIN_DIR}/libmd4c.dll\"
					\"\${MINGW_BIN_DIR}/libpng16-16.dll\"
					\"\${MINGW_BIN_DIR}/libfreetype-6.dll\"
					\"\${MINGW_BIN_DIR}/libglib-2.0-0.dll\"
					\"\${MINGW_BIN_DIR}/libicudt78.dll\"
					\"\${MINGW_BIN_DIR}/libgraphite2.dll\"
					\"\${MINGW_BIN_DIR}/libintl-8.dll\"
					\"\${MINGW_BIN_DIR}/libpcre2-8-0.dll\"
					\"\${MINGW_BIN_DIR}/libbrotlidec.dll\"
					\"\${MINGW_BIN_DIR}/libbz2-1.dll\"
					\"\${MINGW_BIN_DIR}/libbrotlicommon.dll\"
					\"\${MINGW_BIN_DIR}/libiconv-2.dll\"
					if(\"\${LUA_ENABLE}\")
						\"\${MINGW_BIN_DIR}/lua55.dll\"
					endif()
					if (\"\${HAVE_LIB_CDIO}\")
						\"\${MINGW_BIN_DIR}/libcdio-19.dll\"
						\"\${MINGW_BIN_DIR}/libgcc_s_seh-1.dll\"
					endif()
				)
        
				foreach(DLL_PATH \${MINGW_DLLS})
					if(EXISTS \"\${DLL_PATH}\")
						file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}\" TYPE FILE FILES \"\${DLL_PATH}\")
					endif()
				endforeach()
			endif()
	
			# Copy additional documentations
			if(EXISTS \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE\")
				file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}\" TYPE FILE FILES \"${CMAKE_CURRENT_SOURCE_DIR}/LICENSE\")
			endif()
			if(EXISTS \"${CMAKE_CURRENT_SOURCE_DIR}/docs/WHATSNEW\")
				file(INSTALL DESTINATION \"\${CMAKE_INSTALL_PREFIX}\" TYPE FILE FILES \"${CMAKE_CURRENT_SOURCE_DIR}/docs/WHATSNEW\")
			endif()
		")
    endif()		
endif()
