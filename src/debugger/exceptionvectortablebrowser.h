//
// exceptionvectortablebrowser.h: Exception Vector Table
//
// by Jean-Paul Mari
//

#ifndef __EXCEPTIONVECTORTABLEBROWSER_H__
#define __EXCEPTIONVECTORTABLEBROWSER_H__

#include <QtWidgets>
#include <stdint.h>


class ExceptionVectorTableBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		ExceptionVectorTableBrowserWindow(QWidget *parent = 0);
		~ExceptionVectorTableBrowserWindow(void);

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
		QPushButton *refresh;
//		QLineEdit *address;
//		QPushButton *go;
//		WatchInfo *PtrWatchInfo;
//		int32_t memBase;
//		size_t NbWatch;
};

#endif	// __EXCEPTIONVECTORTABLEBROWSER_H__
