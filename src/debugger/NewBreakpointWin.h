//
// NewBreakpointWin.h: New breakpoint
//
// by Jean-Paul Mari
//

#ifndef __NEWBREAKPOINTWIN_H__
#define __NEWBREAKPOINTWIN_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>
#include "debugger/BreakpointsWin.h"

class NewBreakpointWindow: public QWidget
{
	enum BreakpointType
	{
		BP_CODE,
		BP_DATA
	};

	enum BreakpointAccess
	{
		BP_READ = 0x1,
		BP_WRITE = 0x2,
		BP_READWRITE = (BP_READ | BP_WRITE)
	};

	Q_OBJECT

	public:
		NewBreakpointWindow(QWidget * parent = 0);
		void SetBreakpointWin(BreakpointsWindow* BpW);
		~NewBreakpointWindow();

	public slots:

	protected:
		void keyPressEvent(QKeyEvent *);

	protected slots:
		void AddBreakpointAddress(void);
		void SelectBreakpointAddress(void);
		void TypeChanged(int index);

	private:
		QVBoxLayout *layout;
		QLineEdit *address;
		QPushButton *add;
		BreakpointsWindow* BPWin;
		QComboBox* type;
		QLabel* accessLabel;
		QComboBox* access;
};

#endif
