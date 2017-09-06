//
// about.h: Credits where credits are due ;-)
//
// by James Hammons
// (C) 2010 Underground Software
//

#ifndef __ABOUT_H__
#define __ABOUT_H__

#include <QtWidgets>

class AboutWindow: public QWidget
{
	public:
		AboutWindow(QWidget * parent = 0);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QLabel * text;
		QLabel * image;
};

#endif	// __ABOUT_H__
