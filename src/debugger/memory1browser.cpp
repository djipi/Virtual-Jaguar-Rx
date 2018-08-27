//
// memory1browser.cpp - Jaguar memory window 1 browser
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  08/07/2017  Created this file
//

// STILL TO DO:
//

#include "memory1browser.h"
#include "memory.h"
#include "debugger/DBGManager.h"
#include "settings.h"


//
Memory1BrowserWindow::Memory1BrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
//	layout(new QVBoxLayout), text(new QTextBrowser),
	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh"))),
	address(new QLineEdit),
	go(new QPushButton(tr("Go"))),
	memBase(0), memOrigin(0), NumWinOrigin(0)
{
	//setWindowTitle(tr("Memory 1 Browser"));

	//address->setInputMask("hhhhhh");
	address->setPlaceholderText("0x<value>, decimal value or symbol name");

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
	//layout->addWidget(refresh);
	layout->addLayout(hbox1);

	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContentsWindow()));
	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
}


// Refresh / Display the window contents
void Memory1BrowserWindow::RefreshContents(size_t NumWin)
{
	char string[100];

	if (isVisible())
	{
		sprintf(string, "Memory %i - 0x%06X", (unsigned int)((NumWinOrigin = NumWin) + 1), (unsigned int)memOrigin);
		setWindowTitle(tr(string));
		RefreshContentsWindow();
	}
}


// Refresh / Display the window contents
void Memory1BrowserWindow::RefreshContentsWindow(void)
{
	char string[1024], buf[64];
	QString memDump;
	size_t i, j;
	uint8_t c;

	for (i = 0; i < 480; i += 16)
	{
		sprintf(string, "%s%06X: ", (i ? "<br>" : ""), (unsigned int)(memBase + i));

		for (j = 0; j < 16; j++)
		{
			sprintf(buf, "%02X ", jaguarMainRAM[memBase + i + j]);
			strcat(string, buf);
		}

		sprintf(buf, "| ");
		strcat(string, buf);

		for (j = 0; j < 16; j++)
		{
			c = jaguarMainRAM[memBase + i + j];
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


//
void Memory1BrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		if (e->key() == Qt::Key_PageUp)
		{
			if ((memBase -= 480) < 0)
			{
				memBase = 0;
			}

			RefreshContentsWindow();
		}
		else
		{
			if (e->key() == Qt::Key_PageDown)
			{
				if ((memBase += 480) > (vjs.DRAM_size - 480))
				{
					memBase = vjs.DRAM_size - 480;
				}

				RefreshContentsWindow();
			}
			else
			{
				if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
				{
					if ((memBase -= 16) < 0)
					{
						memBase = 0;
					}

					RefreshContentsWindow();
				}
				else
				{
					if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
					{
						if ((memBase += 16) > (vjs.DRAM_size - 480))
						{
							memBase = vjs.DRAM_size - 480;
						}

						RefreshContentsWindow();
					}
					else
					{
						if (e->key() == Qt::Key_Return)
						{
							GoToAddress();
						}
					}
				}
			}
		}
	}
}


// Go to the requested address
// Address can be an hexa, decimal or a symbol name
void Memory1BrowserWindow::GoToAddress(void)
{
	bool ok;
	size_t len;
	QString newAddress;
	size_t newmemBase;

	QPalette p = address->palette();
	newAddress = address->text();

	if ((len = newAddress.size()))
	{
		if ((len > 1) && (newAddress.at(0) == QChar('0')) && (newAddress.at(1) == QChar('x')))
		{
			newmemBase = newAddress.toUInt(&ok, 16);
		}
		else
		{
			if (!(newmemBase = DBGManager_GetAdrFromSymbolName(newAddress.toLatin1().data())))
			{
				newmemBase = newAddress.toUInt(&ok, 10);
			}
			else
			{
				ok = true;
			}
		}

		if (!ok || (newmemBase < 0) || (newmemBase > vjs.DRAM_size))
		{
			p.setColor(QPalette::Text, Qt::red);
		}
		else
		{
			p.setColor(QPalette::Text, Qt::black);
			memOrigin = (memBase = newmemBase);
			RefreshContents(NumWinOrigin);
		}
		address->setPalette(p);
	}
}
