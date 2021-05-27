//
// FilesrcListWin.cpp - List all source code filenames
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  09/23/2018  Created this file
//

// STILL TO DO:
// Remove the 1st information, named '1', at the top
// To allow source code file opening / viewing
//

#include "debugger/FilesrcListWin.h"
//#include "memory.h"
#include "debugger/DBGManager.h"


//
FilesrcListWindow::FilesrcListWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
treeView(new QTreeView),
standardModel(new QStandardItemModel),
rootNode(new QStandardItem),
filesrcItems(NULL),
statusbar(new QStatusBar),
nbItem(0)
{
	// Setup root
	rootNode = standardModel->invisibleRootItem();
	//register the model
	treeView->setModel(standardModel);
	treeView->expandAll();
	layout->addWidget(treeView);

	// Status bar
	layout->addWidget(statusbar);
	setLayout(layout);
}


//
FilesrcListWindow::~FilesrcListWindow(void)
{
}


//
void FilesrcListWindow::Reset(void)
{
	standardModel->setRowCount(0);
	free(filesrcItems);
	filesrcItems = NULL;
	nbItem = 0;
}


//
void FilesrcListWindow::RefreshContents(void)
{
	size_t Error, Nbr;
	char msg[1024];

	if (!filesrcItems)
	{
		if ((Nbr = UpdateInfos()))
		{
			sprintf(msg, "%i files found", Nbr);
			Error = FSL_NOERROR;
		}
		else
		{
			sprintf(msg, "No files found");
			Error = FSL_NOFILESRCLIST;
		}

		// Display status bar
		if (Error)
		{
			if ((Error & FSL_WARNING))
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
size_t FilesrcListWindow::UpdateInfos(void)
{
	size_t Nbr, i;

	Nbr = DBGManager_GetNbSources();

	for (i = 0; i < Nbr; i++)
	{
		AddFilename(DBGManager_GetNumFullSourceFilename(i), rootNode, 0);
	}

	return Nbr;
}


// Add source code filename in the list
void FilesrcListWindow::AddFilename(char *FileName, QStandardItem *root, size_t ItemPos)
{
	char *Ptr = FileName;
	Sfilesrcitem *PtrNewFilesrc;
	char Buffer[255];
	char a;

#ifdef _WIN32
	while ((a = *Ptr++) && (a != '\\'));
#else
	while ((a = *Ptr++) && (a != '/'));
#endif
	if (a)
	{
		strncpy(Buffer, FileName, (Ptr - FileName - 1));
		Buffer[(Ptr - FileName - 1)] = 0;
	}
	else
	{
		strcpy(Buffer, FileName);
	}
	PtrNewFilesrc = (Sfilesrcitem *)AddItem(Buffer, ItemPos);
	if (!PtrNewFilesrc->PreviousItem)
	{
		PtrNewFilesrc->PreviousItem = root;
		root->appendRow(PtrNewFilesrc->Item);
		PtrNewFilesrc->Item->setEditable(false);
	}

	if (a)
	{
		return (AddFilename(Ptr, PtrNewFilesrc->Item, (ItemPos + 1)));
	}
}


// Add item to the list
// Return void * on new item or already existing one
void *FilesrcListWindow::AddItem(char *ItemName, size_t ItemPos)
{
	Sfilesrcitem *Ptr = filesrcItems;

	// Look for already existing item
	for (size_t i = 0; i < nbItem; i++)
	{
		if ((Ptr->column == ItemPos) && !strcmp(Ptr->Item->text().toLocal8Bit().constData(), ItemName))
		{
			return Ptr;
		}
		else
		{
			Ptr++;
		}
	}

	// Add item in the list
	filesrcItems = (Sfilesrcitem *)realloc(filesrcItems, (sizeof(Sfilesrcitem) * ++nbItem));
	(filesrcItems + (nbItem - 1))->column = ItemPos;
	(filesrcItems + (nbItem - 1))->PreviousItem = NULL;
	(filesrcItems + (nbItem - 1))->Item = new QStandardItem(ItemName);
	return (filesrcItems + (nbItem - 1));
}
