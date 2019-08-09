//
// CartFilesListWin.cpp - List files in the cartridge
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM   Oct./2018  Created this file, and changed position of the status bar
// JPM   Aug./2019  Update texts descriptions
//

// TO DO:
// To allow file opening / viewing
// Remove/modify the 1st information, named '1', at the top
// To add a filter
//

#include "debugger/CartFilesListWin.h"
#include "memory.h"
#include "settings.h"
#include "debugger/DBGManager.h"


//
CartFilesListWindow::CartFilesListWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
TableView(new QTableView),
model(new QStandardItemModel),
TVlayout(new QVBoxLayout),
Mlayout(new QVBoxLayout),
layout(new QVBoxLayout),
treeView(new QTreeView),
standardModel(new QStandardItemModel),
rootNode(new QStandardItem),
CartDirectory(NULL),
TVstatusbar(new QStatusBar),
fileItems(NULL),
nbItem(0),
CartUsedBytes(0),
CartDirType(CFL_NOTYPE)
{
	setWindowTitle(tr("cartridge directory & files"));

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

	// Set the new layout with proper identation and readibility
#ifdef CFL_BUFFERTREAM
	model->setColumnCount(5);
#else
	model->setColumnCount(4);
#endif
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("File"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Address"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Size"));
	model->setHeaderData(3, Qt::Horizontal, QObject::tr("Seek"));
#ifdef CFL_BUFFERTREAM
	model->setHeaderData(4, Qt::Horizontal, QObject::tr("Stream"));
#endif
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(0);
	TableView->setFont(fixedFont);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	TVlayout->addWidget(TableView);
	TVlayout->addWidget(TVstatusbar);

	// Setup root
	rootNode = standardModel->invisibleRootItem();
	//register the model
	treeView->setModel(standardModel);
	treeView->expandAll();
	Mlayout->addWidget(treeView);

	// Set layouts
	layout->addLayout(TVlayout);
	layout->addLayout(Mlayout);
	setLayout(layout);
}


//
CartFilesListWindow::~CartFilesListWindow(void)
{
}


//
void CartFilesListWindow::Reset(void)
{
	standardModel->setRowCount(0);
	model->setRowCount(0);
	free(CartDirectory);
	free(fileItems);
	fileItems = NULL;
	CartDirectory = NULL;
	CartUsedBytes = CartNbrFiles = CartDirType = nbItem = 0;
}


//
void CartFilesListWindow::RefreshContents(void)
{
	size_t Error;
	char msg[1024];

	if (isVisible())
	{
		if (!CartDirectory)
		{
			if (CartDirType = GetDirType())
			{
				if ((CartNbrFiles = GetNbrFiles()))
				{
					if ((CartDirectory = (CARTDIRINFO *)CreateInfos()))
					{
						UpdateInfos();
						sprintf(msg, "%u files | %u bytes in cartridge", (unsigned int)CartNbrFiles, (unsigned int)CartUsedBytes);
						Error = CFL_NOERROR;
					}
					else
					{
						sprintf(msg, "Cannot use directory");
						Error = CFL_NODIRUSE;
					}
				}
				else
				{
					sprintf(msg, "No files");
					Error = CFL_NOFILESLIST;
				}
			}
			else
			{
				sprintf(msg, "No directory found");
				Error = CFL_NODIRECTORYLIST;
			}

			// Display status bar
			if (Error)
			{
				if ((Error & CFL_WARNING))
				{
					TVstatusbar->setStyleSheet("background-color: lightyellow; font: bold");
				}
				else
				{
					TVstatusbar->setStyleSheet("background-color: tomato; font: bold");
				}
			}
			else
			{
				TVstatusbar->setStyleSheet("background-color: lightgreen; font: bold");
			}
			TVstatusbar->showMessage(QString(msg));
		}
		else
		{
			UpdateInfos();
		}
	}
}


// Get files number in the cartridge directory
size_t CartFilesListWindow::GetNbrFiles(void)
{
	switch (CartDirType)
	{
	case CFL_OSJAGTYPE:
		return ((DBGManager_GetAdrFromSymbolName((char *)"OSJAG_Directory_End") - DBGManager_GetAdrFromSymbolName((char *)"OSJAG_Directory_Deb"))) / sizeof(long);
		break;

	default:
		return 0;
		break;
	}
}


// Get cartridge directory type
size_t CartFilesListWindow::GetDirType(void)
{
	if (DBGManager_GetAdrFromSymbolName((char *)"OSJAG_Directory"))
	{
		return CFL_OSJAGTYPE;
	}
	else
	{
		return CFL_NOTYPE;
	}
}


// Get filename from index (starting from 0)
void CartFilesListWindow::GetFileInfos(CARTDIRINFO *Ptr, size_t index)
{
	OSJAGDir *Adr;
	size_t Offset;

	switch (CartDirType)
	{
	case CFL_OSJAGTYPE:
		Offset = DBGManager_GetAdrFromSymbolName((char *)"OSJAG_Directory_Deb") + (index * sizeof(long));
		Adr = (OSJAGDir *)&jagMemSpace[Offset = GET32(jagMemSpace, Offset)];
		if (strlen(Adr->Filename))
		{
			Ptr->PtrFilename = Adr->Filename;
			Ptr->valid = true;
		}
		else
		{
			Ptr->PtrFilename = (char *)"(null)";
		}
		Ptr->PtrDataFile = GET32(jagMemSpace, Offset);
		Ptr->SizeFile = GET32(jagMemSpace, (Offset + sizeof(long)));
		break;

	default:
		break;
	}
}


