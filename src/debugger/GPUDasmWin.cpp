//
// GPUdasmWin.cpp - Jaguar GPU disassembly window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  02/01/2017  Created this file
//

// STILL TO DO:
//

#include "GPUDasmWin.h"
#include "dsp.h"
#include "gpu.h"
#include "jagdasm.h"
#include "settings.h"


GPUDasmWindow::GPUDasmWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QLabel),
	memBase(GPUReadLong(0xF02110, DEBUG))
{
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	setLayout(layout);

	layout->addWidget(text);
}


void GPUDasmWindow::RefreshContents(void)
{
	char string[1024];
	QString s;
	char buffer[2048];
	int pc = memBase, oldpc;
	uint32_t GPUPC = GPUReadLong(0xF02110, DEBUG);
	bool GPUPCShow = false;

	text->clear();

	for(uint32_t i=0; i<vjs.nbrdisasmlines; i++)
	{
		oldpc = pc;
		pc += dasmjag(JAGUAR_GPU, buffer, pc);

		if (GPUPC == oldpc)
		{
			sprintf(string, "=> %06X: %s<br>", oldpc, buffer);
			GPUPCShow = true;
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

	if (GPUPCShow)
	{
		text->setText(s);
	}
	else
	{
		UseGPUPCAddress();
		RefreshContents();
	}
}


// Set mem base PC address using the 68K pc current address
void	GPUDasmWindow::UseGPUPCAddress(void)
{
	memBase = GPUReadLong(0xF02110, DEBUG);
}

