//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JPM  06/19/2016  Created this file
// JPM  06/19/2016  Soft debugger support
//

#ifndef __DEBUGGERTAB_H__
#define __DEBUGGERTAB_H__

#include <QtWidgets>

class DebuggerTab: public QWidget
{
	Q_OBJECT

	public:
		DebuggerTab(QWidget * parent = 0);
		~DebuggerTab();
		void SetSettings(void);
		void GetSettings(void);

	public:
		QLineEdit *edit3;

		QCheckBox *displayHWlabels;
		QCheckBox *disasmopcodes;
		QCheckBox *displayFullSourceFilename;
};

#endif	// __DEBUGGERTAB_H__
