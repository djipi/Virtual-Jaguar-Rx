//
// profile.cpp - Global profile storage/definition/manipulation
//
// by James Hammons
// (C) 2013 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  05/01/2013  Created this file
// JLH  10/02/2014  Finally fixed stuff so it works the way it should
// JPM  06/06/2016  Visual Studio support
//
// This is a profile database with two parts: One, a list of devices, and two,
// a list of profiles each containing a pointer to the device list, and map
// name, a preferred slot #, and a key/button map. All the heavy lifting (incl.
// autoconnection of devices to profiles to slots) is done here.
//
// Basically, how this works is that we connect the device the user plugs into
// the computer to a profile in the database to a slot in the virtual Jaguar.
// Hopefully the configuration that the user gives us is sane enough for us to
// figure out how to do the right thing! By default, there is always a keyboard
// device plugged in; any other device that gets plugged in and wants to be in
// slot #0 can override it. This is so there is always a sane configuration if
// nothing is plugged in.
//
// Devices go into the database when the user plugs them in and runs VJ, and
// subsequently does anything to alter any of the existing profiles. Once a
// device has been seen, it can't be unseen!
//

#include "profile.h"
#include <QtWidgets>
#include "gamepad.h"
#include "log.h"
#include "settings.h"


//#define DEBUG_PROFILES
#define MAX_DEVICES  64


Profile profile[MAX_PROFILES];
Profile profileBackup[MAX_PROFILES];
int controller1Profile;
int controller2Profile;
int gamepadIDSlot1;
int gamepadIDSlot2;
int numberOfProfiles;
int numberOfDevices;
char deviceNames[MAX_DEVICES][128];
static int numberOfProfilesSave;

// This is so that new devices have something reasonable to show for default
uint32_t defaultMap[21] = {
	'S', 'X', 'Z', 'C', '-','7', '4', '1', '0', '8', '5', '2', '=', '9', '6',
	'3', 'L', 'K', 'J', 'O', 'P'
};


// Function Prototypes
int ConnectProfileToDevice(int deviceNum, int gamepadID = -1);
int FindProfileForDevice(int deviceNum, int preferred, int * found);


//
// These two functions are mainly to support the controller configuration GUI.
// Could just as easily go there as well (and be better placed there).
//
void SaveProfiles(void)
{
	numberOfProfilesSave = numberOfProfiles;
	memcpy(&profileBackup, &profile, sizeof(Profile) * MAX_PROFILES);
}


void RestoreProfiles(void)
{
	memcpy(&profile, &profileBackup, sizeof(Profile) * MAX_PROFILES);
	numberOfProfiles = numberOfProfilesSave;
}


void ReadProfiles(QSettings * set)
{
	// Assume no profiles, until we read them
	numberOfProfiles = 0;

	// There is always at least one device present, and it's the keyboard
	// (hey, we're PC centric here ;-)
	numberOfDevices = 1;
	strcpy(deviceNames[0], "Keyboard");

	// Read the rest of the devices (if any)
	numberOfDevices += set->beginReadArray("devices");

	for(int i=1; i<numberOfDevices; i++)
	{
		set->setArrayIndex(i - 1);
		strcpy(deviceNames[i], set->value("deviceName").toString().toUtf8().data());
#ifdef DEBUG_PROFILES
printf("Read device name: %s\n", deviceNames[i]);
#endif
	}

	set->endArray();
	numberOfProfiles = set->beginReadArray("profiles");
#ifdef DEBUG_PROFILES
printf("Number of profiles: %u\n", numberOfProfiles);
#endif

	for(int i=0; i<numberOfProfiles; i++)
	{
		set->setArrayIndex(i);
		profile[i].device = set->value("deviceNum").toInt();
		strcpy(profile[i].mapName, set->value("mapName").toString().toUtf8().data());
		profile[i].preferredSlot = set->value("preferredSlot").toInt();

		for(int j=0; j<21; j++)
		{
			QString string = QString("map%1").arg(j);
			profile[i].map[j] = set->value(string).toInt();
		}
#ifdef DEBUG_PROFILES
printf("Profile #%u: device=%u (%s)\n", i, profile[i].device, deviceNames[profile[i].device]);
#endif
	}

	set->endArray();

#ifdef DEBUG_PROFILES
printf("Number of profiles found: %u\n", numberOfProfiles);
#endif
	// Set up a reasonable default if no profiles were found
	if (numberOfProfiles == 0)
	{
#ifdef DEBUG_PROFILES
printf("Setting up default profile...\n");
#endif
		numberOfProfiles++;
		profile[0].device = 0;	// Keyboard is always device #0
		strcpy(profile[0].mapName, "Default");
		profile[0].preferredSlot = CONTROLLER1;

		for(int i=0; i<21; i++)
			profile[0].map[i] = defaultMap[i];
	}
}


