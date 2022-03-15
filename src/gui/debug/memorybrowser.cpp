//
// memorybrowser.cpp - Jaguar memory browser
//
// by James Hammons
// (C) 2012 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  08/14/2012  Created this file
// JPM  March/2022  Modified to support the GPU & DSP memory browser window
//

// STILL TO DO:
//

#include "memorybrowser.h"
//#include "memory.h"


MemoryBrowserWindow::MemoryBrowserWindow(QWidget * parent/*= 0*/, int Type): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh"))),
	go(new QPushButton(tr("Go"))),
	address(new QLineEdit),
	memmin(MemTypeInfo[Type].memmin),
	memmax(MemTypeInfo[Type].memmax),
	memzone(MemTypeInfo[Type].memzone),
	memBase(memmin)
{
	// mem information setup
	setWindowTitle(tr(MemTypeInfo[Type].WindowTitle));

	// entry mask for the address (in hexadecimal)
	address->setInputMask("hhhhhh");

	// layout creation
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

	// set layout
	setLayout(layout);
	layout->addWidget(text);
//	layout->addWidget(refresh);
	layout->addLayout(hbox1);

	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
	connect(address, SIGNAL(returnPressed()), this, SLOT(GoToAddress()));
}


//
MemoryBrowserWindow::~MemoryBrowserWindow(void)
{
}


// Display a window of 480 bytes 
void MemoryBrowserWindow::RefreshContents(void)
{
	char string[1024], buf[64];
	QString memDump;

	// window needs to be visible
	if (isVisible())
	{
		// loop on the 480 bytes
		for (uint32_t i = 0; i < 480; i += 16)
		{
			// first step to set the address (hexadecinal) in the text line
			sprintf(string, "%s%06X: ", (i != 0 ? "<br>" : ""), memBase + i);

			// second step to append 16 bytes (hexdecimal) vale in the text line
			for (uint32_t j = 0; j < 16; j++)
			{
				sprintf(buf, "%02X ", memzone[memBase - memmin + i + j]);
				strcat(string, buf);
			}

			// third step to append a separator
			//sprintf(buf, "| ");
			strcat(string, "| ");

			// fourth step to append each byte value as a character in the text line
			for (uint32_t j = 0; j < 16; j++)
			{
				// get the char and check alphanumeric vs 'special' character
				uint8_t c = memzone[memBase - memmin + i + j];
				
				if (c == 0x20)
				{
					sprintf(buf, "&nbsp;");
				}
				else
				{
					if ((c < 0x20) || (c > 0x7E))
					{
						sprintf(buf, ".");
					}
					else
					{
						sprintf(buf, "&#%i;", c);
					}
				}

				strcat(string, buf);
			}

			// fifth step to add the text line in the lines buffer
			memDump += QString(string);
		}

		// display the lines
		text->clear();
		text->setText(memDump);
	}
}


#if 0
//
void MemoryBrowserWindow::wheelEvent(QMouseEvent *e)
{
}
#endif


// 
void MemoryBrowserWindow::CheckMemZone(void)
{
	if (memBase < memmax)
	{
		if (memBase < (memmax - 480))
		{
			return;
		}
	}

	memBase = memmax - 480;
}


// 
void MemoryBrowserWindow::keyPressEvent(QKeyEvent * e)
{
#if 0
	if (e->key() == Qt::Key_Enter)
	{
		GoToAddress();
	}
	else
#endif
	{
		if (e->key() == Qt::Key_Escape)
		{
			hide();
		}
		else
		{
			if (e->key() == Qt::Key_PageUp)
			{
				memBase -= 480;

				if (memBase < memmin)
				{
					memBase = memmin;
				}

				RefreshContents();
			}
			else
			{
				if (e->key() == Qt::Key_PageDown)
				{
					memBase += 480;
					CheckMemZone();
					RefreshContents();
				}
				else
				{
					if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
					{
						memBase -= 16;

						if (memBase < memmin)
						{
							memBase = memmin;
						}

						RefreshContents();
					}
					else
					{
						if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
						{
							memBase += 16;
							CheckMemZone();
							RefreshContents();
						}
						else
						{
							if (e->key() == Qt::Key_Home)
							{
								memBase = memmin;
								RefreshContents();
							}
							else
							{
								if (e->key() == Qt::Key_End)
								{
									memBase = memmax - 480;
									RefreshContents();
								}
							}
						}
					}
				}
			}
		}
	}
}


// 
void MemoryBrowserWindow::GoToAddress(void)
{
	bool ok;

	// get the new address
	int32_t newmemBase = address->text().toUInt(&ok, 16);
	
	// check address validity
	if (ok && (newmemBase >= memmin))
	{
		// check the address fitting in the memory zone
		memBase = newmemBase;
		CheckMemZone();
		// refresh content
		RefreshContents();
	}
}

