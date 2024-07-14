//
// stdConsole.h: Console standard emulation
//
// by Jean-Paul Mari
//

#ifndef __STDCONSOLE_H__
#define __STDCONSOLE_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class stdConsoleWindow : public QWidget
{
	Q_OBJECT

	public:
		stdConsoleWindow(QWidget * parent = 0);
		~stdConsoleWindow(void);
		void RefreshContents(void);
		void Reset(void);

	public slots:

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QTextBrowser * text;
		QString stdoutDump;
};

#endif // __STDCONSOLE_H__