void WriteProfiles(QSettings * set)
{
#if 0
	// Don't write anything for now...
	return;
#endif
	// NB: Should only do this if something changed; otherwise, no need to do
	//     this.
	set->beginWriteArray("devices");

	for(int i=1; i<numberOfDevices; i++)
	{
		set->setArrayIndex(i - 1);
		set->setValue("deviceName", deviceNames[i]);
	}

	set->endArray();
	set->beginWriteArray("profiles");

	for(int i=0; i<numberOfProfiles; i++)
	{
		set->setArrayIndex(i);
		set->setValue("deviceNum", profile[i].device);
		set->setValue("mapName", profile[i].mapName);
		set->setValue("preferredSlot", profile[i].preferredSlot);

		for(int j=0; j<21; j++)
		{
			QString string = QString("map%1").arg(j);
			set->setValue(string, profile[i].map[j]);
		}
	}

	set->endArray();
}


int GetFreeProfile(void)
{
	// Check for too many, return -1 if so
	if (numberOfProfiles == MAX_PROFILES)
		return -1;

	int profileNum = numberOfProfiles;
	numberOfProfiles++;
	return profileNum;
}


void DeleteProfile(int profileToDelete)
{
	// Sanity check
	if (profileToDelete >= numberOfProfiles)
		return;

	// Trivial case: Profile at end of the array
	if (profileToDelete == (numberOfProfiles - 1))
	{
		numberOfProfiles--;
		return;
	}

//	memmove(dest, src, bytesToMove);
	memmove(&profile[profileToDelete], &profile[profileToDelete + 1], ((numberOfProfiles - 1) - profileToDelete) * sizeof(Profile));
	numberOfProfiles--;
}


int FindDeviceNumberForName(const char * name)
{
	for(int i=0; i<numberOfDevices; i++)
	{
		if (strcmp(deviceNames[i], name) == 0)
#ifdef DEBUG_PROFILES
{
printf("PROFILE: Found device #%i for name (%s)...\n", i, name);
#endif
			return i;
#ifdef DEBUG_PROFILES
}
#endif
	}

	if (numberOfDevices == MAX_DEVICES)
		return -1;

#ifdef DEBUG_PROFILES
printf("Device '%s' not found, creating device...\n", name);
#endif
	// If the device wasn't found, it must be new; so add it to the list.
	int deviceNum = numberOfDevices;
	deviceNames[deviceNum][127] = 0;
	strncpy(deviceNames[deviceNum], name, 127);
	numberOfDevices++;

	return deviceNum;
}


int FindMappingsForDevice(int deviceNum, QComboBox * combo)
{
	int found = 0;

	for(int i=0; i<numberOfProfiles; i++)
	{
//This should *never* be the case--all profiles in list are *good*
//		if (profile[i].device == -1)
//			continue;

		if (profile[i].device == deviceNum)
		{
			combo->addItem(profile[i].mapName, i);
			found++;
		}
	}

	// If no mappings were found, create a default one for it
	if (found == 0)
	{
		profile[numberOfProfiles].device = deviceNum;
		strcpy(profile[numberOfProfiles].mapName, "Default");
		profile[numberOfProfiles].preferredSlot = CONTROLLER1;

		for(int i=0; i<21; i++)
			profile[numberOfProfiles].map[i] = defaultMap[i];

		combo->addItem(profile[numberOfProfiles].mapName, numberOfProfiles);
		numberOfProfiles++;
		found++;
	}

	return found;
}


// N.B.: Unused
int FindUsableProfiles(QComboBox * combo)
{
	int found = 0;

	// Check for device #0 (keyboard) profiles first
	for(int j=0; j<numberOfProfiles; j++)
	{
		// Check for device *and* usable configuration
		if ((profile[j].device == 0) && (profile[j].preferredSlot))
		{
			combo->addItem(QString("Keyboard::%1").arg(profile[j].mapName), j);
			found++;
		}
	}

	// Check for connected host devices next
	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));

		for(int j=0; j<numberOfProfiles; j++)
		{
			if ((profile[j].device == deviceNum) && (profile[j].preferredSlot))
			{
				combo->addItem(QString("%1::%2").arg(Gamepad::GetJoystickName(i)).arg(profile[j].mapName), j);
				found++;
			}
		}
	}

	return found;
}


