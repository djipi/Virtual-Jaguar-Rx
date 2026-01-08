//
// ctrlprofilerwin.h: Profiler control window for Virtual Jaguar Rx
//
// Features:
// - 
//
// Note: This header is for all profilers.
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM   Nov./2025       Created this file
//

#ifndef __CTRLPROFILERWIN_H__
#define __CTRLPROFILERWIN_H__

#include <QtWidgets/QtWidgets>

//
class CtrlProfilerWindow : public QWidget
{
	Q_OBJECT

public:
	CtrlProfilerWindow(QWidget* parent = 0);
	//void SetFnctFrameWin(void);
	~CtrlProfilerWindow();

protected:
	void keyPressEvent(QKeyEvent*);

private:
	QVBoxLayout* layout;
	QGroupBox* loopframebox;
	QLineEdit* loopframeaddress;
	QPushButton* loopframeadd;

private slots:
	void AddLoopFrameAddress(void);
	void SelectLoopFrameAddress(void);
};

#endif // __CTRLPROFILERWIN_H__
