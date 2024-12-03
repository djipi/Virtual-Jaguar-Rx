//
// heapallocatorbrowser.cpp: Memory heap allocation
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (MM/DD/YY)  What
// ---  ---------------  ----------------------------------------------------------
// JPM  01/08/2017       Created this file
// JPM  Sept./2018       Support of the DRAM size limit option, use definitions for error instead of hard values
//                       Detect if heap allocation shares space with SP (Stack), added a status bar and better status report
//                       Set information values in a tab
// JPM  07/04/2019       Fix the support of the DRAM size limit option
// JPM  11/10/2024       Support Calypsi's malloc structure for free nodes, revamp of the error/warning detections
// JPM   Dec./2024       Add VBcc's malloc structure 
//

// STILL TO DO:
// To have filters
// To use a common structure to handle each malloc's library
// To set the information display at the right
// Feature to list the pointer(s) in the code using the allocation
// Support the gcc's malloc structure
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
Adr(0),
CodeMalloc(HA_MALLOC_NONE)
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
	int NbBlocks, TotalBytesUsed;
	HeapAllocation HeapAllocation;
	S__heap_s* PtrCalypsiHeap;
	S_Vclib_memblock PtrVclibHeap;

	// display only if window is visible
	if (isVisible())
	{
		if (Adr68K = Adr)
		{
			Adr68KHigh = TotalBytesUsed = NbBlocks = 0;
#ifndef HA_LAYOUTTEXTS
			model->setRowCount(0);
#endif
			switch (CodeMalloc)
			{
				// Calypsi library malloc allocation
			case HA_MALLOC_CALYPSI:
				PtrCalypsiHeap = (S__heap_s*)&jaguarMainRAM[Adr68K];
				for (S___freenode_s* node = (S___freenode_s*)&jaguarMainRAM[(Adr68K = BigToLittleEndian32(PtrCalypsiHeap->nodelist[1].flink))]; Adr68K && !Error; node = (S___freenode_s*)&jaguarMainRAM[(Adr68K = BigToLittleEndian32(node->flink))])
				{
					if ((Adr68K >= 0x4000) && (Adr68K < vjs.DRAM_size))
					{
						if (Adr68K < m68k_get_reg(NULL, M68K_REG_SP))
						{
							model->insertRow(NbBlocks);
							model->setItem(NbBlocks, 0, new QStandardItem(QString("0x%1").arg(Adr68K, 6, 16, QChar('0'))));
							model->setItem(NbBlocks, 1, new QStandardItem(QString("%1").arg(BigToLittleEndian32(node->size))));
							model->setItem(NbBlocks++, 2, new QStandardItem(QString("%1").arg("Free")));
						}
						else
						{
							Error = HA_HAANDSPSHARESPACE;
						}
					}
					else
					{
						Error = HA_MEMORYALLOCATIONPROBLEM;
					}
				}

				Error ? sprintf(msg, "") : sprintf(msg, "Size: $%06X | Start: $%06X | End: $%06X", BigToLittleEndian32(PtrCalypsiHeap->heapsize), BigToLittleEndian32(PtrCalypsiHeap->heapstart), BigToLittleEndian32(PtrCalypsiHeap->heapend));
				break;

				// VBcc library malloc allocation 
			case HA_MALLOC_VCLIB:
				do
				{
					if ((Adr68K >= 0x4000) && (Adr68K < vjs.DRAM_size))
					{
					if (Adr68K < m68k_get_reg(NULL, M68K_REG_SP))
					{
						memcpy(&PtrVclibHeap, &jaguarMainRAM[Adr68K], sizeof(PtrVclibHeap));
						if (PtrVclibHeap.size = BigToLittleEndian32(PtrVclibHeap.size))
						{
							if (PtrVclibHeap.size <= (vjs.DRAM_size - 0x4000))
							{
								if (((PtrVclibHeap.used = BigToLittleEndian32(PtrVclibHeap.used)) == 1) || !PtrVclibHeap.used)
								{
									PtrVclibHeap.next = BigToLittleEndian32(PtrVclibHeap.next);
									if ((PtrVclibHeap.next >= 0x4000) && (PtrVclibHeap.next < vjs.DRAM_size))
									{
										model->insertRow(NbBlocks);
										model->setItem(NbBlocks, 0, new QStandardItem(QString("0x%1").arg(Adr68K, 6, 16, QChar('0'))));
										model->setItem(NbBlocks, 1, new QStandardItem(QString("%1").arg((PtrVclibHeap.size))));
										model->setItem(NbBlocks++, 2, new QStandardItem(QString("%1").arg(PtrVclibHeap.used ? "Allocated" : "Free")));
									
										TotalBytesUsed += PtrVclibHeap.size;

										if ((Adr68K = PtrVclibHeap.next) > Adr68KHigh)
										{
											Adr68KHigh = Adr68K;
										}
									}
									else
									{
										Error = HA_UNABLENEXTMEMORYALLOC;
									}
								}
								else
								{
									Error = HA_UNABLEALLOCATEMEMORYUSAGE;
								}
							}
							else
							{
								Error = HA_MEMORYBLOCKSIZEPROBLEM;
							}
						}
						else
						{
							sprintf(msg, "%i blocks | %i bytes in blocks | %zi contiguous bytes free", NbBlocks, TotalBytesUsed, (m68k_get_reg(NULL, M68K_REG_SP) - Adr68KHigh));
						}
					}
					else
					{
						Error = HA_HAANDSPSHARESPACE;
					}
				}
					else
					{
						Error = HA_MEMORYALLOCATIONPROBLEM;
					}
				}while (PtrVclibHeap.size && !Error);
				break;

				// LIBM68K malloc allocation
			case HA_MALLOC_LIBM68K:
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
											Error = HA_UNABLENEXTMEMORYALLOC;
										}
									}
									else
									{
										Error = HA_UNABLEALLOCATEMEMORYUSAGE;
									}
								}
								else
								{
									Error = HA_MEMORYBLOCKSIZEPROBLEM;
								}
							}
							else
							{
								sprintf(msg, "%i blocks | %i bytes in blocks | %zi contiguous bytes free", NbBlocks, TotalBytesUsed, (m68k_get_reg(NULL, M68K_REG_SP) - Adr68KHigh));
							}
						}
						else
						{
							Error = HA_HAANDSPSHARESPACE;
						}
					}
					else
					{
						Error = HA_MEMORYALLOCATIONPROBLEM;
					}
				} while (HeapAllocation.size && !Error);
				break;

				// no memory allocator found
			default:
				break;
			}
		}
		else
		{
			// look for the malloc's name
			CodeMalloc = HA_MALLOC_NONE;
			while (MallocNames[CodeMalloc] && !(Adr = DBGManager_GetAdrFromSymbolName((char*)MallocNames[CodeMalloc])) && ++CodeMalloc);
			switch (CodeMalloc)
			{
				// Calypsi library malloc allocation
			case HA_MALLOC_CALYPSI:
				if (!(Adr68K = GET32(jaguarMainRAM, (Adr + 4))) || ((Adr68K < 0x4000) || (Adr68K >= vjs.DRAM_size)))
				{
					Error = HA_MEMORYALLOCATORNOTINITIALIZED;
					Adr = 0;
				}
				else
				{
					return RefreshContents();
				}
				break;

				// VBcc library malloc allocation 
			case HA_MALLOC_VCLIB:
				if (!(Adr68K = Adr) || ((Adr68K < 0x4000) || (Adr68K >= vjs.DRAM_size)))
				{
					Error = HA_MEMORYALLOCATORNOTINITIALIZED;
					Adr = 0;
				}
				else
				{
					return RefreshContents();
				}
				break;

				// LIBM68K malloc allocation
			case HA_MALLOC_LIBM68K:
				if (Adr68K = DBGManager_GetGlobalVariableAdrFromName((char *)"alloc"))
				{
					if (!(Adr68K = GET32(jaguarMainRAM, Adr68K)) || ((Adr68K < 0x4000) || (Adr68K >= vjs.DRAM_size)))
					{
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
					Error = HA_MEMORYALLOCATORNOTCOMPATIBLE;
					Adr = 0;
				}
				break;

				// no memory allocator found
			default:
				Error = HA_MEMORYALLOCATORNOTEXIST;
				break;
			}

#ifdef HA_LAYOUTTEXTS
			HA += QString("");
#else
			model->setRowCount(0);
#endif
		}

		// display status bar
		if (Error)
		{
			// set error text
			switch (Error)
			{
			case HA_UNABLENEXTMEMORYALLOC:
				sprintf(msg, "Unable to determine the next memory allocation");
				break;

			case HA_UNABLEALLOCATEMEMORYUSAGE:
				sprintf(msg, "Unable to determine if the allocated memory is used or not");
				break;

			case HA_MEMORYBLOCKSIZEPROBLEM:
				sprintf(msg, "Memory bloc size has a problem");
				break;

			case HA_HAANDSPSHARESPACE:
				sprintf(msg, "Memory allocations and Stack have reached the same space");
				break;

			case HA_MEMORYALLOCATIONPROBLEM:
				sprintf(msg, "Memory allocations may have a problem");
				break;

			case HA_MEMORYALLOCATORNOTINITIALIZED:
				sprintf(msg, "Memory allocator not yet initialised");
				break;

			case HA_MEMORYALLOCATORNOTCOMPATIBLE:
				sprintf(msg, "Memory allocator is not compatible");
				break;

			case HA_MEMORYALLOCATORNOTEXIST:
				sprintf(msg, "Memory allocator doesn't exist");
				break;

			default:
				break;
			}

			// set message warning/error color
			if ((Error & HA_WARNING))
			{
				// warning
				statusbar->setStyleSheet("background-color: lightyellow; font: bold");
			}
			else
			{
				// error
				statusbar->setStyleSheet("background-color: tomato; font: bold");
			}
		}
		else
		{
			// no error
			statusbar->setStyleSheet("background-color: lightgreen; font: bold");
		}
		MSG += QString(msg);
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
	switch (CodeMalloc)
	{
		// Calypsi library malloc allocation
	case HA_MALLOC_CALYPSI:
		Adr = 0;
		break;

		// VBcc library malloc allocation 
	case HA_MALLOC_VCLIB:
		Adr = 0;
		break;

		// LIBM68K malloc allocation
	case HA_MALLOC_LIBM68K:
		if (size_t Adr68K = DBGManager_GetGlobalVariableAdrFromName((char *)"alloc"))
		{
			SET32(jaguarMainRAM, Adr68K, 0);
			Adr = 0;
		}
		break;

	default:
		break;
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

