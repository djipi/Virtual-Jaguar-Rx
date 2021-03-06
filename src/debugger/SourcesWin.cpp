//
//  SourcesWin.cpp - Sources tracing window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  08/23/2019  Created this file
// JPM   Apr./2021  Fixed potential crash with the tabs reset

// STILL TO DO:
// Use the CloseTab signal's value instead to close the current tab
//

#include <stdlib.h>
#include <string.h>
#include "DBGManager.h"
#include "debugger/SourceCWin.h"
#include "debugger/SourcesWin.h"
#include "m68000/m68kinterface.h"


// 
SourcesWindow::SourcesWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
sourcestabWidget(new QTabWidget),
sourcesinfostab(0),
NbSourcesInfos(0),
CurrentTab(0),
OldCurrentTab(0),
OldCurrentNumLineSrc(0),
indexErrorTab(-1),
sourceErrorTab(0)
{
	// prepare layout
	sourcestabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	sourcestabWidget->setTabsClosable(true);
	layout->addWidget(sourcestabWidget);

	// set layout
	setLayout(layout);

	// connect the signals
	connect(sourcestabWidget, SIGNAL(currentChanged(const int)), this, SLOT(SelectTab(const int)));
	connect(sourcestabWidget, SIGNAL(tabCloseRequested(const int)), this, SLOT(CloseTab(const int)));
}


// Close #tab
void SourcesWindow::CloseTab(const int)
{
	CloseCurrentTab();
}


// Select tab
void SourcesWindow::SelectTab(const int)
{
#if 0
	size_t i = 0;
	QString t = sourcestabWidget->tabText(index);
	while ((i < NbSourcesInfos) && strcmp(sourcesinfostab[i++].Filename, t.toLatin1().data()));

	if ((i != NbSourcesInfos) && (sourcesinfostab[i - 1].IndexTab != -1) && (CurrentTab == (i - 1)))
	{
		sourcesinfostab[(i - 1)].sourceCtab->RefreshContents();
	}
#endif
}


// Sources initialisation
// Prepare tabs for every available source code file
void SourcesWindow::Init(void)
{
	size_t i, j;
	char *Ptr, *Ptr1;

	// get number of sources
	NbSourcesInfos = DBGManager_GetNbSources();
	if (NbSourcesInfos)
	{
		// alloc structure for the source informations
		sourcesinfostab = (SourcesInfos *)calloc(NbSourcesInfos, sizeof(SourcesInfos));

		// fill sources information
		for (i = 0; i < NbSourcesInfos; i++)
		{
			// get source filename without misguiding information
			Ptr = DBGManager_GetNumSourceFilename(i);
			Ptr1 = sourcesinfostab[i].Filename = (char *)malloc(strlen(Ptr) + 1);
			while (((*Ptr == '.') || ((*Ptr == '/') || (*Ptr == '\\'))) && Ptr++);
			strcpy(Ptr1, Ptr);
			// get texts dedicated information
			for (j = 0; j < 2; j++)
			{
				sourcesinfostab[i].NbLinesText[j] = DBGManager_GetSrcNbListPtrFromIndex(i, j);
			}
			sourcesinfostab[i].NumLinesUsed = DBGManager_GetSrcNumLinesPtrFromIndex(i, true);
			sourcesinfostab[i].SourceText = DBGManager_GetSrcListPtrFromIndex(i, false);
			// get remaining information
			sourcesinfostab[i].Language = DBGManager_GetSrcLanguageFromIndex(i);
			sourcesinfostab[i].IndexTab = -1;
		}
	}
	else
	{
		// no source files exist
		sourcesinfostab = NULL;
	}
}


// Get the tracing status
bool SourcesWindow::GetTraceStatus(void)
{
	if (NbSourcesInfos)
	{
		switch (sourcesinfostab[CurrentTab].Language)
		{
		case DBG_LANG_VASM_Assembler:
			break;

		default:
			return true;
			break;
		}
	}

	return false;
}


