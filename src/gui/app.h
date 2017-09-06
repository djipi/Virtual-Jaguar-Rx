//
// app.h: Header file
//
// by James Hammons
// (C) 2009 Underground Software
//

#ifndef __APP_H__
#define __APP_H__

//Hrm. uh??? I thought this wasn't the way to do this stuff...???
#include <QtWidgets>

// Forward declarations
class MainWin;

class App: public QApplication
{
	public:
		App(int & argc, char * argv[]);

	private:
		MainWin * mainWindow;
//		bool noUntunedTankPlease;

	// Globally accessible stuff goes here...
	// Although... Globally accessible stuff should go into settings.cpp...
	// Unless it's stuff related to the GUI, then it should go here. :-P
	// And we make these class variables so we don't have to muck around with
	// chasing down instances of the object...
//	public:
//		static QString filenameToRun;
};

#endif	// __APP_H__
