//
// interuptbrowser.h: Interrupt browser
//

#ifndef __INTERUPTBROWSER_H__
#define __INTERUPTBROWSER_H__

#include <QtWidgets/QtWidgets>

class InteruptBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		InteruptBrowserWindow(QWidget * parent = 0);
		~InteruptBrowserWindow();

	public slots:
		void RefreshContents(void);

	private:
		QCheckBox *jerry[2][4];

	public:
};

#endif	// __INTERUPTBROWSER_H__
