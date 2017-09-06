//
// OS agnostic CDROM interface functions
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  01/16/2010  Created this log ;-)
// JPM  06/15/2016  Visual Studio support
//

//
// This now uses the supposedly cross-platform libcdio to do the necessary
// low-level CD twiddling we need that libsdl can't do currently. Jury is
// still out on whether or not to make this a conditional compilation or not.
//

// Comment this out if you don't have libcdio installed
// (Actually, this is defined in the Makefile to prevent having to edit
//  things too damn much. Jury is still out whether or not to make this
//  change permanent.)
//#define HAVE_LIB_CDIO

#include "cdintf.h"								// Every OS has to implement these

#ifdef HAVE_LIB_CDIO
#include <cdio/cdio.h>							// Now using OS agnostic CD access routines!
#endif
#include "log.h"


/*
static void TestCDIO(void)
{
	// See what (if anything) is installed.
	CdIo_t * p_cdio = cdio_open(0, DRIVER_DEVICE);
	driver_id_t driver_id;

	if (p_cdio != NULL)
	{
		WriteLog("CDIO: The driver selected is %s.\n", cdio_get_driver_name(p_cdio));
		WriteLog("CDIO: The default device for this driver is %s.\n\n", cdio_get_default_device(p_cdio));
		cdio_destroy(p_cdio);
	}
	else
	{
		WriteLog("CDIO: A suitable CD-ROM driver was not found.\n\n");
	}
}
*/

//
// *** OK, here's where we're going to attempt to put the platform agnostic CD interface ***
//

#ifdef HAVE_LIB_CDIO
static CdIo_t * cdioPtr = NULL;
#endif


bool CDIntfInit(void)
{
#ifdef HAVE_LIB_CDIO
	cdioPtr = cdio_open(NULL, DRIVER_DEVICE);

	if (cdioPtr == NULL)
	{
#endif
		WriteLog("CDINTF: No suitable CD-ROM driver found.\n");
		return false;
#ifdef HAVE_LIB_CDIO
	}

	WriteLog("CDINTF: Successfully opened CD-ROM interface.\n");

	return true;
#endif
}


void CDIntfDone(void)
{
	WriteLog("CDINTF: Shutting down CD-ROM subsystem.\n");

#ifdef HAVE_LIB_CDIO
	if (cdioPtr)
		cdio_destroy(cdioPtr);
#endif
}


bool CDIntfReadBlock(uint32_t sector, uint8_t * buffer)
{
#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! CDIntfReadBlock not implemented!")
#else
#warning "!!! FIX !!! CDIntfReadBlock not implemented!"
#endif // _MSC_VER
	// !!! FIX !!!
	WriteLog("CDINTF: ReadBlock unimplemented!\n");
	return false;
}


uint32_t CDIntfGetNumSessions(void)
{
#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! CDIntfGetNumSessions not implemented!")
#else
#warning "!!! FIX !!! CDIntfGetNumSessions not implemented!"
#endif // _MSC_VER
	// !!! FIX !!!
	// Still need relevant code here... !!! FIX !!!
	return 2;
}


void CDIntfSelectDrive(uint32_t driveNum)
{
#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! CDIntfSelectDrive not implemented!")
#else
#warning "!!! FIX !!! CDIntfSelectDrive not implemented!"
#endif // _MSC_VER
	// !!! FIX !!!
	WriteLog("CDINTF: SelectDrive unimplemented!\n");
}


uint32_t CDIntfGetCurrentDrive(void)
{
#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! CDIntfGetCurrentDrive not implemented!")
#else
#warning "!!! FIX !!! CDIntfGetCurrentDrive not implemented!"
#endif // _MSC_VER
	// !!! FIX !!!
	WriteLog("CDINTF: GetCurrentDrive unimplemented!\n");
	return 0;
}


const uint8_t * CDIntfGetDriveName(uint32_t driveNum)
{
#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! CDIntfGetDriveName driveNum is currently ignored!")
#else
#warning "!!! FIX !!! CDIntfGetDriveName driveNum is currently ignored!"
#endif // _MSC_VER
	// driveNum is currently ignored... !!! FIX !!!

#ifdef HAVE_LIB_CDIO
	uint8_t * driveName = (uint8_t *)cdio_get_default_device(cdioPtr);
	WriteLog("CDINTF: The drive name for the current driver is %s.\n", driveName);

	return driveName;
#else
	return (uint8_t *)"NONE";
#endif
}


uint8_t CDIntfGetSessionInfo(uint32_t session, uint32_t offset)
{
#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! CDIntfGetSessionInfo not implemented!")
#else
#warning "!!! FIX !!! CDIntfGetSessionInfo not implemented!"
#endif // _MSC_VER
	// !!! FIX !!!
	WriteLog("CDINTF: GetSessionInfo unimplemented!\n");
	return 0xFF;
}


uint8_t CDIntfGetTrackInfo(uint32_t track, uint32_t offset)
{
#ifdef _MSC_VER
#pragma message("Warning: !!! FIX !!! CDIntfTrackInfo not implemented!")
#else
#warning "!!! FIX !!! CDIntfTrackInfo not implemented!"
#endif // _MSC_VER
	// !!! FIX !!!
	WriteLog("CDINTF: GetTrackInfo unimplemented!\n");
	return 0xFF;
}

