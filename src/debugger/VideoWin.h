//
// VideoWin.h: Video output
//
// by Jean-Paul Mari
//
//

#ifndef __VIDEOWIN_H__
#define __VIDEOWIN_H__

#include <QtWidgets/QtWidgets>
#include <glwidget.h>

class VideoOutputWindow : public QWidget
{
	Q_OBJECT

	public:
		VideoOutputWindow(QWidget * parent = 0);
		~VideoOutputWindow();

	public slots:
	//		void DefineAllKeys(void);
		void RefreshContents(GLWidget *Lt);
		void SetupVideo(GLWidget *Lt);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
		QHBoxLayout *hbox1;
		//QTextBrowser * text;
		//QStatusBar *statusbar;
		GLWidget *gl;
};

#endif	// __VIDEOWIN_H__

