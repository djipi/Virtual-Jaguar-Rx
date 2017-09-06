//
// stackbrowser.h: Jaguar stack browser
//
// by Jean-Paul Mari
// (C) 2012 Underground Software
//

#ifndef __STACKBROWSER_H__
#define __STACKBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class StackBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		StackBrowserWindow(QWidget * parent = 0);


	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		//void GoToAddress(void);

	protected:
		//void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
//		QTextBrowser * text;
		QLabel * text;
		//QPushButton * refresh;
		//QLineEdit * address;
		//QPushButton * go;

		size_t memBase;
};

#endif	// __STACKBROWSER_H__
