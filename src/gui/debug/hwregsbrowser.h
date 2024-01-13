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
#include "hwregsjerrybrowser.h"


typedef enum { hwregsnull = 0x0, hwregsro = 0x1, hwregswo = 0x2, hwregsr = 0x4, hwregsw = 0x8, hwregsrw = (hwregsr | hwregsw) } hwregsaccess;


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
	QVBoxLayout *layout;
	QTabWidget *hwregstabWidget;
	HWRegsBlitterBrowserWindow *hwregsblitterWin;
	HWRegsJerryBrowserWindow *hwregsjerryWin;
};


#endif
