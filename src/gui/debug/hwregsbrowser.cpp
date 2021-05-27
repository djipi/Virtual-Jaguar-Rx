//
// hwregsbrowser.h: Hardware registers browser
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  08/20/2019  Created this file
//

// STILL TO DO:
//

#include "hwregsbrowser.h"


// 
HWRegsBrowserWindow::HWRegsBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
//statusbar(new QStatusBar),
hwregstabWidget(new QTabWidget),
hwregsblitterWin(new HWRegsBlitterBrowserWindow)
{
	setWindowTitle(tr("Hardware Registers Browser"));

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

	//
	hwregstabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	hwregstabWidget->addTab(hwregsblitterWin, tr("Blitter"));
	layout->addWidget(hwregstabWidget);

	// Status bar
	//layout->addWidget(statusbar);
	setLayout(layout);
}


//
HWRegsBrowserWindow::~HWRegsBrowserWindow(void)
{
	Reset();
}


//
void HWRegsBrowserWindow::Reset(void)
{
}


//
void HWRegsBrowserWindow::RefreshContents(void)
{
	hwregsblitterWin->RefreshContents();
}


// 
void HWRegsBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
