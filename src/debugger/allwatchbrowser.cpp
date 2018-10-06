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
// JPM  09/14/2018  Added a status bar, better status report and set information values in a tab
// JPM  10/05/2018  Added a sorting filter 
//

// STILL TO DO:
// Better presentation
// To set the information display at the right
//

//#define AW_DEBUGNUMVARIABLE		4415						// Set the global variable number to debug
#ifndef AW_DEBUGNUMVARIABLE
#define AW_STARTNUMVARIABLE		0							// Must be kept to 0 in case of no debug is required
#else
#define AW_STARTNUMVARIABLE		AW_DEBUGNUMVARIABLE - 1
#endif


#include "debugger/allwatchbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"


// 
AllWatchBrowserWindow::AllWatchBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
#ifdef AW_LAYOUTTEXTS
text(new QTextBrowser),
#else
TableView(new QTableView),
model(new QStandardItemModel),
#endif
NbWatch(0),
statusbar(new QStatusBar),
PtrWatchInfo(NULL)
{
	setWindowTitle(tr("All Watch"));

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

#ifdef AW_LAYOUTTEXTS
	// Set original layout
	text->setFont(fixedFont);
	layout->addWidget(text);
#else
	// Set the new layout with proper identation and readibility
	model->setColumnCount(3);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Value"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(0);
	TableView->setFont(fixedFont);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	layout->addWidget(TableView);
#endif

	// Status bar
	layout->addWidget(statusbar);
	setLayout(layout);
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
	char msg[100];
#ifdef AW_LAYOUTTEXTS
	char string[1024];
#endif
	QString WatchAll;
	size_t Error = AW_NOERROR;
	char *PtrValue;

	if (isVisible())
	{
		if (!NbWatch)
		{
			// Pre-catch the information for each global variables
			if (NbWatch = DBGManager_GetNbGlobalVariables())
			{
				PtrWatchInfo = (WatchInfo *)calloc(NbWatch, sizeof(WatchInfo));

				for (uint32_t i = AW_STARTNUMVARIABLE; i < NbWatch; i++)
				{
					PtrWatchInfo[i].PtrVariableName = DBGManager_GetGlobalVariableName(i + 1);
					PtrWatchInfo[i].TypeTag = DBGManager_GetGlobalVariableTypeTag(i + 1);
#ifdef AW_LAYOUTTEXTS
					PtrWatchInfo[i].addr = DBGManager_GetGlobalVariableAdr(i + 1);
					if (!strlen(PtrWatchInfo[i].PtrVariableBaseTypeName = DBGManager_GetGlobalVariableTypeName(i + 1)))
					{
						PtrWatchInfo[i].PtrVariableBaseTypeName = (char *)"<font color='#ff0000'>N/A</font>";
					}
#else
					PtrWatchInfo[i].PtrVariableBaseTypeName = DBGManager_GetGlobalVariableTypeName(i + 1);
#endif
				}
			}
		}
#ifndef AW_LAYOUTTEXTS
		TableView->setSortingEnabled(false);
		model->setRowCount(0);
#endif
		if (NbWatch)
		{
			for (uint32_t i = AW_STARTNUMVARIABLE; i < NbWatch; i++)
			{
				if ((PtrWatchInfo[i].TypeTag & (DBG_TAG_TYPE_array | DBG_TAG_TYPE_structure)))
				{
#if defined(AW_SUPPORTARRAY) || defined(AW_SUPPORTSTRUCTURE)
					//PtrValue = (char *)memcpy(Value, &jaguarMainRAM[PtrWatchInfo[i].addr], 20);
					PtrValue = NULL;
#else
					PtrValue = NULL;
#endif
				}
				else
				{
					PtrValue = DBGManager_GetGlobalVariableValue(i + 1);
				}
#ifdef AW_LAYOUTTEXTS
				if (i)
				{
					WatchAll += QString("<br>");
				}
				sprintf(string, "%i : %s | %s | 0x%06X | %s", (i + 1), PtrWatchInfo[i].PtrVariableBaseTypeName, PtrWatchInfo[i].PtrVariableName, (unsigned int)PtrWatchInfo[i].addr, PtrValue ? PtrValue : (char *)"<font color='#ff0000'>N/A</font>");
				WatchAll += QString(string);
#else
				model->insertRow(i);
				model->setItem(i, 0, new QStandardItem(QString("%1").arg(PtrWatchInfo[i].PtrVariableName)));
				model->setItem(i, 1, new QStandardItem(QString("%1").arg(PtrValue)));
				model->setItem(i, 2, new QStandardItem(QString("%1").arg(PtrWatchInfo[i].PtrVariableBaseTypeName)));
#endif
			}
#ifdef AW_LAYOUTTEXTS
			text->clear();
			text->setText(WatchAll);
#else
			TableView->setSortingEnabled(true);
#endif
			sprintf(msg, "Ready");
		}
		else
		{
			sprintf(msg, "No watches");
			Error = AW_NOALLWATCH;
		}

		// Display status bar
		if (Error)
		{
			if ((Error & AW_WARNING))
			{
				statusbar->setStyleSheet("background-color: lightyellow; font: bold");
			}
			else
			{
				statusbar->setStyleSheet("background-color: tomato; font: bold");
			}
		}
		else
		{
			statusbar->setStyleSheet("background-color: lightgreen; font: bold");
		}
		statusbar->showMessage(QString(msg));
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

