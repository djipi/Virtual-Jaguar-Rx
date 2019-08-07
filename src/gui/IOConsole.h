//
// IOConsole.h: Console Input/Output
//
// by Jean-Paul Mari
//

#ifndef __IOCONSOLE_H__
#define __IOCONSOLE_H__

#include <QtWidgets>
#include <stdint.h>

class IOConsoleWindow : public QWidget
{
	Q_OBJECT

	public:
		IOConsoleWindow(QWidget * parent = 0);
		~IOConsoleWindow(void);
		void RefreshContents(void);
		void Reset(void);

	public slots:

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QLabel * text;
};

#endif // __IOCONSOLE_H__
