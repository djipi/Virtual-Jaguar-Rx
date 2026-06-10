//
// tracyprofiler.cpp - Profiler with Tracy
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  mm/dd/yyyy)  What
// ---  ----------  -----------------------------------------------------------
// JPM  10/29/2025  Created this file
// JPM   Oct./2025  Added pause feature, better control for the Tracy profiler feed and memory allocation tracking
// JPM   Nov./2025  Tracy profiler connection with a cancellable dialog, Lua initialization, and timer manipulations
// JPM   Dec./2025  Added M68K functions tracking messages, and frames marking
// JPM  06/09/2026  Make Lua optional
//

//#define TracyFunction functionName
//#define TracyLine linenumber
//#define TracyFile filename

//
//#define TracyFunction
//
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <QtWidgets/QMessageBox>
#include <QtWidgets/QPushButton>
#include <QtCore/QTimer>
#include <QtWidgets/QApplication>
#include "tracyprofiler.h"
#ifdef LUA_ENABLE
#include "tracy\TracyLua.hpp"
#endif
#include "client\TracyProfiler.hpp"

//#ifdef _MSC_VER
#if 0
// Define the Lua zone state that Tracy requires
namespace tracy
{
	LuaZoneState& GetLuaZoneState(void)
	{
		static LuaZoneState luaZoneState;
		return luaZoneState;
	}
}
#endif

//
#undef TracyLine
#undef TracyFunction
#undef TracyFile

// Message color codes
#define COLOUR_DEFAULT	0xFFFFFFFF
#define COLOUR_INFO		0xFFFFFF80
#define COLOUR_WARNING	0xFFFFA000
#define COLOUR_ERROR	0xFF8080FF
#define COLOUR_SUCCESS	0xFF00FF00
#define COLOUR_DEBUG	0xFF808080

// M68K variables for the Tracy profiler
static constexpr double M68K_CLOCK_HZ = 13290000.0;			// 13.29 MHz
double NANOS_PER_CYCLE = 1000000000.0 / M68K_CLOCK_HZ;		// M68K @ 13.29 MHz: 1 cycle is more or less 75.244 nanoseconds
int64_t g_tracy_time_offset;
bool g_tracy_emulation;


//
TracyProfiler::TracyProfiler(void) :
tracyPaused(true),
TracyCancel(false)
{
	g_tracy_emulation = false;

	// name the application in Tracy, is displayed in the Trace information
	TracyCAppInfo("Virtual Jaguar Rx - Profiler", strlen("Virtual Jaguar Rx - Profiler"));
	//TracyCSetThreadName("M68K CPU");
	//TracyCPlotConfig("68000 cycles", TracyPlotFormatNumber, true, true, 0xFFFFFF80);
}


#ifdef LUA_ENABLE
// Tracy profiler Lua initialization
void TracyProfiler::InitLua(lua_State* LuaLib)
{
	tracy::LuaRegister(LuaLib);
}
#endif


// Wait for Tracy profiler connection with a cancellable dialog
// Returns true if connected, false if cancelled
bool TracyProfiler::WaitForConnection(void)
{
	// check profiler availability
	if (!tracy::ProfilerAvailable())
	{
		// unsuccessfully connected
		return false;
	}
	else
	{
		// check if already connected
		if (tracy::GetProfiler().IsConnected())
		{
			return true;
		}
		else
		{
			// create a message box
			QMessageBox msgBox;
			msgBox.setWindowTitle("Tracy Profiler");
			msgBox.setText("Waiting for Tracy profiler connection...");
			msgBox.setInformativeText("Please start the Tracy profiler application and connect to this instance.");
			msgBox.setIcon(QMessageBox::Information);
			// add Cancel button
			QPushButton* cancelButton = msgBox.addButton("Cancel", QMessageBox::RejectRole);
			msgBox.setDefaultButton(cancelButton);
			// dialog non-modal so we can check connection status
			msgBox.setModal(false);
			msgBox.show();
			// create a timer to check connection status periodically (every 100ms)
			QTimer timer;
			bool connected = false;
			QObject::connect(&timer, &QTimer::timeout, [&]() {
				if (tracy::GetProfiler().IsConnected())
				{
					connected = true;
					msgBox.accept();
				}
				});
			timer.start(100);
			// wait for either connection or cancellation
			int result = msgBox.exec();
			timer.stop();
			if (result == QMessageBox::Rejected || !connected)
			{
				// cancellation or dialog was closed
				return false;
			}
			else
			{
				// successfully connected
				return true;
			}
		}
	}
}


