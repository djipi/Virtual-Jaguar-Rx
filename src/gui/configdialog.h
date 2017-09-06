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
//

#ifndef __CONFIGDIALOG_H__
#define __CONFIGDIALOG_H__

#include <QtWidgets>

class GeneralTab;
class ControllerTab;
class AlpineTab;
class DebuggerTab;

class ConfigDialog: public QDialog
{
	Q_OBJECT

	public:
		ConfigDialog(QWidget * parent = 0);
		~ConfigDialog();
		void UpdateVJSettings(void);

	private:
		void LoadDialogFromSettings(void);
		QString CheckForTrailingSlash(QString);

	private:
		QTabWidget * tabWidget;
		QDialogButtonBox * buttonBox;

	public:
		GeneralTab * generalTab;
		ControllerTab * controllerTab1;
//		ControllerTab * controllerTab2;
		AlpineTab * alpineTab;
		DebuggerTab * debuggerTab;
};

#endif	// __CONFIGDIALOG_H__
