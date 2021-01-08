//
// m68kDasmWin.h: M68K disassembly window
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  06/27/2016  Created this file
// JPM   Aug./2020  Added different layouts, and a status bar
//

#ifndef __M68KDASMWIN_H__
#define __M68KDASMWIN_H__

#define MD_LAYOUTTEXTS						// Use a layout with just texts otherwise the new layout is used
#ifdef MD_LAYOUTTEXTS
#define MD_LAYOUTFILE	1					// Display the filenames (1: display only the first filename)
#else
#define MD_LAYOUTFILE	1					// Must display only the first filename
#endif

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class m68KDasmWindow: public QWidget
{
	Q_OBJECT

	public:
		m68KDasmWindow(QWidget * parent = 0);

	public slots:
		void	RefreshContents(void);
		void	SetAddress(int address);
		void	Use68KPCAddress(void);

	protected:

	private:
		QVBoxLayout *layout;
#if MD_LAYOUTFILE == 1
		QStatusBar *statusbar;
#endif
#ifdef MD_LAYOUTTEXTS
		QTextBrowser *text;
#endif
		size_t memBase;
};

#endif	// __M68KDASMWIN_H__
