//
// DSPDasmWin.h: Jaguar DSP disassembly window
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  02/02/2017  Created this file
//

#ifndef __DSPDASMBROWSER_H__
#define __DSPDASMBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class DSPDasmWindow: public QWidget
{
	Q_OBJECT

	public:
		DSPDasmWindow(QWidget * parent = 0);


	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		//void GoToAddress(void);
		void	UseDSPPCAddress(void);
		//void	SetAddress(int address);

	protected:
		//void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
//		QTextBrowser * text;
		QLabel * text;
		//QPushButton * refresh;
		//QPushButton * go;
		//QLineEdit * address;
		//QRadioButton * gpu;
		//QRadioButton * dsp;

		int32_t memBase;
};

#endif	// __DSPDASMBROWSER_H__
