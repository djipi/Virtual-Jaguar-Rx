//
// localbrowser.cpp - Local variables
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  RG = Richard Goedeken
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  11/03/2017  Created this file
// JPM  Sept./2018  Added a status bar and better status report, and set information values in a tab
//  RG   Jan./2021  Linux build fixes
// JPM    May/2021  Display the structure's members
//

// STILL TO DO:
// Feature to list the pointer(s) in the code using the allocation
// To set the information display at the right
// To support the array
// To support the union
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
TableView(new QTreeView),
model(new QStandardItemModel),
NbLocal(0),
FuncName(NULL),
LocalInfo(NULL),
statusbar(new QStatusBar)
{
	setWindowTitle(tr("Locals"));
#ifdef LOCAL_FONTS
	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
#endif
	// Set the new layout with proper identation and readibility
	model->setColumnCount(3);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Value"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Type"));
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
#ifdef LOCAL_FONTS
	TableView->setFont(fixedFont);
#endif
	layout->addWidget(TableView);

	// Status bar
	layout->addWidget(statusbar);
	setLayout(layout);
}


//
LocalBrowserWindow::~LocalBrowserWindow(void)
{
	free(LocalInfo);
}


// Get the local variables information
// Return true for a new local variables set
bool LocalBrowserWindow::UpdateInfos(void)
{
	size_t Adr;
	char *Ptr;

	// get number of local variables located in the M68K PC address
	if ((NbLocal = DBGManager_GetNbVariables(Adr = m68k_get_reg(NULL, M68K_REG_PC))))
	{
		// get function name from the M68K PC address
		if ((Ptr = DBGManager_GetFunctionName(Adr)))
		{
			// avoid to read the same information in case of the function has not changed
			if (!FuncName || strcmp(FuncName, Ptr))
			{
				// function is different
				FuncName = Ptr;
				if (LocalInfo = (S_LocalInfo*)realloc(LocalInfo, (sizeof(S_LocalInfo) * NbLocal)))
				{
					for (size_t i = 0; i < NbLocal; i++)
					{
						// get local variable name and his information
						if ((LocalInfo[i].PtrVariable = DBGManager_GetInfosVariable(Adr, i + 1)))
						{
							LocalInfo[i].Adr = 0;
							LocalInfo[i].PtrCPURegisterName = NULL;
						}
					}
				}

				return true;
			}
			else
			{
				return false;
			}
		}
	}

	FuncName = NULL;

	return false;
}


// Prepare a complete row based on the variable information
// The row will append rows if necessary
QList<QStandardItem *> LocalBrowserWindow::prepareRow(void* Info)
{
	// set the list
	QList<QStandardItem *> ptrRow = { new QStandardItem(((S_VariablesStruct*)Info)->PtrName), new QStandardItem(""), new QStandardItem(((S_VariablesStruct*)Info)->PtrTypeName) };

	// check if variable has additional variables (such as structure)
	if (size_t nb = ((S_VariablesStruct*)Info)->NbTabVariables)
	{
		for (size_t i = 0; i < nb; i++)
		{
			ptrRow.first()->appendRow(prepareRow(((S_VariablesStruct*)Info)->TabVariables[i]));
		}
	}

	// return the list
	return ptrRow;
}


// Set the values of each line in accordance of the rows created from prepareRow() function
void LocalBrowserWindow::setValueRow(QStandardItem *Row, size_t Adr, char* Value, void* Info)
{
	QStandardItem *child = Row->child(0, 1);
	if (child)
	{
		// check if variable has additional variables list (such as structure)
		if (size_t nb = ((S_VariablesStruct*)Info)->NbTabVariables)
		{
			// get the pointer's value
			Adr = GET32(jagMemSpace, Adr);

			for (size_t i = 0; i < nb; i++)
			{
				// do not display arrays
				if (!((((S_VariablesStruct*)Info)->TabVariables[i]->TypeTag & DBG_TAG_TYPE_array)))
				{
					// set value in the row
					Value = DBGManager_GetVariableValueFromAdr(Adr + ((S_VariablesStruct*)Info)->TabVariables[i]->Offset, ((S_VariablesStruct*)Info)->TabVariables[i]->TypeEncoding, ((S_VariablesStruct*)Info)->TabVariables[i]->TypeByteSize);
					child = Row->child((int)i, 1);
					child->setText(QString("%1").arg(Value));
					setValueRow(child, Adr + ((S_VariablesStruct*)Info)->TabVariables[i]->Offset, Value, (void*)((S_VariablesStruct*)Info)->TabVariables[i]);
				}
			}
		}
	}
}


