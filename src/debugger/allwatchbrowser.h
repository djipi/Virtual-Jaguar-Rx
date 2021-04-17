//
// allwatchbrowser.h: All Watch
//
// by Jean-Paul Mari
//

#ifndef __ALLWATCHBROWSER_H__
#define __ALLWATCHBROWSER_H__

//#define AW_LAYOUTTEXTS						// Use a layout with just texts
//#define AW_SUPPORTARRAY						// Support array
//#define AW_SUPPORTSTRUCTURE					// Support structure

#include <QtWidgets/QtWidgets>
#include <stdint.h>

// Error code definitions
#define	AW_NOERROR		0x00
#define	AW_ERROR		0x80
#define	AW_WARNING		0x40
#define	AW_NOALLWATCH	(0x01 | AW_WARNING)


// 
class AllWatchBrowserWindow: public QWidget
{
	Q_OBJECT

	//
	typedef struct WatchInfo
	{
#ifdef AW_LAYOUTTEXTS
		size_t addr;
#endif
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

	protected slots:
		void SearchSymbol(void);
		void SelectSearchSymbol(void);

	private:
		QVBoxLayout *layout;
#ifdef AW_LAYOUTTEXTS
		QTextBrowser *text;
#else
		QTableView *TableView;
		QStandardItemModel *model;
#endif
		QStatusBar *statusbar;
		WatchInfo *PtrWatchInfo;
		size_t NbWatch;
		QPushButton *search;
		QLineEdit* symbol;
		size_t CurrentWatch;
};

#endif	// __ALLWATCHBROWSER_H__
