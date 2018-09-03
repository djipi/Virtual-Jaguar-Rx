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
//

// STILL TO DO:
// Better display presentation
// To display call function
// To use DWARF frame information
//

#include "debugger/callstackbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"


//
CallStackBrowserWindow::CallStackBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QTextBrowser)
{
	setWindowTitle(tr("Call Stack"));

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	setLayout(layout);

	layout->addWidget(text);
}


//
CallStackBrowserWindow::~CallStackBrowserWindow(void)
{
}


//
void CallStackBrowserWindow::RefreshContents(void)
{
	unsigned int a6, Sa6, ret;
	char *FuncName;
	QString CallStack;
	char string[1024];

	if (isVisible())
	{
		if ((a6 = m68k_get_reg(NULL, M68K_REG_A6)) && DBGManager_GetType())
		{
			while ((Sa6 = a6))
			{
				a6 = GET32(jaguarMainRAM, Sa6);
				ret = GET32(jaguarMainRAM, Sa6 + 4);

				sprintf(string, "0x%06X | Ret: 0x%06X | From: %s - 0x%06X", Sa6, ret, (FuncName = DBGManager_GetFunctionName(ret)), (unsigned int)DBGManager_GetAdrFromSymbolName(FuncName));
				CallStack += QString(string);
				if (a6)
				{
					sprintf(string, "<br>");
					CallStack += QString(string);
				}
			}

			text->clear();
			text->setText(CallStack);
		}
		else
		{
			text->clear();
		}
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


