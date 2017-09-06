//
// gamepad.h: Header file
//
// by James Hammons
// (C) 2013 Underground Software
//

#ifndef __GAMEPAD_H__
#define __GAMEPAD_H__

#define JOY_KEY			0x000000
#define JOY_BUTTON		0x010000
#define JOY_HAT			0x020000
#define JOY_AXIS		0x040000

#define	JOY_TYPE_MASK		0xFF0000
#define JOY_BUTTON_MASK		0x00FFFF
#define JOY_HATNUM_MASK		0x0000F8
#define JOY_HATBUT_MASK		0x000007
#define JOY_AXISNUM_MASK	0x00FFFE
#define JOY_AXISDIR_MASK	0x000001

#include <stdint.h>
#include "SDL.h"

// buttonID is the combination of the type (BUTTON, HAT) and the button #
// (0-255 for buttons, 0-31 for hats). Hats also have 0-7 for a button #
// that corresponds to a direction.

class Gamepad
{
// really should make all methods and members be static so that we can
// call this stuff without instantiating one. :-) [DONE]
	public:
		Gamepad();
		~Gamepad();

		// Class methods...
		static void AllocateJoysticks(void);
		static void DeallocateJoysticks(void);
		static const char * GetJoystickName(int joystickID);
		static bool GetState(int joystickID, int buttonID);
		static int CheckButtonPressed(void);
		static int GetButtonID(void);
		static int GetJoystickID(void);
		static void Update(void);
		static void DumpJoystickStatesToLog(void);

		// Support up to 8 gamepads
		static int numJoysticks;
		static SDL_Joystick * pad[8];
		static char padName[8][128];
		static int numButtons[8];
		static int numAxes[8];
		static int numHats[8];
		static bool button[8][256];
		static uint8_t hat[8][32];
		static int32_t axis[8][32];
};

#endif	// __GAMEPAD_H__