// Initialize to authorize the Tracy profiler host timer
bool TracyProfiler::Start(void)
{
	// force the Tracy profiler to use the host timer to allow connection with the user
	Timer(false, false);
	// wait for Tracy profiler connection
	TracyCancel = WaitForConnection();
	// get current Tracy profiler host timer
	Timer(true, false);
	// Tracy profiler is now emulating the 68000 CPU timing
	g_tracy_emulation = true;
	// displayed the message in the Tracy profiler messages list
	TracyCMessageC("=== Atari Jaguar: 68000  ===", 28, COLOUR_DEFAULT);
	return TracyCancel;
}


// Enable, or disable, the Tracy profiler host timer
void TracyProfiler::Timer(bool onoff, bool newvalue)
{
	g_tracy_time_offset = onoff ? tracy::Profiler::GetTime() : 0;
	if (newvalue)
	{
		while (!(g_tracy_time_offset = tracy::Profiler::GetTime()));
	}
}


// Pause, or resume, the Tracy profiler
// Arguments:
// true: pause, false: resume
void TracyProfiler::Pause(bool pause)
{
	// set the pause state
	tracyPaused = !pause;
}


// Start a new frame for the Tracy profiler
void  TracyProfiler::M68KFrameStart(size_t frameNumber)
{
	TracyCFrameMarkStart("M68K Frame");
}


// End the current frame for the Tracy profiler
void  TracyProfiler::M68KFrameEnd(size_t frameNumber)
{
	TracyCFrameMarkEnd("M68K Frame");
}


// Add a memory allocution event for the Tracy memory allocation
// null-pointer is not recorded in Tracy
void TracyProfiler::M68Kmalloc(void* zoneCtx, size_t ptr, size_t size, int depth)
{
	if (!tracyPaused)
	{
		//
		char buffer[256];
		if (ptr)
		{
			// check potential weird size
			if (size > 0x200000)
			{
				// display message for a weird size
				sprintf(buffer, "M68K malloc: ptr=0x%06x size=%u bytes (weird size, profiler may have an issue)", (unsigned int)ptr, (unsigned int)size);
				TracyCMessageC(buffer, strlen(buffer), COLOUR_DEBUG);
			}
			else
			{
				// display message for a non-null pointer
				sprintf(buffer, "M68K malloc: ptr=0x%06x size=%u bytes", (unsigned int)ptr, (unsigned int)size);
				TracyCMessageC(buffer, strlen(buffer), COLOUR_SUCCESS);
			}
			// add the pointer address in the memory allocation
			((TracyCZoneCtx*)zoneCtx)->active = ((depth >= 0) ? TracyCAlloc((void*)ptr, size), (((TracyCZoneCtx*)zoneCtx)->id = depth), true : false);
		}
		else
		{
			// display message for a null pointer
			sprintf(buffer, "M68K malloc: ptr=0x%06x size=%u bytes (memory allocation error)", (unsigned int)ptr, (unsigned int)size);
			TracyCMessageC(buffer, strlen(buffer), COLOUR_ERROR);
			RAZIndex(zoneCtx);
		}
	}
}


// Add the free event for the Tracy memory allocation
// null-pointer will have a null ctx pointer
// unknown pointer will have a negative (-1) ctx pointer
// flush mode won't display messages
void TracyProfiler::M68Kfree(void* zoneCtx, size_t ptr, bool flush)
{
	//if (!tracyPaused || flush)
	{
		//
		char buffer[256];
		if (zoneCtx != (void*)-1)
		{
			// display message for a null or an existing pointer
			if (!flush)
			{
				sprintf(buffer, "M68K free: ptr=0x%06x", (unsigned int)ptr);
				unsigned int colour = ptr ? COLOUR_SUCCESS : COLOUR_WARNING;
				TracyCMessageC(buffer, strlen(buffer), colour);
			}
			// free the non-null pointer address in the memory allocation
			(ptr && zoneCtx && ((TracyCZoneCtx*)zoneCtx)->active) ? TracyCFree((void*)ptr), true : false;
			zoneCtx ? RAZIndex(zoneCtx), true : false;
		}
		else
		{
			// display message for an unknown pointer
			if (!flush)
			{
				sprintf(buffer, "M68K free: ptr=0x%06x (unknown)", (unsigned int)ptr);
				TracyCMessageC(buffer, strlen(buffer), COLOUR_ERROR);
			}
		}
	}
}


