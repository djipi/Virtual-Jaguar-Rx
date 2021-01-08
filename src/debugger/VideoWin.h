//
// VideoWin.h: Credits where credits are due ;-)
//
// based on the work from James Hammons
// (C) 2010 Underground Software
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

	protected:
		//void keyPressEvent(QKeyEvent *);

	private:
		QHBoxLayout *layout;
		QLabel * text;
};

#endif	// __VIDEOWIN_H__

