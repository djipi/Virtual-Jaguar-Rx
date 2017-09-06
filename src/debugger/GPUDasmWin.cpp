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
//#include "memory.h"
#include "dsp.h"
#include "gpu.h"
#include "jagdasm.h"
#include "settings.h"


GPUDasmWindow::GPUDasmWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout), text(new QLabel),
	//refresh(new QPushButton(tr("Refresh"))),
	//go(new QPushButton(tr("Go"))),
	//address(new QLineEdit),
	//gpu(new QRadioButton(tr("GPU"))),
	//dsp(new QRadioButton(tr("DSP"))),
	//memBase(0x4000)
	memBase(GPUReadLong(0xF02110, DEBUG))
{
	//setWindowTitle(tr("RISC Disassembly Browser"));

	//address->setInputMask("hhhhhh");
	//QHBoxLayout * hbox1 = new QHBoxLayout;
	//hbox1->addWidget(refresh);
	//hbox1->addWidget(address);
	//hbox1->addWidget(go);

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
	//layout->addLayout(hbox1);

	//connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
	//connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
}


void GPUDasmWindow::RefreshContents(void)
{
	char string[1024];//, buf[64];
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

//	text->clear();
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


#if 0
void GPUDasmWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Return)
		hide();
#if 1
	else if (e->key() == Qt::Key_PageUp)
	{
		memBase -= 64;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_PageDown)
	{
		memBase += 64;

		if (memBase > (0x1000000 - 480))
			memBase = 0x1000000 - 480;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
	{
		memBase -= 2;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
	{
		memBase += 2;

		if (memBase > (0x1000000 - 480))
			memBase = 0x1000000 - 480;

		RefreshContents();
	}
#endif
}
#endif


#if 0
void GPUDasmWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
#endif


// Set mem base PC address using the 68K pc current address
void	GPUDasmWindow::UseGPUPCAddress(void)
{
	memBase = GPUReadLong(0xF02110, DEBUG);
}


// Set mem base PC address
#if 0
void GPUDasmWindow::SetAddress(int address)
{
	memBase = address;
	//	RefreshContents();
}
#endif
