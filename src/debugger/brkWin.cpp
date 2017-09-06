//
// brkWin.cpp - Breakpoints
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  30/08/2017  Created this file
//

// STILL TO DO:
//

#include "debugger\brkWin.h"
//#include "memory.h"
#include "debugger\DBGManager.h"


//
BrkWindow::BrkWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QTextBrowser),
//	layout(new QVBoxLayout), text(new QLabel),
//	refresh(new QPushButton(tr("Refresh"))),
	address(new QLineEdit)
//	go(new QPushButton(tr("Go"))),
//	memBase(0),
//	NbWatch(0),
//	PtrWatchInfo(NULL)
{
	setWindowTitle(tr("Breakpoints window"));

#if 0
//	address->setInputMask("hhhhhh");
//	QHBoxLayout * hbox1 = new QHBoxLayout;
//	hbox1->addWidget(refresh);
//	hbox1->addWidget(address);
//	hbox1->addWidget(go);

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
//	layout->addLayout(hbox1);

//	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
//	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
#endif
}


//
BrkWindow::~BrkWindow(void)
{
#if 0
	NbWatch = 0;
	free(PtrWatchInfo);
#endif
}


//
void BrkWindow::RefreshContents(void)
{
#if 0
	char string[1024];
//	char buf[64];
	QString WatchAll;

	if (isVisible())
	{
		if (!NbWatch)
		{
			if (NbWatch = DBGManager_GetNbExternalVariables())
			{
				PtrWatchInfo = (WatchInfo *)calloc(NbWatch, sizeof(WatchInfo));
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to check the memory desalocation for PtrWatchInfo !!!")
#else
				#warning "!!! Need to do the memory desalocation for PtrWatchInfo !!!"
#endif // _MSC_VER
					
				for (uint32_t i = 0; i < NbWatch; i++)
				{
					PtrWatchInfo[i].PtrVariableName = DBGManager_GetExternalVariableName(i + 1);
					PtrWatchInfo[i].addr = DBGManager_GetExternalVariableAdr(i + 1);
					PtrWatchInfo[i].TypeTag = DBGManager_GetExternalVariableTypeTag(i + 1);
					if (!strlen(PtrWatchInfo[i].PtrVariableBaseTypeName = DBGManager_GetExternalVariableTypeName(i + 1)))
					{
						PtrWatchInfo[i].PtrVariableBaseTypeName = (char *)"<font color='#ff0000'>N/A</font>";
					}
				}
			}
		}

		for (uint32_t i = 0; i < NbWatch; i++)
		{
			if (PtrWatchInfo[i].PtrVariableName && PtrWatchInfo[i].PtrVariableBaseTypeName)
			{
				sprintf(string, "%i : %s | %s | 0x%06X | %s", (i + 1), PtrWatchInfo[i].PtrVariableBaseTypeName, PtrWatchInfo[i].PtrVariableName, PtrWatchInfo[i].addr, (PtrWatchInfo[i].TypeTag & 0x8) ? "" : DBGManager_GetExternalVariableValue(i + 1));
				WatchAll += QString(string);
				sprintf(string, "<br>");
				WatchAll += QString(string);
			}
		}

		text->clear();
		text->setText(WatchAll);
	}
#endif
}


//
void BrkWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		if (e->key() == Qt::Key_PageUp)
		{
#if 0
			memBase -= 480;

			if (memBase < 0)
				memBase = 0;

			RefreshContents();
#endif
		}
		else
		{
			if (e->key() == Qt::Key_PageDown)
			{
#if 0
				memBase += 480;

				if (memBase > (0x200000 - 480))
					memBase = 0x200000 - 480;

				RefreshContents();
#endif
			}
			else
			{
				if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
				{
#if 0
					memBase -= 16;

					if (memBase < 0)
						memBase = 0;

					RefreshContents();
#endif
				}
				else
				{
					if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
					{
#if 0
						memBase += 16;

						if (memBase > (0x200000 - 480))
							memBase = 0x200000 - 480;

						RefreshContents();
#endif
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


//
void BrkWindow::RefreshBrkList(size_t Address)
{
}


// Go to the requested address
// Address can be an hexa, decimal or a symbol name
void BrkWindow::GoToAddress(void)
{
	size_t Address;
	bool ok;
	QString newAddress;

	newAddress = address->text();

	if ((newAddress.at(0) == QChar('0')) && (newAddress.at(1) == QChar('x')))
	{
		Address = newAddress.toUInt(&ok, 16);
	}
	else
	{
		if (!(Address = DBGManager_GetAdrFromSymbolName(newAddress.toLatin1().data())))
		{
			Address = newAddress.toUInt(&ok, 10);
		}
	}

	RefreshBrkList(Address);
}

