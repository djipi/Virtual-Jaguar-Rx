//
// NewBreakpointsWin.cpp - New breakpoint
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  mm/dd/yyyy  What
// ---  ----------  -----------------------------------------------------------
// JPM  10/19/2018  Created this file
// JPM  March/2021  Breakpoint list window refresh
// JPM  March/2022  Added hexadecimal's value with $
// JPM   July/2026  Added data breakpoint
//

// STILL TO DO:
// Set information (name, etc.) for the asm function
// Find a way to refresh the breakpoints list window
//

#include "debugger/NewBreakpointWin.h"
#include "jaguar.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"
#include "settings.h"


//
NewBreakpointWindow::NewBreakpointWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
address(new QLineEdit),
add(new QPushButton(tr("Add"))),
type(new QComboBox),
access(new QComboBox),
accessLabel(new QLabel(tr("Access:")))
{
	// set the window title
	setWindowTitle(tr("Code & Data breakpoint"));

	// set the place holder text for the address input
	address->setPlaceholderText("$<value>, 0x<value>, decimal value or symbol name");

	// breakpoint type with Code by default
	type->addItem(tr("Code"), BP_CODE);
	type->addItem(tr("Data"), BP_DATA);
	type->setCurrentIndex(BP_CODE);

	// breakpoint access with Write by default
	access->addItem(tr("Read"), BP_READ);
	access->addItem(tr("Write"), BP_WRITE);
	access->addItem(tr("Read/Write"), BP_READWRITE);
	access->setCurrentIndex(access->findData(BP_WRITE));
	// hidden while "Code" is selected
	accessLabel->hide();
	access->hide();

	// set the row for the address
	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(address);
	// set the row for the breakpoint type, access and the add button
	QHBoxLayout* hbox2 = new QHBoxLayout;
	hbox2->addWidget(new QLabel(tr("Type:")));
	hbox2->addWidget(type);
	hbox2->addSpacing(15);
	hbox2->addWidget(accessLabel);
	hbox2->addWidget(access);
	hbox2->addStretch();
	hbox2->addWidget(add);
	// add the rows to the main layout
	layout->addLayout(hbox1);
	layout->addLayout(hbox2);
	setLayout(layout);

	// set the connection for the add button, type selection and the address input
	connect(add, SIGNAL(clicked()), this, SLOT(AddBreakpointAddress()));
	connect(address, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(SelectBreakpointAddress()));
	connect(type, SIGNAL(currentIndexChanged(int)),	this, SLOT(TypeChanged(int)));
}


// Handle the key press event
void NewBreakpointWindow::keyPressEvent(QKeyEvent * e)
{
	// hide the window if the escape key is pressed
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		// add a breakpoint if the return key is pressed
		if (e->key() == Qt::Key_Return)
		{
			AddBreakpointAddress();
		}
	}
}


// Set the breakpoint functions window
void NewBreakpointWindow::SetBreakpointWin(BreakpointsWindow* BpW)
{
	BPWin = BpW;
}


// Set the address's text color to black
void NewBreakpointWindow::SelectBreakpointAddress(void)
{
	address->setStyleSheet("color: black");
}


// Handle the breakpoint type change event
void NewBreakpointWindow::TypeChanged(int index)
{
	// show the access combobox only if the breakpoint type is "Data"
	bool isData = (index == 1);
	accessLabel->setVisible(isData);
	access->setVisible(isData);
	// adjust the window size to fit the new layout
	adjustSize();      
}


// Add a breakpoint to the address
// Address can be an hexa, decimal or a symbol name
void NewBreakpointWindow::AddBreakpointAddress(void)
{
	bool ok;
	size_t len;
	QString newAddress;
	size_t adr;
	S_BrkInfo Brk;

	memset(&Brk, 0, sizeof(Brk));
	newAddress = address->text();

	// get the value's length
	if ((len = newAddress.size()))
	{
		// get the address from hexadecimal's value (0x)
		if ((len > 2) && (newAddress.at(0) == QChar('0')) && (newAddress.at(1) == QChar('x')))
		{
			adr = newAddress.mid(2).toUInt(&ok, 16);
		}
		else
		{
			// get the address from the symbol's name
			if (!(adr = DBGManager_GetAdrFromSymbolName(newAddress.toLatin1().data())))
			{
				// get the address from hexadecimal's value ($)
				if ((len > 1) && (newAddress.at(0) == QChar('$')))
				{
					adr = newAddress.mid(1).toUInt(&ok, 16);
				}
				else
				{
					// get the address from the decimal's value
					adr = newAddress.toUInt(&ok, 10);
				}
			}
			else
			{
				ok = true;
			}
		}

		// check validity address
		if (ok && (adr < 0xffffff))
		{
			// set information based on address
			Brk.Name = DBGManager_GetSymbolNameFromAdr(adr);
			Brk.Filename = DBGManager_GetFullSourceFilenameFromAdr(adr, NULL);
			Brk.NumLine = DBGManager_GetNumLineFromAdr(adr, DBG_TAG_subprogram);
			Brk.LineSrc = DBGManager_GetLineSrcFromAdrNumLine(adr, Brk.NumLine);
			// set the breakpoint type based on the combobox selection
			Brk.IsCode = (type->currentIndex() == 0);
			// set the breakpoint Read/Write flag based on the combobox selection (code is always read)
			Brk.Access = Brk.IsCode ? BP_READ : access->currentData().toInt();
			// in all cases, consider address as valid
			Brk.Adr = adr;

			// add the breakpoint
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
			// address is not valid
			address->setStyleSheet("color: red");
		}

		// update the breakpoint functions window
		BPWin->RefreshContents();
	}
}


//
NewBreakpointWindow::~NewBreakpointWindow()
{
}
