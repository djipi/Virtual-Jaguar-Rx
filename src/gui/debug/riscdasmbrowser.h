//
// riscdasmbrowser.h: Jaguar RISC disassembly browser
//
// by James Hammons
// (C) 2013 Underground Software
//

#ifndef __RISCDASMBROWSER_H__
#define __RISCDASMBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class RISCDasmBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		RISCDasmBrowserWindow(QWidget * parent = 0);


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
		QPushButton * go;
		QLineEdit * address;
		QRadioButton * gpu;
		QRadioButton * dsp;

		int32_t memBase;
};

#endif	// __RISCDASMBROWSER_H__
