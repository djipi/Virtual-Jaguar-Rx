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

	protected slots:
		void stateChangedStyleSheetColor(int index);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout * layout;
		QTextBrowser * text;
		QString stdoutDump;
		bool colorcommand;
		char colorcode[30] = { 0 };
		char colorcommandid[3] = { 0x1b, 0x5b, 0 };			// \033[
		size_t colorindex;

	public:
		QCheckBox *StyleSheetColor;
};

#endif // __STDCONSOLE_H__
