//
// interuptbrowser.cpp: Interrupt browser
//
// Part of the Virtual Jaguar Rx Project
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM   July/2022  Created this file
// JPM   Jan./2024  Added ASI & I2S from JINTCTRL
//

// STILL TO DO:
// 

#include "interuptbrowser.h"
#include "jerry.h"


// Jerry's JINTCTRL interupt names
const char* JINTCTRLdesc[6] = { "External", "DSP", "Timer One (sample rate)", "Timer Two (tempo)", "Asynchronous Serial Interface", "Synchronous Serial Interface" };


// 
InteruptBrowserWindow::InteruptBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog)
{
	// set window title
	setWindowTitle(tr("Interrupt Browser"));

	// create layout
	QVBoxLayout *layout4 = new QVBoxLayout;

	// create layout for JERRY's layout
	QGroupBox *boxJERRY = new QGroupBox("JERRY");
	QHBoxLayout *layoutJERRY = new QHBoxLayout(boxJERRY);
	QGroupBox *boxSources = new QGroupBox("Sources");
	layoutJERRY->addWidget(boxSources);
	QVBoxLayout *layoutSources = new QVBoxLayout(boxSources);
	QGroupBox *boxPending = new QGroupBox("Pending");
	layoutJERRY->addWidget(boxPending);
	QVBoxLayout *layoutPending = new QVBoxLayout(boxPending);
	for (size_t i = 0; i < 2; i++)
	{
		QVBoxLayout *layoutdesc = !i ? layoutSources : layoutPending;
		for (size_t j = 0; j < 6; j++)
		{
			jerry[i][j] = /*i ? new QCheckBox(tr("")) : */ new QCheckBox(tr(JINTCTRLdesc[j]));
			layoutdesc->addWidget(jerry[i][j]);
			jerry[i][j]->setEnabled(false);
		}
	}

	// set layout
	layout4->addWidget(boxJERRY, 0, 0);
	setLayout(layout4);
}


//
InteruptBrowserWindow::~InteruptBrowserWindow()
{
}


//
void InteruptBrowserWindow::RefreshContents(void)
{
	if (isVisible())
	{
		// refresh JERRY information
		for (size_t i = 0; i < 2; i++)
		{
			size_t v = !i ? jerryInterruptMask : jerryPendingInterrupt;
			for (size_t j = 0, b = 1; j < 6; j++, b <<= 1)
			{
				jerry[i][j]->setChecked(v & b);
			}
		}
	}
}
