//
// stdConsole.cpp - Console standard emulation
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (m/d/y)  What
// ---  ------------  -----------------------------------------------------------
// JPM  07/11/2024    Created this file
// JPM  09/22/2024    Text output color detection, amber style sheet color mode
// JPM  10/07/2024    Fix output color encoding
// JPM  10/25/2025    Clear window content
// JPM  01/04/2026    Smoother bar scrolling
//

// STILL TO DO:
// Support for the stdin, stderr
//

#include "stdConsole.h"
#include "jaguar.h"
#include "debugger/DBGManager.h"
#include <QtWidgets/QScrollBar>


// Constructor
stdConsoleWindow::stdConsoleWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
controlLayout(new QHBoxLayout),
text(new QTextBrowser),
StyleSheetColor(new QCheckBox("Amber")),
ClearButton(new QPushButton("Clear")),
colorcommand(0),
colorindex(0)
{
	// window initialization
	setWindowTitle(tr("Console standard emulation"));
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	text->setAlignment(Qt::AlignLeft | Qt::AlignBottom);
	//text->setMinimumWidth(40);
	//text->setStyleSheet("QTextBrowser { background-color : black; color : orange; }");

	// set button minimum width to display text properly
	ClearButton->setMinimumWidth(60);

	// set layout
	layout->addWidget(text);
	controlLayout->addWidget(StyleSheetColor);
	controlLayout->addWidget(ClearButton);
	controlLayout->addStretch();
	layout->addLayout(controlLayout);
	setLayout(layout);

	// connections
	connect(StyleSheetColor, SIGNAL(stateChanged(int)), this, SLOT(stateChangedStyleSheetColor(int)));
	connect(ClearButton, SIGNAL(clicked()), this, SLOT(handleClearButton()));
}


// Destructor
stdConsoleWindow::~stdConsoleWindow(void)
{
}


// Handle style sheet color change
void stdConsoleWindow::stateChangedStyleSheetColor(int check)
{
	// save setting
	if (check)
	{
		// amber color
		text->setStyleSheet("QTextBrowser { background-color : black; color : orange; }");
	}
	else
	{
		// default color
		text->setStyleSheet("");
	}
}


// Clear the window contents with a message
void stdConsoleWindow::handleClearButton(void)
{
	text->clear();
	stdoutDump.clear();
	stdoutDump += QString("\n\n**** Console Standard Emulation ****\n\n");
	colorindex = 0;
	colorcommand = false;
	memset(colorcode, 0, sizeof(colorcode));
	RefreshContents();
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
	QString str = "";

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
					(c != 'm') ? (colorcode[colorindex++] = c) : (colorcommand = false);
				}
				// use color encoding in normal mode
				if ((c == 'm') && !StyleSheetColor->checkState())
				{
				}
				colorindex = 0;
			}
			else
			{
				// save the text to the window's string
				char c = stdConsoleInfo[STDCONSOLE_STDOUT].BufText[index++];
				stdoutDump += c;
				str += c;
			}
		}
		// erase the content from the Console standard emulation's stdout
		memset(stdConsoleInfo[STDCONSOLE_STDOUT].BufText, 0, sizeof(stdConsoleInfo[STDCONSOLE_STDOUT].BufText));
	}

	// manage scroll bar position
	QScrollBar * sb = text->verticalScrollBar();
	bool atBottom = (sb->value() == sb->maximum());
	int savedValue = sb->value();

	// first time fill and move to the end
	if (text->document()->isEmpty() && !stdoutDump.isEmpty())
	{
		text->setText(stdoutDump);
		text->moveCursor(QTextCursor::End);
	}
	else
	{
		// move to the end and append new text
		if (!str.isEmpty())
		{
			text->moveCursor(QTextCursor::End);
			text->insertPlainText(str);
		}
	}

	// restore scroll bar position
	if (isVisible())
	{
		if (atBottom)
		{
			sb->setValue(sb->maximum());
		}
		else
		{
			sb->setValue(savedValue);
		}
	}
}


// Handle key press event
void stdConsoleWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
