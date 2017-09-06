#ifndef __PROFILE_H__
#define __PROFILE_H__

#include <stdint.h>

class QComboBox;
class QSettings;

#define MAX_PROFILES  64		// 64 profiles ought to be enough for everybody
#define CONTROLLER1   0x01
#define CONTROLLER2   0x02


struct Profile
{
	int device;					// Host device number (-1 == invalid profile)
	char mapName[32];			// Human readable map name
	int preferredSlot;			// CONTROLLER1 and/or CONTROLLER2
	uint32_t map[21];			// Keys/buttons/axes
};


// Function prototypes
void SaveProfiles(void);
void RestoreProfiles(void);
void ReadProfiles(QSettings *);
void WriteProfiles(QSettings *);
int GetFreeProfile(void);
void DeleteProfile(int);
int FindDeviceNumberForName(const char *);
int FindMappingsForDevice(int, QComboBox *);
int FindUsableProfiles(QComboBox *);
bool ConnectProfileToController(int, int);
void AutoConnectProfiles(void);


// Exported variables
extern Profile profile[];
extern int controller1Profile;
extern int controller2Profile;
extern int gamepadIDSlot1;
extern int gamepadIDSlot2;
//extern int numberOfProfiles;

#endif	// __PROFILE_H__

