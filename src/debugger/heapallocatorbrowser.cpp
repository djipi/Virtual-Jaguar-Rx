//
// heapallocatorbrowser.cpp: Memory heap allocation
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  01/08/2017  Created this file
// JPM  09/05/2018  Support of the DRAM size limit option
// JPM  09/05/2018  Use definitions for error instead of hard values
// JPM  09/05/2018  Detect if heap allocation shares space with SP (Stack)
// JPM  09/06/2018  Added a status bar and better status report
// JPM  09/07/2018  Set information values in a tab
//

// STILL TO DO:
// To have filters
// To set the information display at the right
// Feature to list the pointer(s) in the code using the allocation
//


#include "settings.h"
#include "debugger/heapallocatorbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"


// 
HeapAllocatorBrowserWindow::HeapAllocatorBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
#ifdef HA_LAYOUTTEXTS
text(new QTextBrowser),
#else
TableView(new QTableView),
model(new QStandardItemModel),
proxyModel(new QSortFilterProxyModel),
#endif
statusbar(new QStatusBar),
Adr(0)
{
	setWindowTitle(tr("Heap Allocation"));

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

#ifdef HA_LAYOUTTEXTS
	// Set original layout
	text->setFont(fixedFont);
	layout->addWidget(text);
#else
	// Set the new layout with proper identation and readibility
	model->setColumnCount(3);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Pointer"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Size"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Use"));
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(0);
	TableView->setFont(fixedFont);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	layout->addWidget(TableView);
	// Set filter
	proxyModel->setSourceModel(model);
	QRegExp regExp("*", Qt::CaseInsensitive, QRegExp::Wildcard);
	proxyModel->setFilterRegExp(regExp);
#endif

	// Status bar
	layout->addWidget(statusbar);
	setLayout(layout);
}


//
HeapAllocatorBrowserWindow::~HeapAllocatorBrowserWindow(void)
{
}


