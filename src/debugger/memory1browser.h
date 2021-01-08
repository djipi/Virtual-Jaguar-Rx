//
// memory1browser.h: Jaguar memory window 1 browser
//
// by Jean-Paul Mari
//

#ifndef __MEMORY1BROWSER_H__
#define __MEMORY1BROWSER_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class Memory1BrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		Memory1BrowserWindow(QWidget * parent = 0);

	public slots:
		void RefreshContents(size_t NumWin);
		void RefreshContentsWindow(void);
		void GoToAddress(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QLabel * text;
		QPushButton * refresh;
		QLineEdit * address;
		QPushButton * go;
		int memBase;
		size_t memOrigin;
		size_t NumWinOrigin;
};

#endif	// __MEMORY1BROWSER_H__
