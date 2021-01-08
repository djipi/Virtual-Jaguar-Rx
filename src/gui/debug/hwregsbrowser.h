//
// hwregsbrowser.h: Hardware registers browser
//
// by Jean-Paul Mari
//

#ifndef __HWREGSBROWSER_H__
#define __HWREGSBROWSER_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>
#include "hwregsblitterbrowser.h"


// 
class HWRegsBrowserWindow : public QWidget
{
	Q_OBJECT

public:
	HWRegsBrowserWindow(QWidget *parent = 0);
	~HWRegsBrowserWindow(void);
	void Reset(void);

public slots:
	void RefreshContents(void);

protected:
	void keyPressEvent(QKeyEvent *);

private:
	//QStatusBar *statusbar;
	QVBoxLayout *layout;
	QTabWidget *hwregstabWidget;
	HWRegsBlitterBrowserWindow *hwregsblitterWin;
};


#endif
