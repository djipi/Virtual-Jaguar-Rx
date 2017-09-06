//
// GPUdasmWin.h: Jaguar GPU disassembly window
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  02/01/2017  Created this file
//

#ifndef __GPUDASMBROWSER_H__
#define __GPUDASMBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class GPUDasmWindow: public QWidget
{
	Q_OBJECT

	public:
		GPUDasmWindow(QWidget * parent = 0);


	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		//void GoToAddress(void);
		void	UseGPUPCAddress(void);
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

#endif	// __GPUDASMBROWSER_H__
