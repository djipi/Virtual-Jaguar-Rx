//
// memory1browser.h: Jaguar memory window 1 browser
//
// by James Hammons
// (C) 2012 Underground Software
//

#ifndef __MEMORY1BROWSER_H__
#define __MEMORY1BROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class Memory1BrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		Memory1BrowserWindow(QWidget * parent = 0);

	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(size_t NumWin);
		void RefreshContentsWindow(void);
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

		size_t memBase;
		size_t memOrigin;
		size_t NumWinOrigin;
};

#endif	// __MEMORY1BROWSER_H__
