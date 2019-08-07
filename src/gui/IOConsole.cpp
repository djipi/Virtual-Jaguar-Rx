//
// IOConsole.cpp - Console Input/Output
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  04/23/2019  Created this file
//

// STILL TO DO:
//

#include "debugger/DBGManager.h"
#include "IOConsole.h"


//
IOConsoleWindow::IOConsoleWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
text(new QLabel)
{
	setWindowTitle(tr("Console Input/Output"));

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
	text->setStyleSheet("QLabel { background-color : black; color : green; }");
	setLayout(layout);

	layout->addWidget(text);
}


//
IOConsoleWindow::~IOConsoleWindow(void)
{
}


// 
void IOConsoleWindow::Reset(void)
{
}


// Refresh / Display the window contents
void IOConsoleWindow::RefreshContents(void)
{
	QString fileDump;

	fileDump += QString("TESTs");

	text->clear();
	text->setText(fileDump);
}


//
void IOConsoleWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}

