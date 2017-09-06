//
// help.cpp - Built-in help file
//
// by James Hammons
// (C) 2011 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  08/01/2011  Created this file
// JLH  10/08/2011  Added Esc & Return as exit keys
//

// STILL TO DO:
//

#include "help.h"


HelpWindow::HelpWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog)
{
	setWindowTitle(tr("Virtual Jaguar Help"));

	// Need to set the size as well...
	resize(560, 480);

	layout = new QVBoxLayout();
//	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	text = new QTextBrowser;
	text->setSource(QUrl("qrc:/res/help.html"));
	layout->addWidget(text);
}

void HelpWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Return)
		hide();
}
