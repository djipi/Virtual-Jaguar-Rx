//
// memorybrowser.h: Jaguar memory browser
//
// by James Hammons
// (C) 2012 Underground Software
//

#ifndef __MEMORYBROWSER_H__
#define __MEMORYBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class MemoryBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		MemoryBrowserWindow(QWidget * parent = 0);


	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		void GoToAddress(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
//		QTextBrowser * text;
		QLabel * text;
		QPushButton * refresh;
		QLineEdit * address;
		QPushButton * go;

		int32_t memBase;
};

#endif	// __MEMORYBROWSER_H__