bool ConnectProfileToController(int profileNum, int controllerNum)
{
	// Sanity checks...
	if (profileNum < 0)
		return false;

	if (profile[profileNum].device == -1)
		return false;

	if (controllerNum < 0 || controllerNum > 2)
		return false;

	uint32_t * dest = (controllerNum == 0 ? &vjs.p1KeyBindings[0] : &vjs.p2KeyBindings[0]);

	for(int i=0; i<21; i++)
		dest[i] = profile[profileNum].map[i];

	WriteLog("PROFILE: Successfully mapped device '%s' (%s) to controller #%u...\n", deviceNames[profile[profileNum].device], profile[profileNum].mapName, controllerNum);
	return true;
}


/*
One more stab at this...

 -  Connect keyboard to slot #0.
 -  Loop thru all connected devices. For each device:
    -  Grab all profiles for the device. For each profile:
       -  Check to see what its preferred device is.
       -  If PD is slot #0, see if slot is already taken (gamepadIDSlot1 != -1).
          If not taken, take it; otherwise put in list to tell user to solve the
          conflict for us.
          -  If the slot is already taken and *it's the same device* as the one
             we're looking at, set it in slot #1.
       -  If PD is slot #1, see if slot is already taken. If not, take it;
          otherwise, put in list to tell user to solve conflict for us.
       -  If PD is slot #0 & #1, see if either is already taken. Try #0 first,
          then try #1. If both are already taken, skip it. Do this *after* we've
          connected devices with preferred slots.
*/
void AutoConnectProfiles(void)
{
//	int foundProfiles[MAX_PROFILES];
	controller1Profile = -1;
	controller2Profile = -1;
	gamepadIDSlot1 = -1;
	gamepadIDSlot2 = -1;

	// Connect the keyboard automagically only if no gamepads are plugged in.
	// Otherwise, check after all other devices have been checked, then try to
	// add it in.
	if (Gamepad::numJoysticks == 0)
	{
#ifdef DEBUG_PROFILES
printf("AutoConnect: Setting up keyboard...\n");
#endif
//NO!		ConnectProfileToDevice(0);
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to set up scanning for multiple keyboard profiles !!!")
#else
#warning "!!! Need to set up scanning for multiple keyboard profiles !!!"
#endif // _MSC_VER
		ConnectProfileToController(0, 0);
		return;
	}

	// Connect the profiles that prefer a slot, if any.
	// N.B.: Conflicts are detected, but ignored. 1st controller to grab a
	//       preferred slot gets it. :-P
	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));
//		bool p1Overwriteable = 
#ifdef DEBUG_PROFILES
printf("AutoConnect: Attempting to set up profile for device '%s' (%i)\n", Gamepad::GetJoystickName(i), deviceNum);
#endif

		for(int j=0; j<numberOfProfiles; j++)
		{
			if (deviceNum != profile[j].device)
				continue;

			int slot = profile[j].preferredSlot;

			if (slot == CONTROLLER1)
			{
				if (gamepadIDSlot1 == -1)
					controller1Profile = j, gamepadIDSlot1 = i;
				else
				{
					// Autoresolve simple conflict: two controllers sharing one
					// profile mapped to slot #0.
					if ((deviceNum == profile[controller1Profile].device) && (controller2Profile == -1))
						controller2Profile = j, gamepadIDSlot2 = i;
					else
						; // Alert user to conflict and ask to resolve
				}
			}
			else if (slot == CONTROLLER2)
			{
				if (gamepadIDSlot2 == -1)
					controller2Profile = j, gamepadIDSlot2 = i;
				else
				{
					// Autoresolve simple conflict: two controllers sharing one
					// profile mapped to slot #1.
					if ((deviceNum == profile[controller2Profile].device) && (controller1Profile == -1))
						controller1Profile = j, gamepadIDSlot1 = i;
					else
						; // Alert user to conflict and ask to resolve
				}
			}
		}
	}

	// Connect the "don't care" states, if any. We don't roll it into the above,
	// because it can override the profiles that have a definite preference.
	// These should be lowest priority.
	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));

		for(int j=0; j<numberOfProfiles; j++)
		{
			if (deviceNum != profile[j].device)
				continue;

			int slot = profile[j].preferredSlot;

			if (slot == (CONTROLLER1 | CONTROLLER2))
			{
				if (gamepadIDSlot1 == -1)
					controller1Profile = j, gamepadIDSlot1 = i;
				else if (gamepadIDSlot2 == -1)
					controller2Profile = j, gamepadIDSlot2 = i;
			}
		}
	}

	// Connect the keyboard device (lowest priority)
	// N.B.: The keyboard is always mapped to profile #0, so we can locate it
	//       easily. :-)
	int slot = profile[0].preferredSlot;
