//
// DasmWin.h: Jaguar disassembly window
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  06/26/2016  Created this file
//

#ifndef __DASMWIN_H__
#define __DASMWIN_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>

//class DasmWindow: public QMdiSubWindow
class DasmWindow : public QWidget
{
	Q_OBJECT

	public:
		//DasmWindow(QMdiArea * parent);
		DasmWindow(QWidget * parent);
		//DasmWindow(QWidget * parent = 0);
		//DasmWindow(QDockWidget * parent);

	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		void GoToAddress(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QDockWidget * layout1;
		//QTabWidget * layout1;
		QVBoxLayout * layout;
//		QTextBrowser * text;
		QLabel * text;
		QPushButton * refresh;
		QLineEdit * address;
		QPushButton * go;

		int32_t memBase;
};

#endif	// __DASMWIN_H__
