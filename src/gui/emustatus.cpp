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
EmuStatusWindow::EmuStatusWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout),
	text(new QLabel),
	GPURunning(GPUIsRunning())
{
	setWindowTitle(tr("Emulator status"));

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	setLayout(layout);

	layout->addWidget(text);
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
		sprintf(string, "                DRAM | %i KB", (vjs.DRAM_size / 1024));
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
