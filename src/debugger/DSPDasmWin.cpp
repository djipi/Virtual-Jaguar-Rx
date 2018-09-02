//
// DSPDasmWin.cpp - Jaguar DSP disassembly window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  02/02/2017  Created this file
//

// STILL TO DO:
//

#include "DSPDasmWin.h"
#include "dsp.h"
#include "gpu.h"
#include "jagdasm.h"
#include "settings.h"


DSPDasmWindow::DSPDasmWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QLabel),
	memBase(DSPReadLong(0xF1A110, DEBUG))
{
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	setLayout(layout);

	layout->addWidget(text);
}


void DSPDasmWindow::RefreshContents(void)
{
	char string[1024];
	QString s;
	char buffer[2048];
	int pc = memBase, oldpc;
	uint32_t DSPPC = DSPReadLong(0xF1A110, DEBUG);
	bool DSPPCShow = false;

	text->clear();

	for(uint32_t i=0; i<vjs.nbrdisasmlines; i++)
	{
		oldpc = pc;
		pc += dasmjag(JAGUAR_DSP, buffer, pc);

		if (DSPPC == oldpc)
		{
			sprintf(string, "=> %06X: %s<br>", oldpc, buffer);
			DSPPCShow = true;
		}
		else
		{
			sprintf(string, "   %06X: %s<br>", oldpc, buffer);
		}

		buffer[0] = 0;	// Clear string
		char singleCharString[2] = { 0, 0 };

		for(uint j=0; j<strlen(string); j++)
		{
			if (string[j] == 32)
				strcat(buffer, "&nbsp;");
			else
			{
				singleCharString[0] = string[j];
				strcat(buffer, singleCharString);
			}
		}

		s += QString(buffer);
	}

	if (DSPPCShow)
	{
		text->setText(s);
	}
	else
	{
		UseDSPPCAddress();
		RefreshContents();
	}
}


// Set mem base PC address using the 68K pc current address
void	DSPDasmWindow::UseDSPPCAddress(void)
{
	memBase = DSPReadLong(0xF1A110, DEBUG);
}

