//
// allwatchbrowser.h: All Watch
//
// by Jean-Paul Mari
//

#ifndef __ALLWATCHBROWSER_H__
#define __ALLWATCHBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class AllWatchBrowserWindow: public QWidget
{
	Q_OBJECT

	//
	typedef struct WatchInfo
	{
		size_t addr;
		size_t TypeTag;
		char *PtrVariableName;
		char *PtrVariableBaseTypeName;
	}S_WatchInfo;

	public:
		AllWatchBrowserWindow(QWidget *parent = 0);
		~AllWatchBrowserWindow(void);
		void Reset(void);

	public slots:
		void RefreshContents(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
		QTextBrowser *text;
		WatchInfo *PtrWatchInfo;
		size_t NbWatch;
};

#endif	// __ALLWATCHBROWSER_H__
