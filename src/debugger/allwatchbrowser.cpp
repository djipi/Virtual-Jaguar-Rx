//
// allwatchbrowser.cpp - All Watch
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
// Better presentation
//

#include "debugger/allwatchbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"


// 
AllWatchBrowserWindow::AllWatchBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QTextBrowser),
	NbWatch(0),
	PtrWatchInfo(NULL)
{
	setWindowTitle(tr("All Watch"));

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	setLayout(layout);

	layout->addWidget(text);
}


//
AllWatchBrowserWindow::~AllWatchBrowserWindow(void)
{
	Reset();
}


//
void AllWatchBrowserWindow::Reset(void)
{
	free(PtrWatchInfo);
	NbWatch = 0;
	PtrWatchInfo = NULL;
}


//
void AllWatchBrowserWindow::RefreshContents(void)
{
	char string[1024];
	QString WatchAll;

	if (isVisible())
	{
		text->clear();

		if (!NbWatch)
		{
			if (NbWatch = DBGManager_GetNbGlobalVariables())
			{
				PtrWatchInfo = (WatchInfo *)calloc(NbWatch, sizeof(WatchInfo));

				for (uint32_t i = 0; i < NbWatch; i++)
				{
					PtrWatchInfo[i].PtrVariableName = DBGManager_GetGlobalVariableName(i + 1);
					PtrWatchInfo[i].addr = DBGManager_GetGlobalVariableAdr(i + 1);
					PtrWatchInfo[i].TypeTag = DBGManager_GetGlobalVariableTypeTag(i + 1);
					if (!strlen(PtrWatchInfo[i].PtrVariableBaseTypeName = DBGManager_GetGlobalVariableTypeName(i + 1)))
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
				sprintf(string, "%i : %s | %s | 0x%06X | %s", (i + 1), PtrWatchInfo[i].PtrVariableBaseTypeName, PtrWatchInfo[i].PtrVariableName, (unsigned int)PtrWatchInfo[i].addr, (PtrWatchInfo[i].TypeTag & 0x8) ? "" : DBGManager_GetGlobalVariableValue(i + 1));
				WatchAll += QString(string);
				sprintf(string, "<br>");
				WatchAll += QString(string);
			}
		}

		text->setText(WatchAll);
	}
}


// 
void AllWatchBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}

