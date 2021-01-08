//
// emustatus.h: Jaguar emulator status
//
// by Jean-Paul Mari
//

#ifndef __EMUSTATUS_H__
#define __EMUSTATUS_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class EmuStatusWindow : public QWidget
{
	Q_OBJECT

	public:
		EmuStatusWindow(QWidget * parent = 0);

	public slots:
		void RefreshContents(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QLabel * text;
		bool	GPURunning;
		bool	M68000DebugHaltStatus;
};

#endif	// __EMUSTATUS_H__
