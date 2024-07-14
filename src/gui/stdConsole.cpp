//
// stdConsole.cpp - Console standard emulation
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  07/11/2024  Created this file
//

// STILL TO DO:
// Support for the stdin, stderr
//

#include "debugger/DBGManager.h"
#include "stdConsole.h"
#include "jaguar.h"


//
stdConsoleWindow::stdConsoleWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
text(new QTextBrowser)
{
	// window initialisation
	setWindowTitle(tr("Console standard emulation"));
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	text->setStyleSheet("QTextBrowser { background-color : black; color : orange; }");
	text->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	//text->setMinimumWidth(40);

	// set layout
	layout->addWidget(text);
	setLayout(layout);
}


//
stdConsoleWindow::~stdConsoleWindow(void)
{
}


// 
void stdConsoleWindow::Reset(void)
{
	text->clear();
	stdoutDump += QString("\n\n**** Console Standard Emulation Reset ****\n\n");
}


// Update / Display the window contents
void stdConsoleWindow::RefreshContents(void)
{
	// update the content from the Console standard emulation's stdout
	if (stdConsoleInfo[STDCONSOLE_STDOUT].BufText[0])
	{
		stdoutDump += QString(stdConsoleInfo[STDCONSOLE_STDOUT].BufText);
		memset(stdConsoleInfo[STDCONSOLE_STDOUT].BufText, 0, sizeof(stdConsoleInfo[STDCONSOLE_STDOUT].BufText));
	}

	// display content
	if (isVisible())
	{
		text->clear();
		text->setText(stdoutDump);
	}
}


//
void stdConsoleWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}

