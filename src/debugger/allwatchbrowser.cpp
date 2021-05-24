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
// JPM  April/2019  Added a sorting filter, tableview unique rows creation
// JPM  April/2021  Added a search feature.
//

// STILL TO DO:
// Better presentation
// To set the information display at the right
// To understand/fix the problem with the sorting filter
// Display arrays information
// Display structures information
//

//#define AW_SORTINGFILTER									// Authorise the sorting filtes
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
symbol(new QLineEdit),
search(new QPushButton(tr("Search"))),
#ifdef AW_LAYOUTTEXTS
text(new QTextBrowser),
#else
TableView(new QTableView),
model(new QStandardItemModel),
#endif
NbWatch(0),
CurrentWatch(0),
statusbar(new QStatusBar),
PtrWatchInfo(NULL)
{
	setWindowTitle(tr("All Watch"));

	// set the font
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

	// search bar
	QHBoxLayout * hbox1 = new QHBoxLayout;
	symbol->setPlaceholderText("symbol name");
	hbox1->addWidget(symbol);
	hbox1->addWidget(search);
	layout->addLayout(hbox1);

	// status bar
	layout->addWidget(statusbar);
	setLayout(layout);

	// connect slot
	connect(search, SIGNAL(clicked()), this, SLOT(SearchSymbol()));
	connect(symbol, SIGNAL(cursorPositionChanged(int, int)), this, SLOT(SelectSearchSymbol()));
}


//
AllWatchBrowserWindow::~AllWatchBrowserWindow(void)
{
	Reset();
}


// Search the symbol in the watch list
void AllWatchBrowserWindow::SearchSymbol(void)
{
	bool found = false;
	size_t i;

	// user cannot enter symbol to allow the search
	symbol->setDisabled(true);

	// look for the symbol in the watch list
	for (i = AW_STARTNUMVARIABLE; (i < NbWatch) && !found; i++)
	{
		// check symbol presence
		if (!symbol->text().compare(((S_VariablesStruct*)PtrWatchInfo[i])->PtrName, Qt::CaseSensitive))
		{
			found = true;
		}
	}

	if (found)
	{
		// remove previous highlight
		if (CurrentWatch)
		{
			model->item((int)(CurrentWatch - 1), 0)->setBackground(QColor(255, 255, 255));
			model->item((int)(CurrentWatch - 1), 1)->setBackground(QColor(255, 255, 255));
			model->item((int)(CurrentWatch - 1), 2)->setBackground(QColor(255, 255, 255));
		}
		// Get the slider maximum position
		int MaxSlider = TableView->verticalScrollBar()->maximum();		
		// Number of items displayed in the scroll bar slider
		int DeltaSlider = (int)NbWatch - MaxSlider;
		// set the scroll bar
		TableView->verticalScrollBar()->setSliderPosition((int)i - (DeltaSlider / 2) - 1);
		// highlight watch symbol
		CurrentWatch = i;
		model->item((int)(CurrentWatch - 1), 0)->setBackground(QColor(0xff, 0xfa, 0xcd));
		model->item((int)(CurrentWatch - 1), 1)->setBackground(QColor(0xff, 0xfa, 0xcd));
		model->item((int)(CurrentWatch - 1), 2)->setBackground(QColor(0xff, 0xfa, 0xcd));
		// allow new symbol
		symbol->setText("");
	}
	else
	{
		// invalid symbol
		symbol->setStyleSheet("color: red");
	}

	// user can enter a symbol
	symbol->setEnabled(true);
	symbol->setFocus();
}


//
void AllWatchBrowserWindow::SelectSearchSymbol(void)
{
	symbol->setStyleSheet("color: black");
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
	//S_VariablesStruct* Var;

	if (isVisible())
	{
		if (!NbWatch)
		{
			// Pre-catch the information for each global variables
			if (NbWatch = DBGManager_GetNbVariables(NULL))
			{
				PtrWatchInfo = (void**)calloc(NbWatch, sizeof(S_VariablesStruct*));
#ifndef AW_LAYOUTTEXTS
#ifdef AW_SORTINGFILTER
				TableView->setSortingEnabled(false);
#endif
				model->setRowCount(0);
#endif
				for (uint32_t i = AW_STARTNUMVARIABLE; i < NbWatch; i++)
				{
					if ((PtrWatchInfo[i] = (void*)DBGManager_GetInfosVariable(NULL, i + 1)))
					{
#ifdef AW_LAYOUTTEXTS
						PtrWatchInfo[i].addr = DBGManager_GetGlobalVariableAdr(i + 1);
						if (!strlen(PtrWatchInfo[i].PtrVariableBaseTypeName = DBGManager_GetGlobalVariableTypeName(i + 1)))
						{
							PtrWatchInfo[i].PtrVariableBaseTypeName = (char *)"<font color='#ff0000'>N/A</font>";
						}
#else
						model->insertRow(i);
#endif
					}
				}
			}
		}

		if (NbWatch)
		{
			for (uint32_t i = AW_STARTNUMVARIABLE; i < NbWatch; i++)
			{
				if (((S_VariablesStruct*)PtrWatchInfo[i])->TypeTag & (DBG_TAG_TYPE_array | DBG_TAG_TYPE_structure))
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
					PtrValue = DBGManager_GetVariableValueFromAdr(((S_VariablesStruct*)PtrWatchInfo[i])->Addr, ((S_VariablesStruct*)PtrWatchInfo[i])->TypeEncoding, ((S_VariablesStruct*)PtrWatchInfo[i])->TypeByteSize);
				}
#ifdef AW_LAYOUTTEXTS
				if (i)
				{
					WatchAll += QString("<br>");
				}
				sprintf(string, "%i : %s | %s | 0x%06X | %s", (i + 1), PtrWatchInfo[i].PtrVariableBaseTypeName, PtrWatchInfo[i].PtrVariableName, (unsigned int)PtrWatchInfo[i].addr, PtrValue ? PtrValue : (char *)"<font color='#ff0000'>N/A</font>");
				WatchAll += QString(string);
#else
				model->setItem(i, 0, new QStandardItem(QString("%1").arg(((S_VariablesStruct*)PtrWatchInfo[i])->PtrName)));
				model->setItem(i, 1, new QStandardItem(QString("%1").arg(PtrValue)));
				model->setItem(i, 2, new QStandardItem(QString("%1").arg(((S_VariablesStruct*)PtrWatchInfo[i])->PtrTypeName)));
#endif
			}
#ifdef AW_LAYOUTTEXTS
			text->clear();
			text->setText(WatchAll);
#else
#ifdef AW_SORTINGFILTER
			TableView->setSortingEnabled(true);
#endif
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


//  Handle keyboard event
void AllWatchBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	// ESC to close / hide the window
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		// search symbol
		if (e->key() == Qt::Key_Return)
		{
			SearchSymbol();
		}
	}
}

