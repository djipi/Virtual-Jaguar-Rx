//
// BlitterRegsWin.cpp: Display the Blitter's regs values
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM   Jan./2022  Created this file

#include <stdlib.h>
#include "debugger/BlitterRegsWin.h"


// 
BlitterRegsWindow::BlitterRegsWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
mainLayout(new QVBoxLayout),
vaLayout(new QVBoxLayout),
a1Layout(new QHBoxLayout),
a2Layout(new QHBoxLayout),
A1Adr(new QLabel),
a1Frame(new QFrame),
statusbar(new QStatusBar)
{
	setWindowTitle(tr("Blitter's registers value"));
#ifdef BLITTEREGS_FONTS
	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
#endif
#ifdef BLITTEREGS_FONTS
	TableView->setFont(fixedFont);
#endif

	// set the A1's register
	a1Frame->setFrameStyle(QFrame::Panel | QFrame::Raised);
	a1Frame->setLineWidth(2);
	A1Adr->setText("toto");
	a1Layout->addWidget(a1Frame);
	a1Layout->addWidget(A1Adr);
	//a1Layout->setObjectName("tata");

	// create layout
	vaLayout->addLayout(a1Layout);
	vaLayout->addLayout(a2Layout);
	mainLayout->addLayout(vaLayout);
	// Status bar
	mainLayout->addWidget(statusbar);
	setLayout(mainLayout);
}


//
BlitterRegsWindow::~BlitterRegsWindow(void)
{
}


// Handle keyboard actions
void BlitterRegsWindow::keyPressEvent(QKeyEvent * e)
{
	// close / hide the window
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
