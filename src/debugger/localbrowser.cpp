//
// localbrowser.cpp - Local variables
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  11/03/2017  Created this file
// JPM  Sept./2018  Added a status bar and better status report, and set information values in a tab
//

// STILL TO DO:
// Feature to list the pointer(s) in the code using the allocation
// To set the information display at the right
// To support the array
// To support the static variables
// To add a filter
//

#include <stdlib.h>

#include "debugger/localbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"
#include "settings.h"
#include "m68000/m68kinterface.h"


// 
LocalBrowserWindow::LocalBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
#ifdef LOCAL_LAYOUTTEXTS
text(new QTextBrowser),
#else
TableView(new QTableView),
model(new QStandardItemModel),
#endif
NbLocal(0),
FuncName((char *)calloc(1, 1)),
LocalInfo(NULL),
statusbar(new QStatusBar)
{
	setWindowTitle(tr("Locals"));

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

#ifdef LOCAL_LAYOUTTEXTS
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
LocalBrowserWindow::~LocalBrowserWindow(void)
{
	free(LocalInfo);
	free(FuncName);
}


//
bool LocalBrowserWindow::UpdateInfos(void)
{
	size_t Adr;
	char *Ptr;

	if (NbLocal = DBGManager_GetNbLocalVariables(Adr = m68k_get_reg(NULL, M68K_REG_PC)))
	{
		if (Ptr = DBGManager_GetFunctionName(Adr))
		{
			if (strcmp(FuncName, Ptr))
			{
				if (FuncName = (char *)realloc(FuncName, strlen(Ptr) + 1))
				{
					strcpy(FuncName, Ptr);

					if (LocalInfo = (WatchInfo *)realloc(LocalInfo, (sizeof(WatchInfo) * NbLocal)))
					{
						for (size_t i = 0; i < NbLocal; i++)
						{
							// Get local variable name and his information
							if (LocalInfo[i].PtrVariableName = DBGManager_GetLocalVariableName(Adr, i + 1))
							{
								LocalInfo[i].Op = DBGManager_GetLocalVariableOp(Adr, i + 1);
								LocalInfo[i].Adr = NULL;
								LocalInfo[i].PtrCPURegisterName = NULL;
								LocalInfo[i].TypeTag = DBGManager_GetLocalVariableTypeTag(Adr, i + 1);
								LocalInfo[i].PtrVariableBaseTypeName = DBGManager_GetLocalVariableTypeName(Adr, i + 1);
								LocalInfo[i].TypeEncoding = DBGManager_GetLocalVariableTypeEncoding(Adr, i + 1);
								LocalInfo[i].TypeByteSize = DBGManager_GetLocalVariableTypeByteSize(Adr, i + 1);
								LocalInfo[i].Offset = DBGManager_GetLocalVariableOffset(Adr, i + 1);
							}
						}
					}
				}
			}

			return true;
		}
	}

	*FuncName = 0;

	return false;
}


//
void LocalBrowserWindow::RefreshContents(void)
{
#ifdef LOCAL_LAYOUTTEXTS
	char string[1024];
#endif
	size_t Error = LOCAL_NOERROR;
	QString Local;
	QString MSG;
	char Value1[100];
#ifdef LOCAL_SUPPORTARRAY
	char Value[100];
#endif
	char *PtrValue;

	const char *CPURegName[] = { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7" };

	if (isVisible())
	{
#ifndef LOCAL_LAYOUTTEXTS
		model->setRowCount(0);
#endif
		if (UpdateInfos())
		{
			for (size_t i = 0; i < NbLocal; i++)
			{
				if (LocalInfo[i].PtrVariableName)
				{
					memset(Value1, 0, sizeof(Value1));
#ifdef LOCAL_LAYOUTTEXTS
					if (i)
					{
						Local += QString("<br>");
					}
#else
					model->insertRow(i);
#endif
					// Local or parameters variables
					if (((LocalInfo[i].Op >= DBG_OP_breg0) && (LocalInfo[i].Op <= DBG_OP_breg31)) || (LocalInfo[i].Op == DBG_OP_fbreg))
					{
						LocalInfo[i].Adr = m68k_get_reg(NULL, M68K_REG_A6) + LocalInfo[i].Offset;

						if ((LocalInfo[i].Op == DBG_OP_fbreg))
						{
							LocalInfo[i].Adr += 8;
						}

						if ((LocalInfo[i].Adr >= 0) && (LocalInfo[i].Adr < vjs.DRAM_size))
						{
							if ((LocalInfo[i].TypeTag & (DBG_TAG_TYPE_array | DBG_TAG_TYPE_structure)))
							{
#if defined(LOCAL_SUPPORTARRAY) || defined(LOCAL_SUPPORTSTRUCTURE)
								//memcpy(Value1, &jaguarMainRAM[LocalInfo[i].Adr], 20);
#ifdef LOCAL_LAYOUTTEXTS
								//sprintf(Value, "\"%s\"", Value1);
#else
								//sprintf(Value, "0x%06X, \"%s\"", LocalInfo[i].Adr, Value1);
#endif
								//PtrValue = Value;
								PtrValue = NULL;
#else
								PtrValue = NULL;
#endif
							}
							else
							{
								PtrValue = DBGManager_GetVariableValueFromAdr(LocalInfo[i].Adr, LocalInfo[i].TypeEncoding, LocalInfo[i].TypeByteSize);
							}
						}
						else
						{
							PtrValue = NULL;
						}
					}
					else
					{
						// Value from CPU register
						if ((LocalInfo[i].Op >= DBG_OP_reg0) && (LocalInfo[i].Op <= DBG_OP_reg31))
						{
							LocalInfo[i].PtrCPURegisterName = (char *)CPURegName[(LocalInfo[i].Op - DBG_OP_reg0)];
							sprintf(Value1, "%d", m68k_get_reg(NULL, (m68k_register_t)((size_t)M68K_REG_D0 + (LocalInfo[i].Op - DBG_OP_reg0))));
							PtrValue = Value1;
						}
						else
						{
							PtrValue = NULL;
						}
					}

#ifndef LOCAL_LAYOUTTEXTS
					model->setItem(i, 0, new QStandardItem(QString("%1").arg(LocalInfo[i].PtrVariableName)));
#endif
					// Check if the local variable is use by the code
					if (!LocalInfo[i].Op)
					{
#ifdef LOCAL_LAYOUTTEXTS
						sprintf(string, "<font color='#A52A2A'>%i : %s | %s | [Not used]</font>", (i + 1), (LocalInfo[i].PtrVariableBaseTypeName ? LocalInfo[i].PtrVariableBaseTypeName : (char *)"<font color='#ff0000'>N/A</font>"), LocalInfo[i].PtrVariableName);
#else
#endif
					}
					else
					{
#ifndef LOCAL_LAYOUTTEXTS
						model->setItem(i, 1, new QStandardItem(QString("%1").arg(PtrValue)));
#else
						sprintf(string, "%i : %s | %s | ", (i + 1), (LocalInfo[i].PtrVariableBaseTypeName ? LocalInfo[i].PtrVariableBaseTypeName : (char *)"<font color='#ff0000'>N/A</font>"), LocalInfo[i].PtrVariableName);
						Local += QString(string);

						if ((unsigned int)LocalInfo[i].Adr)
						{
							sprintf(string, "0x%06X", (unsigned int)LocalInfo[i].Adr);
						}
						else
						{
							if (LocalInfo[i].PtrCPURegisterName)
							{
								sprintf(string, "<font color='#0000FF'>%s</font>", LocalInfo[i].PtrCPURegisterName);
							}
							else
							{
								sprintf(string, "%s", (char *)"<font color='#ff0000'>N/A</font>");
							}
						}

						Local += QString(string);
						sprintf(string, " | %s", (!PtrValue ? (char *)"<font color='#ff0000'>N/A</font>" : PtrValue));
#endif
					}
#ifndef LOCAL_LAYOUTTEXTS
					model->setItem(i, 2, new QStandardItem(QString("%1").arg((LocalInfo[i].PtrVariableBaseTypeName ? LocalInfo[i].PtrVariableBaseTypeName : (char *)"<font color='#ff0000'>N/A</font>"))));
#else
					Local += QString(string);
#endif
				}
			}

			MSG += QString("Ready");
#ifdef LOCAL_LAYOUTTEXTS
			text->clear();
			text->setText(Local);
#endif
		}
		else
		{
			// No locals
			MSG += QString("No locals");
			Error = LOCAL_NOLOCALS;
#ifdef LOCAL_LAYOUTTEXTS
			text->clear();
#endif
		}

		// Display status bar
		if (Error)
		{
			if ((Error & LOCAL_WARNING))
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
	}
}


// 
void LocalBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
