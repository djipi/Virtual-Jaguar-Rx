//
// allwatch.cpp - All Watch
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  12/07/2017  Created this file
//

// STILL TO DO:
//

#include "debugger/allwatchbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"


AllWatchBrowserWindow::AllWatchBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QTextBrowser),
//	layout(new QVBoxLayout), text(new QLabel),
//	refresh(new QPushButton(tr("Refresh"))),
//	address(new QLineEdit),
//	go(new QPushButton(tr("Go"))),
//	memBase(0),
	NbWatch(0),
	PtrWatchInfo(NULL)
{
	setWindowTitle(tr("All Watch"));

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
}


//
AllWatchBrowserWindow::~AllWatchBrowserWindow(void)
{
	NbWatch = 0;
	free(PtrWatchInfo);
}


//
void AllWatchBrowserWindow::RefreshContents(void)
{
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
				sprintf(string, "%i : %s | %s | 0x%06X | %s", (i + 1), PtrWatchInfo[i].PtrVariableBaseTypeName, PtrWatchInfo[i].PtrVariableName, (unsigned int)PtrWatchInfo[i].addr, (PtrWatchInfo[i].TypeTag & 0x8) ? "" : DBGManager_GetExternalVariableValue(i + 1));
				WatchAll += QString(string);
				sprintf(string, "<br>");
				WatchAll += QString(string);
			}
		}

		text->clear();
		text->setText(WatchAll);
	}
}


#if 0
void AllWatchBrowserWindow::keyPressEvent(QKeyEvent * e)
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
#endif


#if 0
void AllWatchBrowserWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
#endif

