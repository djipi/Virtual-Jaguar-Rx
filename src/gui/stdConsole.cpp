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
// JPM  09/22/2024  Text output color detection
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
text(new QTextBrowser),
colorcommand(0),
colorindex(0)
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


// Reset the window with a message
void stdConsoleWindow::Reset(void)
{
	text->clear();
	stdoutDump += QString("\n\n**** Console Standard Emulation Reset ****\n\n");
	colorindex = 0;
	colorcommand = false;
	memset(colorcode, 0, sizeof(colorcode));
}


// Update / Display the window contents
void stdConsoleWindow::RefreshContents(void)
{
	// update the content from the Console standard emulation's stdout
	if (stdConsoleInfo[STDCONSOLE_STDOUT].BufText[0])
	{
		// loop om the content from the Console standard emulation's stdout
		size_t index = 0;
		while (stdConsoleInfo[STDCONSOLE_STDOUT].BufText[index])
		{
			// check output for color
			!colorcommand ? (colorcommand = (!strncmp(&stdConsoleInfo[STDCONSOLE_STDOUT].BufText[index], colorcommandid, 2) ? (index += 2) : false)) : false;		// \033[
			if (colorcommand)
			{
				// get color encoding
				char c = 0;
				while ((c != 'm') && (c = stdConsoleInfo[STDCONSOLE_STDOUT].BufText[index]) && index++)
				{
					(c != 'm') ? (colorcode[colorindex++] = c) : (colorcommand = false);	// x;yym
				}
			}
			else
			{
				// save the text to the window's string
				stdoutDump += stdConsoleInfo[STDCONSOLE_STDOUT].BufText[index++];
			}
		}
		// erase the content from the Console standard emulation's stdout
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

