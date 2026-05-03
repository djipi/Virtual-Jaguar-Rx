cmake_minimum_required(VERSION 3.16)
project(VirtualJaguarRx VERSION 2.1.3 LANGUAGES C CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)

# Options
option(TRACY_ENABLE "Enable Tracy profiler support" OFF)
option(GDBSTUB_ENABLE "Enable GDB stub support" OFF)
set(CMAKE_VERBOSE_MAKEFILE OFF CACHE BOOL "Show full compilation commands")

# Find required packages
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_AUTOUIC OFF)

# Qt5
find_package(Qt5 REQUIRED COMPONENTS Core Widgets Gui OpenGL)
if(Qt5_FOUND)
    message(STATUS "Found Qt5: ${Qt5_VERSION} (${Qt5_DIR})")
    message(STATUS "  Qt5Core: ${Qt5Core_VERSION}")
    message(STATUS "  Qt5Widgets: ${Qt5Widgets_VERSION}")
    message(STATUS "  Qt5Gui: ${Qt5Gui_VERSION}")
    message(STATUS "  Qt5OpenGL: ${Qt5OpenGL_VERSION}")
endif()
# SDL2
find_package(SDL2 REQUIRED)
# OpenGL
find_package(OpenGL REQUIRED)
# ZLIB
find_package(ZLIB REQUIRED)
# Platform-specific settings
find_package(PkgConfig REQUIRED)

# Manually find libdwarf from local lib/ and include/ directories
find_path(LIBDWARF_INCLUDE_DIRS dwarf.h HINTS "${CMAKE_SOURCE_DIR}/include")
find_library(LIBDWARF_LIBRARIES NAMES libdwarf.a dwarf libdwarf HINTS "${CMAKE_SOURCE_DIR}/lib")

if(NOT LIBDWARF_INCLUDE_DIRS OR NOT LIBDWARF_LIBRARIES)
    message(FATAL_ERROR "libdwarf not found in project's include/ and lib/ directories.")
else()
    message(STATUS "Found libdwarf: ${LIBDWARF_LIBRARIES}")
endif()

# libelf
pkg_check_modules(LIBELF REQUIRED libelf)
# Lua library
find_package(Lua 5.3 REQUIRED)

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
# Optional profiler sources for the Tracy profiler
if(TRACY_ENABLE)
    list(APPEND PROFILER_SOURCES
        src/profiler/tracyprofiler.cpp
        src/profiler/tracyprofiler.h
    )
endif()

# Remote sources
set(REMOTE_SOURCES
    src/remote/baseremote.h
    src/remote/dummyremote.h
)
# Optional remote sources for the GDB stub
if(GDBSTUB_ENABLE)
    list(APPEND PROFILER_SOURCES
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
    src/debugger/NewFnctBreakpointWin.cpp
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
    src/debugger/NewFnctBreakpointWin.h
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

# Create executable
add_executable(${PROJECT_NAME} WIN32 MACOSX_BUNDLE ${ALL_SOURCES})

# Set target properties
set_target_properties(${PROJECT_NAME} PROPERTIES
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
    ${SDL2_INCLUDE_DIRS}
    ${LUA_INCLUDE_DIR}
    ${LIBELF_INCLUDE_DIRS}
    ${LIBDWARF_INCLUDE_DIRS}
)

# Compile definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE
    QT_OPENGL_LIB
    QT_CORE_LIB
    QT_GUI_LIB
    QT_WIDGETS_LIB
)

if(TRACY_ENABLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE TRACY_ENABLE)
    # Add Tracy include path if needed
    # target_include_directories(${PROJECT_NAME} PRIVATE path/to/tracy)
endif()

# Platform-specific definitions
if(MSVC)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _MSC_VER)
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
    ${ZLIB_LIBRARIES}
    ${LUA_LIBRARIES}
    ${LIBELF_LIBRARIES}
    ${LIBDWARF_LIBRARIES}
)

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

# Tracy profiler library (if enabled)
if(TRACY_ENABLE)
    # Link Tracy library - adjust path as needed
    # target_link_libraries(${PROJECT_NAME} PRIVATE TracyClient)
endif()

# Installation
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    BUNDLE DESTINATION .
)

# Install resources on Linux
if(UNIX AND NOT APPLE)
    install(DIRECTORY res/ DESTINATION share/virtualjaguar/res)
    install(FILES docs/virtualjaguar.1 DESTINATION share/man/man1)
endif()

# CPack configuration for packaging
set(CPACK_PACKAGE_NAME "VirtualJaguarRx")
set(CPACK_PACKAGE_VENDOR "Virtual Jaguar Rx Development Team")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Atari Jaguar Console Emulator")
set(CPACK_PACKAGE_VERSION ${PROJECT_VERSION})
set(CPACK_PACKAGE_INSTALL_DIRECTORY "VirtualJaguarRx")

if(WIN32)
    set(CPACK_GENERATOR "NSIS;ZIP")
elseif(APPLE)
    set(CPACK_GENERATOR "DragNDrop")
else()
    set(CPACK_GENERATOR "TGZ;DEB")
endif()

include(CPack)