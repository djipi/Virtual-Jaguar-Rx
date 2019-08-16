//
// BreakpointsWin.h: Breakpoints
//
// by Jean-Paul Mari
//

#ifndef __BREAKPOINTSWIN_H__
#define __BREAKPOINTSWIN_H__

//#define BRK_STATUSBAR				// Status bar usage
//#define BRK_REFRESHBUTTON				// Refresh button
#define BRK_HITCOUNTS				// Support the hit count

#include <QtWidgets>
#include <stdint.h>


class BreakpointsWindow: public QWidget
{
	Q_OBJECT

	public:
		BreakpointsWindow(QWidget *parent = 0);
		~BreakpointsWindow(void);
		void Reset(void);
		void RefreshContents(void);

	public slots:

	protected:
		void keyPressEvent(QKeyEvent *);
		void UpdateInfos(void);
		void UpdateTable(bool refresh);

	private:
		QVBoxLayout *layout;
#ifdef BRK_STATUSBAR
		QStatusBar *statusbar;
#endif
		QTableView *TableView;
		QStandardItemModel *model;
		QPushButton *refresh;
};

#endif	// __BREAKPOINTSWIN_H__