//
void HeapAllocatorBrowserWindow::RefreshContents(void)
{
#ifdef HA_LAYOUTTEXTS
	char string[1024] = { 0 };
	QString HA;
#endif
	char msg[1024];
	QString MSG;
	size_t Adr68K, Adr68KHigh;
	size_t Error = HA_NOERROR;
	size_t NbBlocks, TotalBytesUsed;
	HeapAllocation HeapAllocation;

	if (isVisible())
	{
		if (Adr68K = Adr)
		{
			Adr68KHigh = TotalBytesUsed = NbBlocks = 0;
#ifndef HA_LAYOUTTEXTS
			model->setRowCount(0);
#endif
			do
			{
				if ((Adr68K >= 0x4000) && (Adr68K < vjs.DRAM_size))
				{
					if (Adr68K < m68k_get_reg(NULL, M68K_REG_SP))
					{
						memcpy(&HeapAllocation, &jaguarMainRAM[Adr68K], sizeof(HeapAllocation));

						if (HeapAllocation.size = ((HeapAllocation.size & 0xff) << 24) + ((HeapAllocation.size & 0xff00) << 8) + ((HeapAllocation.size & 0xff0000) >> 8) + ((HeapAllocation.size & 0xff000000) >> 24))
						{
							if (HeapAllocation.size <= (vjs.DRAM_size - 0x4000))
							{
								if ((HeapAllocation.used = ((HeapAllocation.used & 0xff) << 8) + ((HeapAllocation.used & 0xff00) >> 8)) <= 1)
								{
									HeapAllocation.nextalloc = ((HeapAllocation.nextalloc & 0xff) << 24) + ((HeapAllocation.nextalloc & 0xff00) << 8) + ((HeapAllocation.nextalloc & 0xff0000) >> 8) + ((HeapAllocation.nextalloc & 0xff000000) >> 24);

									if ((HeapAllocation.nextalloc >= 0x4000) && (HeapAllocation.nextalloc < vjs.DRAM_size))
									{
#ifdef HA_LAYOUTTEXTS
										if (NbBlocks++)
										{
											HA += QString("<br>");
										}
										sprintf(string, "0x%06x | 0x%0x (%zi) | %s | 0x%06x", Adr68K, HeapAllocation.size - sizeof(HeapAllocation), HeapAllocation.size - sizeof(HeapAllocation), HeapAllocation.used ? "Allocated" : "Free", HeapAllocation.nextalloc);
										HA += QString(string);
#else
										model->insertRow(NbBlocks);
										model->setItem(NbBlocks, 0, new QStandardItem(QString("0x%1").arg(Adr68K, 6, 16, QChar('0'))));
										model->setItem(NbBlocks, 1, new QStandardItem(QString("%1").arg((HeapAllocation.size - sizeof(HeapAllocation)))));
										model->setItem(NbBlocks++, 2, new QStandardItem(QString("%1").arg(HeapAllocation.used ? "Allocated" : "Free")));
#endif
										TotalBytesUsed += HeapAllocation.size;

										if ((Adr68K = HeapAllocation.nextalloc) > Adr68KHigh)
										{
											Adr68KHigh = Adr68K;
										}
									}
									else
									{
										sprintf(msg, "Unable to determine the next memory allocation");
										Error = HA_UNABLENEXTMEMORYALLOC;
									}
								}
								else
								{
									sprintf(msg, "Unable to determine if the allocated memory is used or not");
									Error = HA_UNABLEALLOCATEMEMORYUSAGE;
								}
							}
							else
							{
								sprintf(msg, "Memory bloc size has a problem");
								Error = HA_MEMORYBLOCKSIZEPROBLEM;
							}
						}
						else
						{
							sprintf(msg, "%i blocks | %i bytes in blocks | %i contiguous bytes free", NbBlocks, TotalBytesUsed, (m68k_get_reg(NULL, M68K_REG_SP) - Adr68KHigh));
						}
					}
					else
					{
						sprintf(msg, "Memory allocations and Stack have reached the same space");
						Error = HA_HAANDSPSHARESPACE;
					}
				}
				else
				{
					sprintf(msg, "Memory allocations may have a problem");
					Error = HA_MEMORYALLOCATIONPROBLEM;
				}
			}
			while (HeapAllocation.size && !Error);

			MSG += QString(msg);
		}
		else
		{
			if (Adr = DBGManager_GetAdrFromSymbolName((char *)"__HeapBase"))
			{
				if (Adr68K = DBGManager_GetGlobalVariableAdrFromName((char *)"alloc"))
				{
					if (!(Adr68K = (jaguarMainRAM[Adr68K] << 24) + (jaguarMainRAM[Adr68K + 1] << 16) + (jaguarMainRAM[Adr68K + 2] << 8) + (jaguarMainRAM[Adr68K + 3])) || ((Adr68K < 0x4000) || (Adr68K >= 0x200000)))
					{
						sprintf(msg, "Memory allocator not yet initialised");
						Error = HA_MEMORYALLOCATORNOTINITIALIZED;
						Adr = 0;
					}
					else
					{
						return RefreshContents();
					}
				}
				else
				{
					sprintf(msg, "Memory allocator is not compatible");
					Error = HA_MEMORYALLOCATORNOTCOMPATIBLE;
					Adr = 0;
				}
			}
			else
			{
				sprintf(msg, "Memory allocator doesn't exist");
				Error = HA_MEMORYALLOCATORNOTEXIST;
			}
#ifdef HA_LAYOUTTEXTS
			HA += QString("");
#else
			model->setRowCount(0);
#endif
			MSG += QString(msg);
		}

		// Display status bar
		if (Error)
		{
			if ((Error & HA_WARNING))
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
		statusbar->showMessage(MSG);

#ifdef HA_LAYOUTTEXTS
		// Display values
		text->clear();
		text->setText(HA);
#endif
	}
}


// 
void HeapAllocatorBrowserWindow::Reset(void)
{
	size_t Adr68K;

	if (DBGManager_GetAdrFromSymbolName((char *)"__HeapBase"))
	{
		if (Adr68K = DBGManager_GetGlobalVariableAdrFromName((char *)"alloc"))
		{
			jaguarMainRAM[Adr68K] = jaguarMainRAM[Adr68K + 1] = jaguarMainRAM[Adr68K + 2] = jaguarMainRAM[Adr68K + 3] = 0;
			Adr = 0;
		}
	}
}


// 
void HeapAllocatorBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}

