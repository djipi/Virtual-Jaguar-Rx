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
// JPM   July/2022  Added Jerry registers
//

// STILL TO DO:
//

#include "hwregsbrowser.h"


// 
HWRegsBrowserWindow::HWRegsBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
//statusbar(new QStatusBar),
hwregstabWidget(new QTabWidget),
hwregsblitterWin(new HWRegsBlitterBrowserWindow),
hwregsjerryWin(new HWRegsJerryBrowserWindow)
{
	setWindowTitle(tr("Hardware Registers Browser"));

	// set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

	// set the Blitter tab
	hwregstabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	hwregstabWidget->addTab(hwregsblitterWin, tr("Blitter"));
	// set the Jerry tab
	hwregstabWidget->addTab(hwregsjerryWin, tr("Jerry"));

	// set layout
	layout->addWidget(hwregstabWidget);
	// status bar
	setLayout(layout);

	// connect the signals
	connect(hwregstabWidget, SIGNAL(currentChanged(const int)), this, SLOT(RefreshContents()));
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
	hwregsjerryWin->RefreshContents();
}


// 
void HWRegsBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
