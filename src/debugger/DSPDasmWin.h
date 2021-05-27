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

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class DSPDasmWindow: public QWidget
{
	Q_OBJECT

	public:
		DSPDasmWindow(QWidget * parent = 0);

	public slots:
		void RefreshContents(void);
		void	UseDSPPCAddress(void);

	protected:

	private:
		QVBoxLayout * layout;
		QLabel * text;
		int32_t memBase;
};

#endif	// __DSPDASMBROWSER_H__
