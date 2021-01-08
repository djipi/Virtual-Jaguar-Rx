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

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class GPUDasmWindow: public QWidget
{
	Q_OBJECT

	public:
		GPUDasmWindow(QWidget * parent = 0);

	public slots:
		void RefreshContents(void);
		void	UseGPUPCAddress(void);

	protected:

	private:
		QVBoxLayout * layout;
		QLabel * text;
		int32_t memBase;
};

#endif	// __GPUDASMBROWSER_H__
