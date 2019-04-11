//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JPM  Sept./2016  Created this file, and added Soft debugger support
// JPM  10/09/2018  Added the source file search paths
// JPM  04/06/2019  Added ELF sections check
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

	private:
		QString CheckForTrailingSlash(QString s);

	public:
		QLineEdit *nbrdisasmlines;
		QLineEdit *sourcefilesearchpaths;
		QCheckBox *displayHWlabels;
		QCheckBox *disasmopcodes;
		QCheckBox *displayFullSourceFilename;
		QCheckBox *ELFSectionsCheck;
};

#endif	// __DEBUGGERTAB_H__
