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


//#define DEBUG_SPDISPLAY 1000		// To fill up to 256 bytes with values from 0 to $FF below the SP pointer any above are random values


//
StackBrowserWindow::StackBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout),
	text(new QLabel),
	//refresh(new QPushButton(tr("Refresh"))),
	//address(new QLineEdit),
	//go(new QPushButton(tr("Go"))),
	stackBase(m68k_get_reg(NULL, M68K_REG_SP))
{
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
	char string[1024];

	if (isVisible())
	{
#ifdef DEBUG_SPDISPLAY
		m68k_set_reg(M68K_REG_SP, (vjs.DRAM_size - DEBUG_SPDISPLAY));
#endif
		if ((stackBase = m68k_get_reg(NULL, M68K_REG_SP)) && (stackBase < vjs.DRAM_size))
		{

#ifdef DEBUG_SPDISPLAY
			for (int i = 0; i < DEBUG_SPDISPLAY; i++)
			{
#if DEBUG_SPDISPLAY < 257
				jaguarMainRAM[stackBase + i] = (uint8_t)i;
#else
				jaguarMainRAM[stackBase + i] = (uint8_t)rand();
#endif
			}
#endif
			sprintf(string, "Stack Browser - 0x%06X", (unsigned int)(stackBase, (unsigned int)stackBase));
		}
		else
		{
			sprintf(string, "Stack Browser");
		}

		setWindowTitle(tr(string));
		RefreshContentsWindow();
	}
}


// Refresh / Display the window contents
void StackBrowserWindow::RefreshContentsWindow(void)
{
	char string[2048], buf[64];
	QString memDump;
	size_t i, j;
	uint8_t c;

	if (stackBase < vjs.DRAM_size)
	{
		for (i = 0; i < 480; i += 16)
		{
			if ((stackBase + i) < vjs.DRAM_size)
			{
				sprintf(string, "%s%06X: ", (i ? "<br>" : ""), (unsigned int)(stackBase + i));

				for (j = 0; j < 16; j++)
				{
					if ((stackBase + i + j) < vjs.DRAM_size)
					{
						sprintf(buf, "%02X ", jaguarMainRAM[stackBase + i + j]);
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
					if ((stackBase + i + j) < vjs.DRAM_size)
					{
						c = jaguarMainRAM[stackBase + i + j];
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
	}
	else
	{
		memDump += QString("");
	}

	text->clear();
	text->setText(memDump);
}


// 
void StackBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	size_t offset;

	// Escape key to hide the window
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		if (stackBase < vjs.DRAM_size)
		{
			if (e->key() == Qt::Key_PageUp)
			{
				offset = -480;
			}
			else
			{
				if (e->key() == Qt::Key_PageDown)
				{
					offset = 480;
				}
				else
				{
					if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
					{
						offset = -16;
					}
					else
					{
						if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Plus)
						{
							offset = 16;
						}
						else
						{
							offset = 0;
						}
					}
				}
			}

			if (offset)
			{
				if (offset < 0)
				{
					if ((stackBase += offset) < m68k_get_reg(NULL, M68K_REG_SP))
					{
						stackBase = m68k_get_reg(NULL, M68K_REG_SP);
					}
				}
				else
				{
					if ((stackBase += offset) > (vjs.DRAM_size - 480))
					{
						stackBase = vjs.DRAM_size - 480;
					}

					if (stackBase < m68k_get_reg(NULL, M68K_REG_SP))
					{
						stackBase = m68k_get_reg(NULL, M68K_REG_SP);
					}
				}

				RefreshContentsWindow();
			}
		}
	}
}


/*
void StackBrowserWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	stackBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
*/
