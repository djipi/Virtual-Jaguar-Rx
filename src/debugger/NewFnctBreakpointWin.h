//
// NewFnctBreakpointWin.h: New function breakpoint
//
// by Jean-Paul Mari
//

#ifndef __NEWFNCTBREAKPOINTWIN_H__
#define __NEWFNCTBREAKPOINTWIN_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>
#include "debugger/BreakpointsWin.h"

class NewFnctBreakpointWindow: public QWidget
{
	Q_OBJECT

	public:
		NewFnctBreakpointWindow(QWidget * parent = 0);
		void SetFnctBreakpointWin(BreakpointsWindow* BpW);
		~NewFnctBreakpointWindow();

	public slots:

	protected:
		void keyPressEvent(QKeyEvent *);

	protected slots:
		void AddBreakpointAddress(void);
		void SelectBreakpointAddress(void);

	private:
		QVBoxLayout *layout;
		QLineEdit *address;
		QPushButton *add;
		BreakpointsWindow* BPWin;
};

#endif	// __NEWFNCTBREAKPOINTWIN_H__
