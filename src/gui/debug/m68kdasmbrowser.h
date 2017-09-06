//
// m68kdasmbrowser.h: Jaguar 68K disassembly browser
//
// by James Hammons
// (C) 2012 Underground Software
//

#ifndef __M68KDASMBROWSER_H__
#define __M68KDASMBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class M68KDasmBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		M68KDasmBrowserWindow(QWidget * parent = 0);


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

#endif	// __M68KDASMBROWSER_H__
