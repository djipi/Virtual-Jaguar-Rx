//
// NewFnctBreakpointWin.h: New function breakpoint
//
// by Jean-Paul Mari
//

#ifndef __NEWFNCTBREAKPOINTWIN_H__
#define __NEWFNCTBREAKPOINTWIN_H__

#include <QtWidgets>
#include <stdint.h>

class NewFnctBreakpointWindow: public QWidget
{
	Q_OBJECT

	public:
		NewFnctBreakpointWindow(QWidget * parent = 0);

	public slots:

	protected:
		void keyPressEvent(QKeyEvent *);

	protected slots:
		void AddBreakpointAddress(void);

	private:
		QVBoxLayout *layout;
		QLineEdit *address;
		QPushButton *add;
};

#endif	// __NEWFNCTBREAKPOINTWIN_H__
