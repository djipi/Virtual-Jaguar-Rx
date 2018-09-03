//
// callstackbrowser.h: Call Stack
//
// by Jean-Paul Mari
//

#ifndef __CALLSTACKBROWSER_H__
#define __CALLSTACKBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class CallStackBrowserWindow : public QWidget
{
	Q_OBJECT

	public:
		CallStackBrowserWindow(QWidget *parent = 0);
		~CallStackBrowserWindow(void);

	public slots:
		void RefreshContents(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
		QTextBrowser * text;
};

#endif	// __CALLSTACKBROWSER_H__