#ifdef DEBUG_PROFILES
printf("AutoConnect: Attempting to connect keyboard... (gamepadIDSlot1/2 = %i/%i)\n", gamepadIDSlot1, gamepadIDSlot2);
#endif

	if ((slot == CONTROLLER1) && (gamepadIDSlot1 == -1))
		controller1Profile = 0;
	else if ((slot == CONTROLLER2) && (gamepadIDSlot2 == -1))
		controller2Profile = 0;
	else if (slot == (CONTROLLER1 | CONTROLLER2))
	{
		if (gamepadIDSlot1 == -1)
			controller1Profile = 0;
		else if (gamepadIDSlot2 == -1)
			controller2Profile = 0;
	}

#ifdef DEBUG_PROFILES
printf("AutoConnect: Profiles found: [%i, %i]\n", controller1Profile, controller2Profile);
#endif
	// Finally, attempt to connect profiles to controllers
	ConnectProfileToController(controller1Profile, 0);
	ConnectProfileToController(controller2Profile, 1);
}


//unused...
int ConnectProfileToDevice(int deviceNum, int gamepadID/*= -1*/)
{
//	bool found1 = false;
//	bool found2 = false;
	int numberFoundForController1 = 0;
	int numberFoundForController2 = 0;

	for(int i=0; i<numberOfProfiles; i++)
	{
		// Skip profile if it's not our device
		if (profile[i].device != deviceNum)
			continue;

		if (profile[i].preferredSlot & CONTROLLER1)
		{
			controller1Profile = i;
			gamepadIDSlot1 = gamepadID;
//			found1 = true;
			numberFoundForController1++;
		}

		if (profile[i].preferredSlot & CONTROLLER2)
		{
			controller2Profile = i;
			gamepadIDSlot2 = gamepadID;
//			found2 = true;
			numberFoundForController2++;
		}
	}

//	return found;
	return numberFoundForController1 + numberFoundForController2;
}


// N.B.: Unused
int FindProfileForDevice(int deviceNum, int preferred, int * found)
{
	int numFound = 0;

	for(int i=0; i<numberOfProfiles; i++)
	{
		// Return the profile only if it matches the passed in device and
		// matches the passed in preference...
		if ((profile[i].device == deviceNum) && (profile[i].preferredSlot == preferred))
			found[numFound++] = i;
	}

	return numFound;
}


//
// Also note that we have the intersection of three things here: One the one
// hand, we have the detected joysticks with their IDs (typically in the range
// of 0-7), we have our gamepad profiles and their IDs (typically can have up to
// 64 of them), and we have our gamepad slots that the detected joysticks can be
// connected to.
//
// So, when the user plugs in a gamepad, it gets a joystick ID, then the profile
// manager checks to see if a profile (or profiles) for it exists. If so, then
// it assigns that joystick ID to a gamepad slot, based upon what the user
// requested for that profile.
//
// A problem (perhaps) arises when you have more than one profile for a certain
// device, how do you know which one to use? Perhaps you have a field in the
// profile saying that you use this profile 1st, that one 2nd, and so on...
//
// Some use cases, and how to resolve them:
//
// - User has two of the same device, and plugs them both in. There is only one
//   profile. In this case, the sane thing to do is ignore the "preferred slot"
//   of the dialog and use the same profile for both controllers, and plug them
//   both into slot #0 and #1.
// - User has one device, and plugs it in. There are two profiles. In this case,
//   the profile chosen should be based upon the "preferred slot", with slot #0
//   being the winner. If both profiles are set for slot #0, ask the user which
//   profile to use, and set a flag in the profile to say that it is a preferred
//   profile for that device.
// - In any case where there are conflicts, the user must be consulted and sane
//   defaults used.
//
