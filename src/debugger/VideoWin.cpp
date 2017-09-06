//
// VideoWin.cpp: Windows video display
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  06/23/2016  Created this file

#include "VideoWin.h"
//#include "mainwin.h"


// 
VideoOutputWindow::VideoOutputWindow(QWidget * parent/*= 0*/) : QWidget(parent),
layout(new QHBoxLayout), text(new QLabel)
{
	setWindowTitle(tr("Video windows"));
	//setCentralWidget(videoWidget);

//	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	//	QFont fixedFont("", 8, QFont::Normal);
//	fixedFont.setStyleHint(QFont::TypeWriter);
//	text->setFont(fixedFont);
//	setLayout(layout);

//	layout->addWidget(text);

	QWidget* myWidget = new QWidget;
	myWidget->setStyleSheet("QWidget{ background: red; }");

	GLWidget *glWidget = new GLWidget;
	//QHBoxLayout *mainLayout = new QHBoxLayout;
	layout->addWidget(myWidget);
	layout->setContentsMargins(100, 100, 100, 100);
	setLayout(layout);
}


// Refresh / Display the window contents
void VideoOutputWindow::RefreshContents(GLWidget *Lt)
{
	layout->addWidget(Lt);
	setLayout(layout);
//	QString memDump;

//	memDump += QString("Test");

//	text->clear();
//	text->setText(memDump);
}


// 
VideoOutputWindow::~VideoOutputWindow()
{
}
