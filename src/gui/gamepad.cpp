//
// gamepad.cpp - Host joystick handling (using SDL)
//
// by James Hammons
// (C) 2013 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  01/05/2013  Created this file
//

#include "gamepad.h"
#include "log.h"


// Class member initialization
/*static*/ int Gamepad::numJoysticks = 0;
/*static*/ SDL_Joystick * Gamepad::pad[8];
/*static*/ char Gamepad::padName[8][128];
/*static*/ int Gamepad::numButtons[8];
/*static*/ int Gamepad::numHats[8];
/*static*/ int Gamepad::numAxes[8];
/*static*/ bool Gamepad::button[8][256];
/*static*/ uint8_t Gamepad::hat[8][32];
/*static*/ int32_t Gamepad::axis[8][32];


Gamepad::Gamepad(void)//: numJoysticks(0)
{
	AllocateJoysticks();
}


Gamepad::~Gamepad(void)
{
	DeallocateJoysticks();
}


void Gamepad::AllocateJoysticks(void)
{
//	DeallocateJoysticks();
	numJoysticks = SDL_NumJoysticks();

	// Sanity check
	if (numJoysticks > 8)
		numJoysticks = 8;

	for(int i=0; i<numJoysticks; i++)
	{
		pad[i] = SDL_JoystickOpen(i);
		numButtons[i] = numHats[i] = numAxes[i] = 0;
		// We need to copy the contents of this pointer, as SDL will change it
		// willy nilly to suit itself
//		padName[i] = SDL_JoystickName(i);
		strncpy(padName[i], SDL_JoystickName(i), 127);
		padName[i][127] = 0;	// Just in case name's length > 127

		if (pad[i])
		{
			numButtons[i] = SDL_JoystickNumButtons(pad[i]);
			numHats[i] = SDL_JoystickNumHats(pad[i]);
			numAxes[i] = SDL_JoystickNumAxes(pad[i]);
			WriteLog("Gamepad: Joystick #%i: %s\n", i, padName[i]);

			// Ick, kludges already!!!
			if (strcmp(padName[i], "Sony PLAYSTATION(R)3 Controller") == 0)
			{
				// We do this because these axes stay stuck on -32767 (buttons)
				// or start at 0 and stay stuck at -32767 (D-pad). :-P
				numAxes[i] = 8;
				WriteLog("Gamepad: Blacklisting PS3 controller axes 8 on up...\n");
			}
		}
	}

	WriteLog("Gamepad: Found %u joystick%s.\n", numJoysticks, (numJoysticks == 1 ? "" : "s"));
#if 0
for(int i=0; i<numJoysticks; i++)
	printf("GAMEPAD::AllocateJoysticks: stick #%i = %s\n", i, padName[i]);
#endif
}


void Gamepad::DeallocateJoysticks(void)
{
	for(int i=0; i<numJoysticks; i++)
		SDL_JoystickClose(pad[i]);
}


const char * Gamepad::GetJoystickName(int joystickID)
{
	// Sanity check
	if (joystickID >= 8)
		return NULL;

//printf("GAMEPAD: Getting name (%s) for joystick #%i...\n", padName[joystickID], joystickID);
	return padName[joystickID];
}


bool Gamepad::GetState(int joystickID, int buttonID)
{
	uint8_t hatMask[8] = { 1, 2, 4, 8, 16, 32, 64, 128 };

	if (buttonID & JOY_BUTTON)
	{
		// Handle SDL button
		int buttonNum = (buttonID & JOY_BUTTON_MASK);
		return button[joystickID][buttonNum];
	}
	else if (buttonID & JOY_HAT)
	{
		// Handle SDL hats
		int hatNumber = (buttonID & JOY_HATNUM_MASK) >> 3;
		uint8_t hatDirection = hatMask[buttonID & JOY_HATBUT_MASK];
		return (hat[joystickID][hatNumber] & hatDirection ? true : false);
	}
	else if (buttonID & JOY_AXIS)
	{
		int axisNum = (buttonID & JOY_AXISNUM_MASK) >> 1;
		int direction = (buttonID & JOY_AXISDIR_MASK);
//printf("Checking pad #%u axis %u: axis = %i, direction = %u\n", joystickID, axisNum, axis[joystickID][axisNum], direction);

		if (axis[joystickID][axisNum] != 0)
		{
			if ((axis[joystickID][axisNum] > 32000) && (direction == 0))
//{
//printf("Axis + hit!\n");
				return true;
//}

			if ((axis[joystickID][axisNum] < -32000) && (direction == 1))
//{
//printf("Axis - hit!\n");
				return true;
//}
		}
	}

	// Default == failure
	return false;
}


