//
// configdialog.h - Configuration dialog
//
// by James Hammons
// (C) 2010 Underground Software
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JPM  06/19/2016  Soft debugger support
// JPM  09/03/2018  Added a Models & Bios tab
// JPM  March/2022  Added an exceptions tab
//

#ifndef __CONFIGDIALOG_H__
#define __CONFIGDIALOG_H__

#define	NEWMODELSBIOSHANDLER				// New Jaguar models and bios usage handler

#include <QtWidgets/QtWidgets>

class GeneralTab;
#ifdef NEWMODELSBIOSHANDLER
class ModelsBiosTab;
#endif
class ControllerTab;
class AlpineTab;
class DebuggerTab;
class KeyBindingsTab;
class ExceptionsTab;


class ConfigDialog: public QDialog
{
	Q_OBJECT

	public:
		ConfigDialog(QWidget * parent = 0);
		~ConfigDialog();
		void UpdateVJSettings(void);

	private:
		void LoadDialogFromSettings(void);

	private:
		QTabWidget *tabWidget;
		QDialogButtonBox *buttonBox;

	public:
		GeneralTab *generalTab;
#ifdef NEWMODELSBIOSHANDLER
		ModelsBiosTab *modelsbiosTab;
#endif
		ControllerTab *controllerTab1;
//		ControllerTab *controllerTab2;
		KeyBindingsTab *keybindingsTab;
		AlpineTab *alpineTab;
		DebuggerTab *debuggerTab;
		ExceptionsTab *exceptionsTab;
};

#endif	// __CONFIGDIALOG_H__
