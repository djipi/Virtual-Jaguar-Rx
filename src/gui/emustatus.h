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
		void UpdateM68KCycles(size_t cycles);
		void RefreshContents(void);
		void ResetM68KCycles(void);

	private slots:
		void ResetCycles(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QPushButton * resetcycles;
		QLabel * text;
		bool GPURunning;
		bool M68000DebugHaltStatus;
		size_t M68K_opcodecycles;
		size_t M68K_totalcycles;
};

#endif	// __EMUSTATUS_H__