// Create information from the cartridge directory information
void *CartFilesListWindow::CreateInfos(void)
{
	CARTDIRINFO *Ptr = (CARTDIRINFO *)calloc(CartNbrFiles, sizeof(CARTDIRINFO));
	model->setRowCount(0);

	for (int i = 0; i < CartNbrFiles; i++)
	{
		GetFileInfos(&Ptr[i], i);
		AddFilename(Ptr[i].PtrFilename, rootNode, 0);
		// Display row content
		model->insertRow(i);
		model->setItem(i, 0, new QStandardItem(QString("%1").arg(Ptr[i].PtrFilename)));
		if (Ptr[i].SizeFile)
		{
			model->setItem(i, 1, new QStandardItem(QString("0x%1").arg(Ptr[i].PtrDataFile, 6, 16, QChar('0'))));
			model->setItem(i, 2, new QStandardItem(QString("0x%1").arg(Ptr[i].SizeFile, 6, 16, QChar('0'))));
		}
		CartUsedBytes += Ptr[i].SizeFile;
	}

	return Ptr;
}


// Update the variables information (seek and stream buffer)
void CartFilesListWindow::UpdateInfos(void)
{
	size_t Offset;

	for (int i = 0; i < CartNbrFiles; i++)
	{
		// Check if file validity (exitence)
		if (CartDirectory[i].valid)
		{
			// Get the current seek and tentatively check validity (must be included in the ram zone)
			Offset = DBGManager_GetAdrFromSymbolName((char *)"OSJAG_SeekPosition") + (i * sizeof(long));
			if ((CartDirectory[i].CurrentSeek = GET32(jagMemSpace, Offset)) < vjs.DRAM_size)
			{
				model->setItem(i, 3, new QStandardItem(QString("0x%1").arg(CartDirectory[i].CurrentSeek, 6, 16, QChar('0'))));
			}

			// Get stream buffer address and check validity (must be included in the ram zone)
			Offset = DBGManager_GetAdrFromSymbolName((char *)"OSJAG_PtrBuffer") + (i * sizeof(long));
			if (((CartDirectory[i].PtrBufferStream = GET32(jagMemSpace, Offset)) < vjs.DRAM_size) && CartDirectory[i].PtrBufferStream)
			{
#ifdef CFL_BUFFERTREAM
				model->setItem(i, 4, new QStandardItem(QString("0x%1").arg(CartDirectory[i].PtrBufferStream, 6, 16, QChar('0'))));
#else
				if (!CartDirectory[i].SizeFile)
				{
					model->setItem(i, 1, new QStandardItem(QString("0x%1").arg(CartDirectory[i].PtrBufferStream, 6, 16, QChar('0'))));
				}
#endif
			}
			else
			{
#ifdef CFL_BUFFERTREAM
				model->setItem(i, 4, new QStandardItem(QString("%1").arg("")));
#else
				if (!CartDirectory[i].SizeFile)
				{
					model->setItem(i, 1, new QStandardItem(QString("%1").arg("")));
				}
#endif
			}
		}
	}
}


// Add source code filename in the list
void CartFilesListWindow::AddFilename(char *FileName, QStandardItem *root, size_t ItemPos)
{
	char *Ptr = FileName;
	Sfileitem *PtrNewFile;
	char Buffer[255];
	char a;

	while ((a = *Ptr++) && ((a != '\\') && (a != '/')));

	if (a)
	{
		strncpy(Buffer, FileName, (Ptr - FileName - 1));
		Buffer[(Ptr - FileName - 1)] = 0;
	}
	else
	{
		strcpy(Buffer, FileName);
	}
	PtrNewFile = (Sfileitem *)AddItem(Buffer, ItemPos);
	if (!PtrNewFile->PreviousItem)
	{
		PtrNewFile->PreviousItem = root;
		root->appendRow(PtrNewFile->Item);
		PtrNewFile->Item->setEditable(false);
	}

	if (a)
	{
		return (AddFilename(Ptr, PtrNewFile->Item, (ItemPos + 1)));
	}
}


// Add item to the list
// Return void * on new item or already existing one
void *CartFilesListWindow::AddItem(char *ItemName, size_t ItemPos)
{
	Sfileitem *Ptr = fileItems;

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
	fileItems = (Sfileitem *)realloc(fileItems, (sizeof(Sfileitem) * ++nbItem));
	(fileItems + (nbItem - 1))->column = ItemPos;
	(fileItems + (nbItem - 1))->PreviousItem = NULL;
	(fileItems + (nbItem - 1))->Item = new QStandardItem(ItemName);
	return (fileItems + (nbItem - 1));
}


// 
void CartFilesListWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
