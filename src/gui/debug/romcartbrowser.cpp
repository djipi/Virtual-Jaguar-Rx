//
// romcartbrowser.cpp - Jaguar ROM cartridge browser
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  March/2022  Created this file
//

// STILL TO DO:
//

#include "romcartbrowser.h"
#include "memory.h"


ROMCartBrowserWindow::ROMCartBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout),
	text(new QLabel),
	reset(new QPushButton(tr("Reset"))),
	address(new QLineEdit),
	go(new QPushButton(tr("Go"))),
	romcartsize(ROMCART_6MB),
	listromcartsize(new QComboBox),
	memBase(0x800000)
{
	// display title
	setWindowTitle(tr("ROM Cartridge Browser"));

	// set the ROM cartridge size list (default 6MB)
	listromcartsize->addItem("1 MB", QVariant(ROMCART_1MB));
	listromcartsize->addItem("2 MB", QVariant(ROMCART_2MB));
	listromcartsize->addItem("4 MB", QVariant(ROMCART_4MB));
	listromcartsize->addItem("6 MB", QVariant(ROMCART_6MB));
	listromcartsize->setCurrentIndex(ROMCART_6MB);
	// entry mask for the address (in hexadecimal)
	address->setInputMask("hhhhhh");
	// allow keyboard action on the text display
	text->setFocusPolicy(Qt::WheelFocus);

	// layout creation
	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(listromcartsize);
	hbox1->addWidget(reset);
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
	//layout->addWidget(reset);
	layout->addLayout(hbox1);

	connect(reset, SIGNAL(clicked()), this, SLOT(ResetAddress()));
	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
	connect(address, SIGNAL(returnPressed()), this, SLOT(GoToAddress()));
	connect(listromcartsize, SIGNAL(currentIndexChanged(int)), this, SLOT(CurrentIndexRomCartSize(int)));
	//connect(text, SIGNAL(wheel(QWheelEvent*)), this, SLOT(wheel(QWheelEvent*)));
}


//
ROMCartBrowserWindow::~ROMCartBrowserWindow(void)
{
}


#if 0
//
void ROMCartBrowserWindow::wheel(QWheelEvent* e)
{

}
#endif


//
void ROMCartBrowserWindow::CurrentIndexRomCartSize(int index)
{
	romcartsize = index;
	CheckMaxRomCartSize();
	RefreshContents();
}


//
void ROMCartBrowserWindow::ResetAddress(void)
{
	memBase = 0x800000;
	romcartsize = ROMCART_6MB;
	RefreshContents();
}


// Display a window of 480 bytes 
void ROMCartBrowserWindow::RefreshContents(void)
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
				sprintf(buf, "%02X ", jaguarMainROM[(memBase - 0x800000) + i + j]);
				strcat(string, buf);
			}

			// third step to append a separator
			//sprintf(buf, "| ");
			strcat(string, "| ");

			// fourth step to append each byte value as a character in the text line
			for (uint32_t j = 0; j < 16; j++)
			{
				// get the char and check alphanumeric vs 'special' character
				uint8_t c = jaguarMainROM[(memBase - 0x800000) + i + j];
				
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
void ROMCartBrowserWindow::wheelEvent(QMouseEvent *e)
{
}
#endif


//
void ROMCartBrowserWindow::keyPressEvent(QKeyEvent * e)
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

				if (memBase < 0x800000)
				{
					memBase = 0x800000;
				}

				RefreshContents();
			}
			else
			{
				if (e->key() == Qt::Key_PageDown)
				{
					memBase += 480;
					CheckMaxRomCartSize();
					RefreshContents();
				}
				else
				{
					if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
					{
						memBase -= 16;

						if (memBase < 0x800000)
						{
							memBase = 0x800000;
						}

						RefreshContents();
					}
					else
					{
						if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
						{
							memBase += 16;
							CheckMaxRomCartSize();
							RefreshContents();
						}
						else
						{
							if (e->key() == Qt::Key_Home)
							{
								memBase = 0x800000;
								RefreshContents();
							}
							else
							{
								if (e->key() == Qt::Key_End)
								{
									memBase = (TabMaxSize[romcartsize] - 480);
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
void ROMCartBrowserWindow::CheckMaxRomCartSize(void)
{
	if (memBase < TabMaxSize[romcartsize])
	{
		if (memBase < (TabMaxSize[romcartsize] - 480))
		{
			return;
		}
	}

	memBase = (TabMaxSize[romcartsize] - 480);
}


// 
void ROMCartBrowserWindow::GoToAddress(void)
{
	bool ok;

	// get the new address
	uint32_t newmemBase = address->text().toUInt(&ok, 16);
	
	// check address validity
	if (ok && (newmemBase >= 0x800000))
	{
		// check the address fitting in the ROM cartridge size
		memBase = newmemBase;
		CheckMaxRomCartSize();
		// refresh content
		RefreshContents();
	}
}
