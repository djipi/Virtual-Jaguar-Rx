//
// exceptionvectortablebrowser.h: Exception Vector Table
//
// by Jean-Paul Mari
//

#ifndef __EXCEPTIONVECTORTABLEBROWSER_H__
#define __EXCEPTIONVECTORTABLEBROWSER_H__

//#define EV_LAYOUTTEXTS						// Use a layout with just texts

#include <QtWidgets/QtWidgets>
#include <stdint.h>


// 
class ExceptionVectorTableBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		ExceptionVectorTableBrowserWindow(QWidget *parent = 0);
		~ExceptionVectorTableBrowserWindow(void);

	public slots:
		void RefreshContents(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
#ifdef EV_LAYOUTTEXTS
		QTextBrowser *text;
#else
		QTableView *TableView;
		QStandardItemModel *model;
#endif
		QPushButton *refresh;
};

#endif	// __EXCEPTIONVECTORTABLEBROWSER_H__
