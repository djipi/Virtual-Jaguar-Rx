//
// memorybrowser.cpp - Jaguar memory browser
//
// by James Hammons
// (C) 2012 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JLH  08/14/2012  Created this file
//

// STILL TO DO:
//

#include "jagdasm.h"
#include "jaguar.h"

#include "memorybrowser.h"
#include "memory.h"


MemoryBrowserWindow::MemoryBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh"))),
	address(new QLineEdit),
	go(new QPushButton(tr("Go"))),
	memBase(0)
{
	setWindowTitle(tr("Memory Browser"));

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


void MemoryBrowserWindow::RefreshContents(void)
{
	char string[1024], buf[64];
	QString memDump;

	if (isVisible())
	{
		for (uint32_t i = 0; i < 480; i += 16)
		{
			sprintf(string, "%s%06X: ", (i != 0 ? "<br>" : ""), memBase + i);

			for (uint32_t j = 0; j < 16; j++)
			{
				if ((memBase + i + j) < 0x800000)
					sprintf(buf, "%02X ", jaguarMainRAM[memBase + i + j]);
				else if (((memBase + i + j) >= 0xF10000) && ((memBase + i + j) <= 0xF1FFFE))
					
					sprintf(buf, "%02X ", JaguarReadByte(memBase + i + j));
				
				strcat(string, buf);
			}

			sprintf(buf, "| ");
			strcat(string, buf);

			for (uint32_t j = 0; j < 16; j++)
			{
				uint8_t c;
				if ((memBase + i + j) < 0x800000)
					c = jaguarMainRAM[memBase + i + j];
				else if (((memBase + i + j) >= 0xF10000) && ((memBase + i + j) <= 0xF1FFFE))
					c = JaguarReadByte(memBase + i + j);
				sprintf(buf, "&#%i;", c);

				if (c == 0x20)
					sprintf(buf, "&nbsp;");

				if ((c < 0x20) || (c > 0x7E))
					sprintf(buf, ".");

				strcat(string, buf);
			}

			memDump += QString(string);
		}

		text->clear();
		text->setText(memDump);
	}
}


void MemoryBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
		hide();
	else if (e->key() == Qt::Key_PageUp)
	{
		memBase -= 480;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_PageDown)
	{
		memBase += 480;

		if (memBase > (0xF1FFFE - 480))
			memBase = 0xF1FFFE - 480;

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

		if (memBase > (0xF1FFFE - 480))
			memBase = 0xF1FFFE - 480;

		RefreshContents();
	}
}


void MemoryBrowserWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}

