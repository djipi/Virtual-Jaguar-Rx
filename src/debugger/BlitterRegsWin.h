//
// BlitterRegsWin.h: Display the Blitter's regs values
//
// by Jean-Paul Mari
//

#ifndef __BLITTERREGSWIN_H__
#define __BLITTERREGSWIN_H__

//#define BLITTEREGS_FONTS								// Support for fonts modifications

#include <QtWidgets/QtWidgets>
#include <stdint.h>


// 
class BlitterRegsWindow : public QWidget
{
	Q_OBJECT

public:
	BlitterRegsWindow(QWidget *parent = 0);
	~BlitterRegsWindow(void);

public slots:
	//void RefreshContents(void);
	//bool UpdateInfos(void);

protected:
	void keyPressEvent(QKeyEvent *);

private:
	QVBoxLayout *vaLayout;
	QHBoxLayout *a1Layout;
	QHBoxLayout *a2Layout;
	QVBoxLayout *mainLayout;
	QStatusBar *statusbar;
	QFrame *a1Frame;
	QLabel *A1Adr;
};

#endif	// __BLITTERREGSWIN_H__
