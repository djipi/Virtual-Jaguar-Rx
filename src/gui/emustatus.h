//
// emustatus.h: Jaguar emulator status
//
// by Jean-Paul Mari
// (C) 2012 Underground Software
//

#ifndef __EMUSTATUS_H__
#define __EMUSTATUS_H__

#include <QtWidgets>
#include <stdint.h>

class EmuStatusWindow : public QWidget
{
	Q_OBJECT

	public:
		EmuStatusWindow(QWidget * parent = 0);


	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		//void GoToAddress(void);

	protected:
		//void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
//		QTextBrowser * text;
		QLabel * text;
		//QPushButton * refresh;
		//QLineEdit * address;
		//QPushButton * go;

		bool	GPURunning;
		bool	M68000DebugHaltStatus;
};

#endif	// __EMUSTATUS_H__
