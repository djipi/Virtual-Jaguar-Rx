//
// DasmWin.cpp - Jaguar disassembly window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  06/26/2016  Created this file
//

// STILL TO DO:
//

#include "debugger/DasmWin.h"
#include "debugger/m68kDasmWin.h"
//#include "memory.h"
#include "m68000/m68kinterface.h"
#include "dsp.h"
#include "gpu.h"


//DasmWindow::DasmWindow(QMdiArea * parent/*= 0*/) : QMdiSubWindow(parent, Qt::Dialog),
DasmWindow::DasmWindow(QWidget * parent /*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout1(new QDockWidget), layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh"))),
	address(new QLineEdit),
	go(new QPushButton(tr("Go"))),
	memBase(0x4000)
{
//	m68kDasmWindow *m68kDasmWin = new m68kDasmWindow();
	setWindowTitle(tr("Jaguar Disassembly Window"));
	layout1->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//addDockWidget(Qt::RightDockWidgetArea, layout1);
	//layout1->addTab(new m68KDasmWindow(), tr("M68K Disassembly"));
	//layout = new QTabWidget;
	//layout->QWidget();

	//QDockWidget *shapesDockWidget = new QDockWidget(tr("Shapes"));
	//shapesDockWidget->setObjectName("shapesDockWidget");
	//shapesDockWidget->setWidget(treeWidget);
	//shapesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
	//shapesDockWidget->show();

#if 0
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
#endif
}


void DasmWindow::RefreshContents(void)
{
	char string[1024];//, buf[64];
	QString s;

	char buffer[2048];
	int pc = memBase, oldpc;

	for(uint32_t i=0; i<32; i++)
	{
		oldpc = pc;
		pc += m68k_disassemble(buffer, pc, 0, 1);
//		WriteLog("%06X: %s\n", oldpc, buffer);
		sprintf(string, "%06X: %s<br>", oldpc, buffer);

		buffer[0] = 0;	// Clear string
		char singleCharString[2] = { 0, 0 };

		for(int j=0; j<strlen(string); j++)
		{
			if (string[j] == 32)
				strcat(buffer, "&nbsp;");
			else
			{
				singleCharString[0] = string[j];
				strcat(buffer, singleCharString);
			}
		}

//		s += QString(string);
		s += QString(buffer);
	}

	text->clear();
	text->setText(s);
}


void DasmWindow::keyPressEvent(QKeyEvent * e)
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

		if (memBase > (0xF00000 - 64))
			memBase = 0xF00000 - 64;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
	{
		memBase -= 16;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
	{
		memBase += 16;

		if (memBase > (0xF00000 - 64))
			memBase = 0xF00000 - 64;

		RefreshContents();
	}
#endif
}


void DasmWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}

