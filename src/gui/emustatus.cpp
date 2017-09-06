//
// emustatus.cpp - Jaguar emulator status
//
// by Jean-Paul Mari
// (C) 2012 Underground Software
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  02/02/2017  Created this file
//

// STILL TO DO:
//

#include "emustatus.h"
#include "memory.h"
#include "gpu.h"
#include "m68000/m68kinterface.h"
#include "jaguar.h"


EmuStatusWindow::EmuStatusWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout),
	text(new QLabel),
	//refresh(new QPushButton(tr("Refresh"))),
	//address(new QLineEdit),
	//go(new QPushButton(tr("Go"))),
	GPURunning(GPUIsRunning())
{
	setWindowTitle(tr("Emulator status"));

/*
	address->setInputMask("hhhhhh");
	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(refresh);
	hbox1->addWidget(address);
	hbox1->addWidget(go);
*/

	// Need to set the size as well...
//	resize(560, 480);

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
//	QFont fixedFont("", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
////	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	layout->addWidget(text);
//	layout->addWidget(refresh);
/*
	layout->addLayout(hbox1);
*/

/*
	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
*/
}


void EmuStatusWindow::RefreshContents(void)
{
	char string[1024];
	QString emuStatusDump;

	if (isVisible())
	{
		text->clear();

		GPURunning = GPUIsRunning();
		sprintf(string, "          GPU active | %s\n", (GPURunning ? "Yes" : "No"));
		emuStatusDump += QString(string);
		M68000DebugHaltStatus = M68KDebugHaltStatus();
		sprintf(string, "M68K debugger status | %s\n", (M68000DebugHaltStatus ? "Halt" : "Run"));
		emuStatusDump += QString(string);
		sprintf(string, "        M68K tracing | %s", (startM68KTracing ? "On" : "Off"));
		emuStatusDump += QString(string);

		text->setText(emuStatusDump);
	}
}


/*
void EmuStatusWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
		hide();
	else if (e->key() == Qt::Key_PageUp)
	{
		memBase -= 480;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_PageDown)
	{
		memBase += 480;

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
	{
		memBase -= 16;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
	{
		memBase += 16;

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

		RefreshContents();
	}
}
*/


/*
void EmuStatusWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
*/
