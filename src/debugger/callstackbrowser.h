//
// callstackbrowser.h: Call Stack
//
// by Jean-Paul Mari
//

#ifndef __CALLSTACKBROWSER_H__
#define __CALLSTACKBROWSER_H__

//#define CS_LAYOUTTEXTS						// Use a layout with just texts

#include <QtWidgets>
#include <stdint.h>

// Error code definitions
#define	CS_NOERROR		0x00
#define	CS_ERROR		0x80
#define	CS_WARNING		0x40
#define	CS_NOCALLSTACK	(0x01 | CS_WARNING)


// 
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
		QStatusBar *statusbar;
#ifdef CS_LAYOUTTEXTS
		QTextBrowser * text;
#else
		QTableView *TableView;
		QStandardItemModel *model;
#endif
};

#endif	// __CALLSTACKBROWSER_H__