// Check if the line's number has changed across the tabs
// Return true in case of the line is different, otherwise return false
bool SourcesWindow::CheckChangeLine(void)
{
	size_t NumLine;

	// get the line number based on the current M68K PC address
	if (NumLine = DBGManager_GetNumLineFromAdr(m68k_get_reg(NULL, M68K_REG_PC), DBG_NO_TAG))
	{
		if (OldCurrentTab == CurrentTab)
		{
			if (OldCurrentNumLineSrc != NumLine)
			{
				OldCurrentNumLineSrc = NumLine;
				return true;
			}
		}
		else
		{
			OldCurrentTab = CurrentTab;
			OldCurrentNumLineSrc = 0;
		}
	}

	return false;
}


// Refresh tabs
// Open a new tab for a file source code
// Set a unique tab for unavailable source code
void SourcesWindow::RefreshContents(void)
{
	size_t m68kPC = m68k_get_reg(NULL, M68K_REG_PC);
	int index = 0;
	size_t i;
	DBGstatus Status;
	char *Filename;

	// check valid M68K PC address
	if (m68kPC && NbSourcesInfos)
	{
		// get source filename pointed by PC address
		Filename = DBGManager_GetFullSourceFilenameFromAdr(m68kPC, &Status);
		if (!Status && Filename)
		{
			// look for a new tab
			for (i = 0; i < NbSourcesInfos; i++, !index)
			{
				if (sourcesinfostab[i].Filename)
				{
					if (strstr(Filename, sourcesinfostab[i].Filename))
					{
						// open a new tab for a source code
						if (sourcesinfostab[i].IndexTab == -1)
						{
							sourcesinfostab[i].IndexTab = index = sourcestabWidget->addTab(sourcesinfostab[i].sourceCtab = new(SourceCWindow), tr(sourcesinfostab[i].Filename));
							sourcesinfostab[i].sourceCtab->FillTab(i, sourcesinfostab[i].SourceText, sourcesinfostab[i].NbLinesText, sourcesinfostab[i].NumLinesUsed);
						}

						sourcestabWidget->setCurrentIndex(sourcesinfostab[i].IndexTab);
						sourcesinfostab[CurrentTab].sourceCtab->SetCursorTrace(sourcesinfostab[CurrentTab].CurrentNumLineSrc, true);
						sourcesinfostab[i].sourceCtab->SetCursorTrace(sourcesinfostab[i].CurrentNumLineSrc = (int)DBGManager_GetNumLineFromAdr(m68kPC, DBG_NO_TAG), false);
						sourcesinfostab[i].sourceCtab->RefreshContents();
						CurrentTab = i;
					}
				}
			}
		}
		else
		{
			// source file doesn't exist
			if (indexErrorTab == -1)
			{
				indexErrorTab = sourcestabWidget->addTab(sourceErrorTab = new(SourceCWindow), tr("Source file not available"));
				//sourceErrorTab->hide();
			}
			sourcestabWidget->setCurrentIndex(indexErrorTab);
		}
	}
}


// Close / Remove current tab
void SourcesWindow::CloseCurrentTab(void)
{
	size_t i = 0;
	int Index;
	QString t = sourcestabWidget->tabText((Index = sourcestabWidget->currentIndex()));

	// check error tab presence
	if (indexErrorTab == Index)
	{
		// close the error tab
		indexErrorTab = -1;
	}
	else
	{
		// close source code text tab
		while ((i < NbSourcesInfos) && strcmp(sourcesinfostab[i++].Filename, t.toLatin1().data()));
		sourcesinfostab[(i - 1)].IndexTab = -1;
	}

	// remove the tab
	sourcestabWidget->removeTab(Index);
}


// Handle keyboard
void SourcesWindow::keyPressEvent(QKeyEvent * e)
{
	// close/remove the current tab
	if (e->key() == Qt::Key_Escape)
	{
		CloseCurrentTab();
	}
}


// Reset all source files tab
void SourcesWindow::Reset(void)
{
	// clear the tabs
	sourcestabWidget->clear();

	// clear tab information
	while (NbSourcesInfos)
	{
		free(sourcesinfostab[--NbSourcesInfos].Filename);
	}
	free(sourcesinfostab);
}


// 
void SourcesWindow::Close(void)
{
	Reset();
}
