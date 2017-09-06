//
// help.h: Built-in help system
//
// by James Hammons
// (C) 2011 Underground Software
//

#ifndef __HELP_H__
#define __HELP_H__

#include <QtWidgets>

class HelpWindow: public QWidget
{
	public:
		HelpWindow(QWidget * parent = 0);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QTextBrowser * text;
};

#endif	// __HELP_H__
