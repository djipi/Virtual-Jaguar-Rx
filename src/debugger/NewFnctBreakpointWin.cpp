//
// NewFnctBreakpointsWin.cpp - New function breakpoint
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  10/19/2018  Created this file
// JPM  March/2021  Breakpoint list window refresh
//

// STILL TO DO:
// Set information (name, etc.) for the asm function
// Find a way to refresh the breakpoints list window
//

#include "debugger/NewFnctBreakpointWin.h"
#include "jaguar.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"
#include "settings.h"


//
NewFnctBreakpointWindow::NewFnctBreakpointWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
address(new QLineEdit),
add(new QPushButton(tr("Add")))
{
	setWindowTitle(tr("New function breakpoint"));

	address->setPlaceholderText("0x<value>, decimal value or symbol name");

	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(address);
	hbox1->addWidget(add);

	layout->addLayout(hbox1);
	setLayout(layout);

	connect(add, SIGNAL(clicked()), this, SLOT(AddBreakpointAddress()));
	connect(address, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(SelectBreakpointAddress()));
}


//
void NewFnctBreakpointWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		if (e->key() == Qt::Key_Return)
		{
			AddBreakpointAddress();
		}
	}
}


//
void NewFnctBreakpointWindow::SetFnctBreakpointWin(BreakpointsWindow* BpW)
{
	BPWin = BpW;
}


void NewFnctBreakpointWindow::SelectBreakpointAddress(void)
{
	address->setStyleSheet("color: black");
}


// Add a breakpoint to the address
// Address can be an hexa, decimal or a symbol name
void NewFnctBreakpointWindow::AddBreakpointAddress(void)
{
	bool ok;
	size_t len;
	QString newAddress;
	size_t adr;
	S_BrkInfo Brk;

	memset(&Brk, 0, sizeof(Brk));
	newAddress = address->text();

	if ((len = newAddress.size()))
	{
		if ((len > 1) && (newAddress.at(0) == QChar('0')) && (newAddress.at(1) == QChar('x')))
		{
			adr = newAddress.toUInt(&ok, 16);
		}
		else
		{
			if (!(adr = DBGManager_GetAdrFromSymbolName(newAddress.toLatin1().data())))
			{
				adr = newAddress.toUInt(&ok, 10);
			}
			else
			{
				ok = true;
			}
		}

		// Check validity address
		if (ok && (adr < 0xffffff))
		{
			// Set information based on address
			Brk.Name = DBGManager_GetSymbolNameFromAdr(adr);
			Brk.Filename = DBGManager_GetFullSourceFilenameFromAdr(adr, NULL);
			Brk.NumLine = DBGManager_GetNumLineFromAdr(adr, DBG_TAG_subprogram);
			Brk.LineSrc = DBGManager_GetLineSrcFromAdrNumLine(adr, Brk.NumLine);

			// In all cases, consider address as valid
			Brk.Adr = adr;

			// Add the breakpoint
			if (!m68k_brk_add(&Brk))
			{
				address->setStyleSheet("color: green");
			}
			else
			{
				address->setText("");
			}
		}
		else
		{
			// Address is not valid
			address->setStyleSheet("color: red");
		}

		// update the breakpoint functions window
		BPWin->RefreshContents();
	}
}


//
NewFnctBreakpointWindow::~NewFnctBreakpointWindow()
{
}
