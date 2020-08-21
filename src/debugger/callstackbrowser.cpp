//
// callstackbrowser.cpp - Call Stack
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (M/D/Y)  What
// ---  ------------  -----------------------------------------------------------
// JPM  08/31/2018    Created this file
// JPM  09/12/2018    Added a status bar and better status report
// JPM  10/20/2018    Added the return address information in the call stack
// JPM  08/09/2019    Prevent crash in case of call stack is out of range
// JPM  03/16/2020    Modified the layout window and added source filename from the called source line

// STILL TO DO:
// To set the information display at the right
// To use DWARF frame information?
// To check if call stack pointer is used (DWARF information?)
//

#include "debugger/callstackbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"
#include "settings.h"


// 
CallStackBrowserWindow::CallStackBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
#ifdef CS_LAYOUTTEXTS
text(new QTextBrowser),
#else
TableView(new QTableView),
model(new QStandardItemModel),
#endif
statusbar(new QStatusBar),
layout(new QVBoxLayout)
{
	setWindowTitle(tr("Call Stack"));

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

#ifdef CS_LAYOUTTEXTS
	// Set original layout
	text->setFont(fixedFont);
	layout->addWidget(text);
#else
	// Set the new layout with proper identation and readibility
	model->setColumnCount(4);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Function"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Line"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Return address"));
	model->setHeaderData(3, Qt::Horizontal, QObject::tr("Filename"));
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
CallStackBrowserWindow::~CallStackBrowserWindow(void)
{
}


// 
void CallStackBrowserWindow::RefreshContents(void)
{
	char msg[1024];
	size_t Error = CS_NOERROR;
	DBGstatus FilenameStatus;
	unsigned int a6, Sa6, ret;
	char *Name;
	size_t NumError = 0;
#ifdef CS_LAYOUTTEXTS
	QString CallStack;
	char string[1024];
#else
	int NbRaw = 0;
	QString FunctionName;
#endif

	if (isVisible())
	{
#ifndef CS_LAYOUTTEXTS
		model->setRowCount(0);
#endif
		if ((a6 = m68k_get_reg(NULL, M68K_REG_A6)) && DBGManager_GetType())
		{
			while ((Sa6 = a6) && !NumError)
			{
				if ((Sa6 >= (m68k_get_reg(NULL, M68K_REG_SP) - 4)) && (Sa6 < vjs.DRAM_size))
				{
					a6 = GET32(jaguarMainRAM, Sa6);
					ret = GET32(jaguarMainRAM, Sa6 + 4);
#ifdef CS_LAYOUTTEXTS
					sprintf(string, "0x%06X | Ret: 0x%06X | From: %s - 0x%06X | Line: %s", Sa6, ret, (Name = DBGManager_GetFunctionName(ret)), (unsigned int)DBGManager_GetAdrFromSymbolName(Name), DBGManager_GetLineSrcFromAdr(ret, DBG_NO_TAG));
					CallStack += QString(string);
					if (a6)
					{
						CallStack += QString("<br>");
					}
#else
					// insert line
					model->insertRow(NbRaw);
					// display the function name
					model->setItem(NbRaw, 0, new QStandardItem(QString("%1").arg((Name = DBGManager_GetFunctionName(ret)) ? Name : "(N/A)")));
					// display the called line
					FunctionName = QString(Name = DBGManager_GetLineSrcFromAdr(ret, DBG_NO_TAG));
					//FunctionName.replace("&nbsp;", " ");
					FunctionName = FunctionName.trimmed();
					model->setItem(NbRaw, 1, new QStandardItem(QString("%1").arg(Name ? FunctionName : "(N/A)")));
					// display the return address
					sprintf(msg, "0x%06X", ret);
					model->setItem(NbRaw, 2, new QStandardItem(QString("%1").arg(msg)));
					// display the source filename from called source line
					model->setItem(NbRaw++, 3, new QStandardItem(QString("%1").arg(((Name = DBGManager_GetFullSourceFilenameFromAdr(ret, &FilenameStatus)) && !FilenameStatus) ? Name : "(N/A)")));
#endif
				}
				else
				{
					NumError = 0x1;
				}
			}
#ifdef CS_LAYOUTTEXTS
			text->clear();
			text->setText(CallStack);
#endif
			switch (NumError)
			{
			case 0:
				sprintf(msg, "Ready");
				Error = CS_NOERROR;
				break;

			case 0x1:
				sprintf(msg, "Call Stack out of range");
				Error = CS_ERROR;
				break;

			default:
				sprintf(msg, "Call Stack in limbo");
				Error = CS_WARNING;
				break;
			}
		}
		else
		{
			sprintf(msg, "Call Stack not available");
			Error = CS_NOCALLSTACK;
#ifdef CS_LAYOUTTEXTS
			text->clear();
#endif
		}

		// Display status bar
		if (Error)
		{
			if ((Error & CS_WARNING))
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
void CallStackBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}


