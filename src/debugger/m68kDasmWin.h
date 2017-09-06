//
// m68kDasmWin.h: M68K disassembly window
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  06/27/2016  Created this file
//

#ifndef __M68KDASMWIN_H__
#define __M68KDASMWIN_H__

#include <QtWidgets>
#include <stdint.h>

class m68KDasmWindow: public QWidget
{
	Q_OBJECT

	public:
		m68KDasmWindow(QWidget * parent = 0);

	public slots:
//		void	DefineAllKeys(void);
		void	RefreshContents(void);
#if 0
		void	GoToAddress(void);
#endif
		void	SetAddress(int address);
		void	Use68KPCAddress(void);

	protected:
#if 0
		void	keyPressEvent(QKeyEvent *);
#endif

	private:
		QVBoxLayout * layout;
		QTextBrowser * text;
//		QLabel * text;
#if 0
		QPushButton * refresh;
		QLineEdit * address;
		QPushButton * go;
#endif
		size_t memBase;
};

#endif	// __M68KDASMWIN_H__
