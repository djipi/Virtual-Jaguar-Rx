//
// ctrlprofilerwin.cpp - Profiler control window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM   Nov./2025       Created this file
//

#include "profiler/ctrlprofilerwin.h"
#include "profiler.h"
#include "debugger/DBGManager.h"


// Constructor
CtrlProfilerWindow::CtrlProfilerWindow(QWidget* parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
loopframebox(new QGroupBox(tr("Loop frame"))),
loopframeaddress(new QLineEdit),
loopframeadd(new QPushButton(tr("Add")))
{
	setWindowTitle(tr("Profiler control"));

	// Loop frame group box
	loopframeaddress->setPlaceholderText("$<value>, 0x<value>, decimal value or symbol name");
	loopframebox->setStyleSheet("QGroupBox { border: 1px solid gray; border-radius: 3px; margin-top: 0.5em; } QGroupBox::title { subcontrol-origin: margin; left: 10px; padding: 0 3px 0 3px; }");
	QHBoxLayout* hbox1 = new QHBoxLayout;
	hbox1->addWidget(loopframeaddress);
	hbox1->addWidget(loopframeadd);
	loopframebox->setLayout(hbox1);

	// create the main layout
	layout->addWidget(loopframebox);
	setLayout(layout);

	// Signals/slots connections
	connect(loopframeadd, SIGNAL(clicked()), this, SLOT(AddLoopFrameAddress()));
	connect(loopframeaddress, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(SelectLoopFrameAddress()));
}


// Handle key press event
void CtrlProfilerWindow::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		if (e->key() == Qt::Key_Return)
		{
			AddLoopFrameAddress();
		}
	}
}


// Select the loop frame address
void CtrlProfilerWindow::SelectLoopFrameAddress(void)
{
	loopframeaddress->setStyleSheet("color: black");
}


// Add a loop frame to the address
// Address can be an hexa, decimal or a symbol name
void CtrlProfilerWindow::AddLoopFrameAddress(void)
{
	bool ok;
	size_t len;
	QString newAddress;
	size_t adr;
	S_FrameLoopInfo frmloop;

	//memset(&Brk, 0, sizeof(Brk));
	newAddress = loopframeaddress->text();

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

		// Check validity address
		if (ok && (adr < 0xffffff))
		{
			// Set information based on address
			frmloop.Name = DBGManager_GetSymbolNameFromAdr(adr);
			frmloop.Filename = DBGManager_GetFullSourceFilenameFromAdr(adr, NULL);
			frmloop.NumLine = DBGManager_GetNumLineFromAdr(adr, DBG_TAG_subprogram);
			frmloop.LineSrc = DBGManager_GetLineSrcFromAdrNumLine(adr, frmloop.NumLine);

			// In all cases, consider address as valid
			frmloop.Adr = adr;

			// Add the frame loop
			if (m68kProfilerEntryLoopFrame(&frmloop))
			{
				loopframeaddress->setStyleSheet("color: green");
			}
			else
			{
				loopframeaddress->setText("");
			}
		}
		else
		{
			// Address is not valid
			loopframeaddress->setStyleSheet("color: red");
		}
	}
}


// Destructor
CtrlProfilerWindow::~CtrlProfilerWindow()
{
}
