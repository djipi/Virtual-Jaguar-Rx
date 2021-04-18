//
// VideoWin.cpp: Video output window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  06/23/2016  Created this file
// JPM  April/2021  Added video output display in the window

#include "VideoWin.h"
#include "tom.h"
#include "settings.h"
//#include "mainwin.h"


// 
VideoOutputWindow::VideoOutputWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
hbox1(new QHBoxLayout)
//gl(new GLWidget)
//layout(0),
//hbox1(0)
//statusbar(new QStatusBar),
//text(new QTextBrowser)
{
	setWindowTitle(tr("Output Video"));
	//setCentralWidget(videoWidget);

//	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	//	QFont fixedFont("", 8, QFont::Normal);
//	fixedFont.setStyleHint(QFont::TypeWriter);
//	text->setFont(fixedFont);
//	setLayout(layout);
#if 0
	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(gl);
	layout->addLayout(hbox1);

	gl->setFixedSize(VIRTUAL_SCREEN_WIDTH, (vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL));
#endif
	//layout->addWidget(statusbar);
	//statusbar->setStyleSheet("background-color: lightgreen; font: bold");
	//layout->addWidget(text);
	//layout->addWidget(gl);

	//QWidget* myWidget = new QWidget;
	//myWidget->setStyleSheet("QWidget{ background: red; }");

	//GLWidget *glWidget = new GLWidget;
	//QHBoxLayout *mainLayout = new QHBoxLayout;
	//layout->addWidget(myWidget);
	//layout->setContentsMargins(100, 100, 100, 100);
	//setLayout(layout);

	//hbox1->addWidget(gl);
	//layout->addLayout(hbox1);
	//setLayout(layout);
}


//
void VideoOutputWindow::SetupVideo(GLWidget *Lt)
{
	if (isVisible())
	{
		//layout = new QVBoxLayout;
		//hbox1 = new QHBoxLayout;
		//	QHBoxLayout * hbox1 = new QHBoxLayout;
		//Lt->setFixedSize(VIRTUAL_SCREEN_WIDTH, (vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL));
		//QHBoxLayout * hbox1 = new QHBoxLayout;
		hbox1->addWidget(Lt);
		//hbox1->replaceWidget(gl, Lt);
		layout->addLayout(hbox1);
		setLayout(layout);
		//layout->addLayout(hbox1);
		//setLayout(layout);
		//layout->addLayout(hbox1);
		//setLayout(layout);
		//show();
		//resize(100, 100);
		//adjustSize();
		glViewport(0, 0, (VIRTUAL_SCREEN_WIDTH * 2), (vjs.hardwareTypeNTSC ? (VIRTUAL_SCREEN_HEIGHT_NTSC * 2) : (VIRTUAL_SCREEN_HEIGHT_PAL * 2)));
		//adjustSize();
		//resize(minimumWidth(), minimumHeight());
	}
}


// Refresh / Display the window contents
void VideoOutputWindow::RefreshContents(GLWidget *Lt)
{
#if 0
	if (isVisible())
	{
		for (uint32_t i = 0; i < (uint32_t)(Lt->textureWidth * Lt->rasterHeight); i++)
		{
			uint32_t pixel = Lt->buffer[i];
			uint8_t r = (pixel >> 24) & 0xFF, g = (pixel >> 16) & 0xFF, b = (pixel >> 8) & 0xFF;
			pixel = ((r + g + b) / 3) & 0x00FF;
			gl->buffer[i] = 0x000000FF | (pixel << 16) | (pixel << 8);
		}

		gl->updateGL();
		//adjustSize();
	}
#endif
#if 0
	if (isVisible())
	{

		QHBoxLayout * hbox1 = new QHBoxLayout;
		hbox1->addWidget(Lt);
		layout->addLayout(hbox1);
		setLayout(layout);

		//gl->setFixedSize(VIRTUAL_SCREEN_WIDTH, (vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL));

#if 0
		for (uint32_t y = 0; y < Lt->rasterHeight; y++)
		{
			if (vjs.hardwareTypeNTSC)
			{
				memcpy(gl->buffer + (y * gl->textureWidth), Lt + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
			}
			else
			{
				memcpy(gl->buffer + (y * gl->textureWidth), Lt + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
			}
		}
#endif
#if 0
		for (uint32_t i = 0; i < (uint32_t)(Lt->textureWidth * Lt->rasterHeight); i++)
		{
			uint32_t pixel = Lt->buffer[i];
			uint8_t r = (pixel >> 24) & 0xFF, g = (pixel >> 16) & 0xFF, b = (pixel >> 8) & 0xFF;
			pixel = ((r + g + b) / 3) & 0x00FF;
			gl->buffer[i] = 0x000000FF | (pixel << 16) | (pixel << 8);
		}
#else
		//gl->updateGL();

		//layout->addWidget(Lt);
		//setLayout(layout);
	//	QString memDump;

	//	memDump += QString("Test");

	//	text->clear();
	//	text->setText(memDump);

		//adjustSize();
#endif
	}
#endif
}


//
void VideoOutputWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}


// 
VideoOutputWindow::~VideoOutputWindow()
{
	//delete gl;
	delete hbox1;
	delete layout;
}
