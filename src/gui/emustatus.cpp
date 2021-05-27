//
// emustatus.cpp - Jaguar emulator status
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  02/02/2017  Created this file
// JPM   Apr./2021  Display number of M68K cycles used in tracing mode
//

// STILL TO DO:
//

#include "emustatus.h"
#include "memory.h"
#include "gpu.h"
#include "m68000/m68kinterface.h"
#include "jaguar.h"
#include "settings.h"


// 
EmuStatusWindow::EmuStatusWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
resetcycles(new QPushButton(tr("Reset cycles"))),
text(new QLabel),
M68K_totalcycles(0),
M68K_opcodecycles(0),
GPURunning(GPUIsRunning())
{
	setWindowTitle(tr("Emulator status"));

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	setLayout(layout);

	layout->addWidget(text);
	layout->addWidget(resetcycles);

	connect(resetcycles, SIGNAL(clicked()), this, SLOT(ResetCycles()));
}


//
void EmuStatusWindow::ResetCycles(void)
{
	ResetM68KCycles();
	RefreshContents();
}


//
void EmuStatusWindow::ResetM68KCycles(void)
{
	M68K_totalcycles = M68K_opcodecycles = 0;
}


//
void EmuStatusWindow::UpdateM68KCycles(size_t cycles)
{
	M68K_totalcycles += (M68K_opcodecycles = cycles);
}


// 
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
		sprintf(string, "        M68K tracing | %s\n", (startM68KTracing ? "On" : "Off"));
		emuStatusDump += QString(string);
		sprintf(string, "                DRAM | %zi KB\n", (vjs.DRAM_size / 1024));
		emuStatusDump += QString(string);
		sprintf(string, "        M68K tracing | %zi cycle%s\n", M68K_opcodecycles, (M68K_opcodecycles ? "s" : ""));
		emuStatusDump += QString(string);
		sprintf(string, "  M68K tracing total | %zi cycle%s", M68K_totalcycles, (M68K_totalcycles ? "s" : ""));
		emuStatusDump += QString(string);

		text->setText(emuStatusDump);
	}
}


// 
void EmuStatusWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
