//
// riscdasmbrowser.cpp - Jaguar RISC disassembly browser
//
// by James Hammons
// (C) 2013 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/22/2012  Created this file
//

// STILL TO DO:
//

#include "riscdasmbrowser.h"
//#include "memory.h"
#include "dsp.h"
#include "gpu.h"
#include "jagdasm.h"


RISCDasmBrowserWindow::RISCDasmBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh"))),
	go(new QPushButton(tr("Go"))),
	address(new QLineEdit),
	gpu(new QRadioButton(tr("GPU"))),
	dsp(new QRadioButton(tr("DSP"))),
	memBase(0xf03000)
{
	setWindowTitle(tr("RISC Disassembly Browser"));

	address->setInputMask("hhhhhh");
	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(refresh);
	hbox1->addWidget(address);
	hbox1->addWidget(go);

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
	layout->addLayout(hbox1);

	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
}


void RISCDasmBrowserWindow::RefreshContents(void)
{
	char string[1024];//, buf[64];
	QString s;

	char buffer[2048];
	int pc = memBase, oldpc;

	if (isVisible())
	{
		for (uint32_t i = 0; i < 32; i++)
		{
			oldpc = pc;
			pc += dasmjag(JAGUAR_GPU, buffer, pc);
			sprintf(string, "%06X: %s<br>", oldpc, buffer);

			buffer[0] = 0;	// Clear string
			char singleCharString[2] = { 0, 0 };

			for (uint j = 0; j < strlen(string); j++)
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

		text->clear();
		text->setText(s);
	}
}


void RISCDasmBrowserWindow::keyPressEvent(QKeyEvent * e)
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


void RISCDasmBrowserWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}

