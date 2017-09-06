//
// allwatch.h: All Watch
//
// by James Hammons
// (C) 2012 Underground Software
//

#ifndef __ALLWATCHBROWSER_H__
#define __ALLWATCHBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class AllWatchBrowserWindow: public QWidget
{
	Q_OBJECT

	//
	struct WatchInfo
	{
		//size_t TypeEncoding;
		//size_t TypeByteSize;
		size_t addr;
		size_t TypeTag;
		char *PtrVariableName;
		char *PtrVariableBaseTypeName;
	}S_WatchInfo;

	public:
		AllWatchBrowserWindow(QWidget *parent = 0);
		~AllWatchBrowserWindow(void);

	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
//		void GoToAddress(void);

	protected:
//		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
//		QTextBrowser * text;
//		QLabel *text;
		QTextBrowser *text;
//		QPushButton *refresh;
//		QLineEdit *address;
//		QPushButton *go;
		WatchInfo *PtrWatchInfo;
//		int32_t memBase;
		size_t NbWatch;
};

#endif	// __ALLWATCHBROWSER_H__
