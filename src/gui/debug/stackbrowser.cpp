//
// stackbrowser.cpp - Jaguar stack  browser
//
// by James Hammons
// (C) 2012 Underground Software
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  01/11/2017  Created this file
//

// STILL TO DO:
//

#include "stackbrowser.h"
#include "memory.h"
#include "m68000/m68kinterface.h"
#include "settings.h"


//#define DEBUG_SPDISPLAY 26			// To fill up to 256 bytes with values from 0 to $FF below the SP pointer


//
StackBrowserWindow::StackBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout),
	text(new QLabel),
	//refresh(new QPushButton(tr("Refresh"))),
	//address(new QLineEdit),
	//go(new QPushButton(tr("Go"))),
	memBase(m68k_get_reg(NULL, M68K_REG_SP))
{
	setWindowTitle(tr("Stack Browser"));

/*
	address->setInputMask("hhhhhh");
	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(refresh);
	hbox1->addWidget(address);
	hbox1->addWidget(go);
*/

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
/*
	layout->addLayout(hbox1);
*/

/*
	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
*/
}


// 
void StackBrowserWindow::RefreshContents(void)
{
	char string[1024], buf[64];
	QString memDump;
	size_t i, j;
	uint8_t c;

	if (isVisible())
	{
		memBase = m68k_get_reg(NULL, M68K_REG_SP);

#ifdef DEBUG_SPDISPLAY
#if DEBUG_SPDISPLAY < 257
		memBase -= DEBUG_SPDISPLAY;
		for (i = 0; i < DEBUG_SPDISPLAY; i++)
		{
			jaguarMainRAM[memBase + i] = (uint8_t)i;
		}
#endif
#endif

		for (i = 0; i < 480; i += 16)
		{
			if ((memBase + i) < vjs.DRAM_size)
			{
				sprintf(string, "%s%06X: ", (i ? "<br>" : ""), (unsigned int)(memBase + i));

				for (j = 0; j < 16; j++)
				{
					if ((memBase + i + j) < vjs.DRAM_size)
					{
						sprintf(buf, "%02X ", jaguarMainRAM[memBase + i + j]);
					}
					else
					{
						if (i)
						{
							sprintf(buf, "&nbsp;&nbsp;&nbsp;");
						}
						else
						{
							sprintf(buf, "   ");
						}
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to dig the reason(s) why the 2nd line needs to use the &nbsp; instead of space !!!")
#else
#warning "!!! Need to dig the reason(s) why the 2nd line needs to use the &nbsp; instead of space !!!"
#endif // _MSC_VER
					}
					strcat(string, buf);
				}

				//sprintf(buf, "| ");
				//strcat(string, buf);
				strcat(string, "| ");

				for (j = 0; j < 16; j++)
				{
					if ((memBase + i + j) < vjs.DRAM_size)
					{
						c = jaguarMainRAM[memBase + i + j];
						//sprintf(buf, "&#%i;", c);

						//if (c == 0x20)
						//{
						//	sprintf(buf, "&nbsp;");
						//}
						//else
						{
							//if (c < 0x20)
							if ((c < 0x20) || (c > 0x7E))
							{
								sprintf(buf, ".");
								//buf[0] = '.';
							}
							else
							{
								sprintf(buf, "&#%i;", c);
								//buf[0] = c;
							}
							//buf[1] = 0;
						}

						strcat(string, buf);
					}
				}

				memDump += QString(string);
			}
		}

		text->clear();
		text->setText(memDump);
	}
}


/*
void StackBrowserWindow::keyPressEvent(QKeyEvent * e)
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

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

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

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

		RefreshContents();
	}
}
*/


/*
void StackBrowserWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
*/
