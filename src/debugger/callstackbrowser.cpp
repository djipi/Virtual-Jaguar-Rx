//
// callstackbrowser.cpp - Call Stack
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  08/31/2018  Created this file
// JPM  09/12/2018  Added a status bar and better status report
//

// STILL TO DO:
// To set the information display at the right
// To use DWARF frame information
//

#include "debugger/callstackbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"


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
	model->setColumnCount(2);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Line"));
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
	unsigned int a6, Sa6, ret;
	char *FuncName;
#ifdef CS_LAYOUTTEXTS
	QString CallStack;
	char string[1024];
#else
	size_t NbRaw = 0;
	QString FunctionName;
#endif

	if (isVisible())
	{
#ifndef CS_LAYOUTTEXTS
		model->setRowCount(0);
#endif
		if ((a6 = m68k_get_reg(NULL, M68K_REG_A6)) && DBGManager_GetType())
		{
			while ((Sa6 = a6))
			{
				a6 = GET32(jaguarMainRAM, Sa6);
				ret = GET32(jaguarMainRAM, Sa6 + 4);
#ifdef CS_LAYOUTTEXTS
				sprintf(string, "0x%06X | Ret: 0x%06X | From: %s - 0x%06X | Line: %s", Sa6, ret, (FuncName = DBGManager_GetFunctionName(ret)), (unsigned int)DBGManager_GetAdrFromSymbolName(FuncName), DBGManager_GetLineSrcFromAdr(ret, DBG_NO_TAG));
				CallStack += QString(string);
				if (a6)
				{
					CallStack += QString("<br>");
				}
#else
				model->insertRow(NbRaw);
				model->setItem(NbRaw, 0, new QStandardItem(QString("%1").arg((FuncName = DBGManager_GetFunctionName(ret)) ? FuncName : "(null)")));
				FunctionName = QString(FuncName = DBGManager_GetLineSrcFromAdr(ret, DBG_NO_TAG));
				FunctionName.replace("&nbsp;", " ");
				model->setItem(NbRaw++, 1, new QStandardItem(QString("%1").arg(FuncName ? FunctionName : "(null)")));
#endif
			}
#ifdef CS_LAYOUTTEXTS
			text->clear();
			text->setText(CallStack);
#endif
			sprintf(msg, "Ready");
			Error = CS_NOERROR;
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