int Gamepad::CheckButtonPressed(void)
{
	DumpJoystickStatesToLog();

	// This translates the hat direction to a mask index.
	int hatNum[16] = { -1, 0, 1, -1, 2, -1, -1, -1,
		3, -1, -1, -1, -1, -1, -1, -1 };

	// Return single button ID being pressed (if any)
	for(int i=0; i<numJoysticks; i++)
	{
		for(int j=0; j<numButtons[i]; j++)
		{
			if (button[i][j])
				return (JOY_BUTTON | j);
		}

		for(int j=0; j<numHats[i]; j++)
		{
			if (hat[i][j])
				return (JOY_HAT | hatNum[hat[i][j]]);
		}

		for(int j=0; j<numAxes[i]; j++)
		{
			// We encode these as axis # (in bits 1-15), up or down in bit 0.
//			if (axis[i][j] > 0)
			if (axis[i][j] > 32000)
				return (JOY_AXIS | (j << 1) | 0);

//			if (axis[i][j] < 0)
			if (axis[i][j] < -32000)
				return (JOY_AXIS | (j << 1) | 1);
		}
	}

	return -1;
}


// UNUSED
int Gamepad::GetButtonID(void)
{
	// Return single button ID being pressed (if any)
	return -1;
}


// UNUSED
int Gamepad::GetJoystickID(void)
{
	// Return joystick ID of button being pressed (if any)
	return -1;
}


void Gamepad::Update(void)
{
//	SDL_PollEvent(&event);
	SDL_JoystickUpdate();

	for(int i=0; i<numJoysticks; i++)
	{
		for(int j=0; j<numButtons[i]; j++)
			button[i][j] = SDL_JoystickGetButton(pad[i], j);

		for(int j=0; j<numHats[i]; j++)
			hat[i][j] = SDL_JoystickGetHat(pad[i], j);

		for(int j=0; j<numAxes[i]; j++)
			axis[i][j] = SDL_JoystickGetAxis(pad[i], j);
	}
}


void Gamepad::DumpJoystickStatesToLog(void)
{
	bool pressed = false;

	for(int i=0; i<numJoysticks; i++)
	{
		for(int j=0; j<numButtons[i]; j++)
		{
			if (button[i][j])
			{
				pressed = true;
				break;
				break;
			}
		}

		for(int j=0; j<numHats[i]; j++)
		{
			if (hat[i][j])
			{
				pressed = true;
				break;
				break;
			}
		}

		for(int j=0; j<numAxes[i]; j++)
		{
			// We encode these as axis # (in bits 1-15), up or down in bit 0.
			if (axis[i][j] > 32000)
			{
				pressed = true;
				break;
				break;
			}

			if (axis[i][j] < -32000)
			{
				pressed = true;
				break;
				break;
			}
		}
	}

	if (!pressed)
		return;

	WriteLog("Gamepad: CheckButtonPressed...\n");

	for(int i=0; i<numJoysticks; i++)
	{
		for(int j=0; j<numButtons[i]; j++)
		{
			if (button[i][j])
				WriteLog("Gamepad: Pad #%i, button %i down...\n", i, j);
		}

		for(int j=0; j<numHats[i]; j++)
		{
			if (hat[i][j])
				WriteLog("Gamepad: Pad #%i, hat %i pushed...\n", i, j);
		}

		for(int j=0; j<numAxes[i]; j++)
		{
			// We encode these as axis # (in bits 1-15), up or down in bit 0.
			if (axis[i][j] > 32000)
				WriteLog("Gamepad: Pad #%i, axis %i pushed down...\n", i, j);

			if (axis[i][j] < -32000)
				WriteLog("Gamepad: Pad #%i, axis %i pushed up...\n", i, j);
		}
	}
}


// However, SDL 2 *does* support hot-plugging! :-D
#if 0
// Need to test this. It may be that the only time joysticks are detected is
// when the program is first run. That would suck.
// Well, it turns out that SDL doesn't do hot plugging. :-(
void Gamepad::CheckConsistency(void)
{
	int currentNumJoysticks = SDL_NumJoysticks();

	// Check to see if the # of joysticks reported by SDL changed
	if (currentNumJoysticks == numJoysticks)
		return;

	// Either one or more joysticks were plugged in, or removed. Fix up our
	// internal states to reflect this.

	
}
#endif