// Start a Tracy zone for the 68000 function
// The plot colors will depend on the plot's name (yellow)
void TracyProfiler::M68Kenter(void* zoneCtx, char* functionName, char* filename, size_t linenumber, size_t startCycle)
{
	char buffer[256];

	// start the function zone only if not paused
	if (!tracyPaused)
	{
		// clear the source file & line wording dedicated to the function name
		unsigned int TracyLine = 0;
		char* TracyFile = (char*)"";
		// create the zone context
		TracyCZoneC(ctx, 0xBEBE70, true);
		// display the function's name in the zone
		TracyCZoneName(ctx, functionName, strlen(functionName));
		// store the zone context
		*(TracyCZoneCtx*)zoneCtx = ctx;
		// start the cycle plot
		TracyCPlot("M68K start cycle", (double)startCycle);
		// display the message in the Tracy profiler messages list
		sprintf(buffer, "M68K function: Enter for %s() with id=%u", functionName, ctx.id);
		TracyCMessageC(buffer, strlen(buffer), COLOUR_SUCCESS);
	}
	else
	{
		// display the message in the Tracy profiler messages list
		sprintf(buffer, "M68K function: Enter for %s() (paused)", functionName);
		TracyCMessageC(buffer, strlen(buffer), COLOUR_WARNING);
		// clear the zone context in pause mode
		RAZIndex(zoneCtx);
	}
}


// End the Tracy zone for the 68000 function
// The plot colors will depend on the plot's name (yellow)
void TracyProfiler::M68Kleave(void* zoneCtx, size_t usedCycles)
{
	char text[256];
	// display the message in the Tracy profiler messages list
	sprintf(text, "M68K function: Leave id=%u%s%s", ((TracyCZoneCtx*)zoneCtx)->id, (tracyPaused ? " (paused)" : ""), ((TracyCZoneCtx*)zoneCtx)->active ? "" : " (inactive)");
	TracyCMessageC(text, strlen(text), (((TracyCZoneCtx*)zoneCtx)->active) ? (tracyPaused ? COLOUR_WARNING : COLOUR_SUCCESS) : COLOUR_ERROR);
	//printf("%s - Active: %zu\n", text, ((TracyCZoneCtx*)zoneCtx)->active);

	// end the function zone
	if (((TracyCZoneCtx*)zoneCtx)->id && ((TracyCZoneCtx*)zoneCtx)->active)
	{
		char textWithCommas[64];

		// display the used cycles in the function zone label
		snprintf(text, sizeof(text), "%s cycles", IntegerToStringWithCommas(textWithCommas, sizeof(textWithCommas), usedCycles));
		TracyCZoneText(*(TracyCZoneCtx*)zoneCtx, text, strlen(text));
		// used cycles plots
		TracyCPlot("M68K cycles per function", (double)usedCycles);
		TracyCPlot("M68K function time (\xC2\xB5s)", ((double)usedCycles / M68K_CLOCK_HZ) * 1e6);
		TracyCPlot("M68K function time (ms)", ((double)usedCycles / M68K_CLOCK_HZ) * 1e3);
		// end the zone context without clearing the context
		TracyCZoneEnd(*(TracyCZoneCtx*)zoneCtx);
		// clear the zone context
		RAZIndex(zoneCtx);
	}
}


// Return Tracy zone activity for the 68000 function
bool TracyProfiler::M68Kactive(void* zoneCtx)
{
	// cast the zone context pointer
	return (((TracyCZoneCtx*)zoneCtx)->active);
}


// Format an integer with commas into buffer
char* TracyProfiler::IntegerToStringWithCommas(char* out, size_t len, size_t value)
{
	// transform value to string
	char tmp[100];
	itoa((int)value, tmp, 10);
	// include separator's commas
	memset(out, 0, len);
	size_t i = strlen(tmp);
	if (i)
	{
		int j = 0;
		char* tmpPtr = tmp + i - 1;
		char* pout = out + len - 2;
		while ((*pout-- = *tmpPtr--) && --i && (!(++j % 3) ? (*pout-- = ',') : true));
		while (!*out && out++);
	}
	return out;
}


//
TracyProfiler::~TracyProfiler(void)
{
}
