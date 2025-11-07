//
// tracyprofiler.cpp - Profiler with Tracy
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  10/29/2025       Created this file
// JPM   Oct./2025       Added pause feature, better control for the Tracy profiler feed and memory allocation tracking
// JPM   Nov./2025       Tracy profiler connection with a cancellable dialog, Lua initialization, and timer manipulations
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
#include "tracy\TracyLua.hpp"
#include "client\TracyProfiler.hpp"

// Define the Lua zone state that Tracy requires
namespace tracy
{
	LuaZoneState& GetLuaZoneState(void)
	{
		static LuaZoneState luaZoneState;
		return luaZoneState;
	}
}

//
#undef TracyLine
#undef TracyFunction
#undef TracyFile

// message colour codes
#define COLOUR_DEFAULT	0xFFFFFFFF
#define COLOUR_INFO		0xFFFFFF80
#define COLOUR_WARNING	0xFFFFA000
#define COLOUR_ERROR	0xFF8080FF
#define COLOUR_SUCCESS	0xFF00FF00
#define COLOUR_DEBUG	0xFF808080

// M68K variables for the Tracy profiler
constexpr double M68K_CLOCK_HZ = 13290000.0;				// 13.29 MHz
double NANOS_PER_CYCLE = 1000000000.0 / M68K_CLOCK_HZ;		// M68K @ 13.29 MHz: 1 cycle is more or less 75.244 nanoseconds
int64_t g_tracy_time_offset;
bool g_tracy_emulation;


//
TracyProfiler::TracyProfiler(void)
{
	tracyPaused = true;
	g_tracy_emulation = false;

	// name the application in Tracy, is displayed in the Trace information
	TracyCAppInfo("Virtual Jaguar Rx - Profiler", strlen("Virtual Jaguar Rx - Profiler"));
	//TracyCSetThreadName("M68K CPU");
	//TracyCPlotConfig("68000 cycles", TracyPlotFormatNumber, true, true, 0xFFFFFF80);
}


// Tracy profiler Lua initialization
void TracyProfiler::InitLua(lua_State* LuaLib)
{
	tracy::LuaRegister(LuaLib);
}


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
	bool flag = WaitForConnection();
	// get current Tracy profiler host timer
	Timer(true, false);
	// Tracy profiler is now emulating the 68000 CPU timing
	g_tracy_emulation = true;
	// displayed the message in the Tracy profiler messages list
	TracyCMessageC("=== Atari Jaguar: 68000  ===", 28, COLOUR_DEFAULT);
	return flag;
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
// true: pause, false: unpause
void TracyProfiler::Pause(bool pause)
{
	tracyPaused = !pause;
}


// Reset the Tracy index context
void TracyProfiler::RAZIndex(void* index)
{
	// cast the index context pointer
	TracyCZoneCtx* ctx = (TracyCZoneCtx*)index;
	// reset the context
	*ctx = { 0 };
}


// Add a malloc event for the Tracy memory allocation
// null-pointer is not recorded in Tracy
void TracyProfiler::M68Kmalloc(void* zoneCtx, size_t ptr, size_t size, int depth)
{
	if (!tracyPaused)
	{
		// cast the zone context pointer
		TracyCZoneCtx* ctx = (TracyCZoneCtx*)zoneCtx;
		//
		char buffer[128];
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
			ctx->active = (depth >= 0) ? TracyCAlloc((void*)ptr, size), (ctx->id = depth), true : false;
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
	if (!tracyPaused || flush)
	{
		//
		char buffer[128];
		// cast the zone context pointer
		TracyCZoneCtx* ctx = (TracyCZoneCtx*)zoneCtx;
		if (ctx >= 0)
		{
			// display message for a null or an existing pointer
			if (!flush)
			{
				sprintf(buffer, "M68K free: ptr=0x%06x", (unsigned int)ptr);
				unsigned int colour = ptr ? COLOUR_SUCCESS : COLOUR_WARNING;
				TracyCMessageC(buffer, strlen(buffer), colour);
			}
			// free the non-null pointer address in the memory allocation
			(ptr && ctx && ctx->active) ? TracyCFree((void*)ptr), (ctx->id = 0), (ctx->active = false) : false;
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
	// cast the zone context pointer
	TracyCZoneCtx* zone = (TracyCZoneCtx*)zoneCtx;
	// start the function zone only if not paused
	if (!tracyPaused)
	{
#if 0
#else
		// clear the source file & line wording dedicated to the function name
		unsigned int TracyLine = 0;
		char* TracyFile = (char*)"";
		// colour for the function's name zone
		TracyCZoneC(ctx, 0xBEBE70, true);
		// display the function's name
		TracyCZoneName(ctx, functionName, strlen(functionName));
		//
		*zone = ctx;
#endif
		// start cycle plot
		TracyCPlot("M68K start cycle", (double)startCycle);
	}
	else
	{
		RAZIndex(zoneCtx);
	}
}


// Return Tracy zone activity for the 68000 function
bool TracyProfiler::M68Kactive(void* zoneCtx)
{
	// cast the zone context pointer
	TracyCZoneCtx* zone = (TracyCZoneCtx*)zoneCtx;
	return (zone->active);
}


// End the Tracy zone for the 68000 function
// The plot colors will depend on the plot's name (yellow)
void TracyProfiler::M68Kleave(void* zoneCtx, size_t usedCycles)
{
	// cast the zone context pointer
	TracyCZoneCtx* zone = (TracyCZoneCtx*)zoneCtx;
	// end the function zone
	if (!tracyPaused || (zone->id && zone->active))
	{
		char text[64];
		char textWithCommas[64];

		// display the used cycles in the function zone label
		snprintf(text, sizeof(text), "%s cycles", IntegerToStringWithCommas(textWithCommas, sizeof(textWithCommas), usedCycles));
		TracyCZoneText(*zone, text, strlen(text));
		// used cycles plots
		TracyCPlot("M68K cycles per function", (double)usedCycles);
		TracyCPlot("M68K function time (\xC2\xB5s)", ((double)usedCycles / M68K_CLOCK_HZ) * 1e6);
		TracyCPlot("M68K function time (ms)", ((double)usedCycles / M68K_CLOCK_HZ) * 1e3);
		//
		TracyCZoneEnd(*zone);
		RAZIndex(zoneCtx);
	}
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