// Refresh the local and/or parameters contents list
void LocalBrowserWindow::RefreshContents(void)
{
	size_t Error = LOCAL_NOERROR;
	QString Local;
	QString MSG;
	char Value1[100];
	char *PtrValue;

	// local variables may come from M68K registers
	const char *CPURegName[] = { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7" };

	// refresh only if local's window is displayed
	if (isVisible())
	{
		// get local's information
		if (UpdateInfos())
		{
			// erase the previous variables list
			model->setRowCount(0);

			// loop on the locals found
			for (size_t i = 0; i < NbLocal; i++)
			{
				// check variable's name validity
				if (((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->PtrName)
				{
					// insert the row
					model->appendRow(prepareRow(LocalInfo[i].PtrVariable));

					// check if the local variable is use by the code
					if (((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op)
					{
						// local or parameters variables
						if (((((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op >= DBG_OP_breg0) && (((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op <= DBG_OP_breg31)) || (((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op == DBG_OP_fbreg))
						{
							// get variable's address
							LocalInfo[i].Adr = m68k_get_reg(NULL, M68K_REG_A6) + ((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Offset;
							// check variable's parameter op
							if ((((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op == DBG_OP_fbreg))
							{
								LocalInfo[i].Adr += 8;
							}
						}
						else
						{
							// variable type from CPU register
							if ((((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op >= DBG_OP_reg0) && (((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op <= DBG_OP_reg31))
							{
								// set color text for a register type variable
								model->item((int)i, 0)->setForeground(QColor(0, 0, 0xfe));
								model->item((int)i, 1)->setForeground(QColor(0, 0, 0xfe));
								model->item((int)i, 2)->setForeground(QColor(0, 0, 0xfe));
								// get the register's name
								LocalInfo[i].PtrCPURegisterName = (char *)CPURegName[(((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op - DBG_OP_reg0)];
							}
						}
					}
					else
					{
						// set color text for unused variable (no need to set it for the value's field as no values will be displayed)
						model->item((int)i, 0)->setForeground(QColor(0xc8, 0xc8, 0xc8));
						model->item((int)i, 2)->setForeground(QColor(0xc8, 0xc8, 0xc8));
					}
				}
			}
		}

		// set the values in the fields
		if (NbLocal)
		{
			// loop on the locals found
			for (size_t i = 0; i < NbLocal; i++)
			{
				// variable's address must fit in RAM
				if ((LocalInfo[i].Adr >= 4) && (LocalInfo[i].Adr < vjs.DRAM_size))
				{
					// get the variable's value
					PtrValue = DBGManager_GetVariableValueFromAdr(LocalInfo[i].Adr, ((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->TypeEncoding, ((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->TypeByteSize);
				}
				else
				{
					// check CPU register's type variable
					if (LocalInfo[i].PtrCPURegisterName)
					{
						// get the value from register
						memset(Value1, 0, sizeof(Value1));
						sprintf(Value1, "0x%x", m68k_get_reg(NULL, (m68k_register_t)((size_t)M68K_REG_D0 + (((S_VariablesStruct*)(LocalInfo[i].PtrVariable))->Op - DBG_OP_reg0))));
						PtrValue = Value1;
					}
					else
					{
						// no value can be found
						PtrValue = NULL;
					}
				}

				// do not display arrays
				if (!(((S_VariablesStruct*)LocalInfo[i].PtrVariable)->TypeTag & DBG_TAG_TYPE_array))
				{
					// set the local's variable value
					model->item((int)i, 1)->setText(QString("%1").arg(PtrValue));
					setValueRow(model->item((int)i), LocalInfo[i].Adr, PtrValue, (S_VariablesStruct*)(LocalInfo[i].PtrVariable));
				}
			}

			MSG += QString("Ready");
		}
		else
		{
			// erase the previous variables list
			model->setRowCount(0);

			// No locals
			MSG += QString("No locals");
			Error = LOCAL_NOLOCALS;
		}

		// display status bar
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


// Handle keyboard actions
void LocalBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	// close / hide the window
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
