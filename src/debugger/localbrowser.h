//
// localbrowser.h: Local variables
//
// by Jean-Paul Mari
//

#ifndef __LOCALBROWSER_H__
#define __LOCALBROWSER_H__

//#define LOCAL_LAYOUTTEXTS						// Use a layout with just texts
//#define LOCAL_SUPPORTARRAY						// Support array
//#define LOCAL_SUPPORTSTRUCTURE					// Support structure

#include <QtWidgets>
#include <stdint.h>

// Error code definitions
#define	LOCAL_NOERROR		0x00
#define	LOCAL_ERROR			0x80
#define	LOCAL_WARNING		0x40
#define	LOCAL_NOLOCALS		(0x01 | LOCAL_WARNING)


// 
class LocalBrowserWindow: public QWidget
{
	Q_OBJECT

	//
	typedef struct WatchInfo
	{
		size_t Op;
		size_t Adr;
		int Offset;
		size_t TypeTag;
		size_t TypeEncoding;
		size_t TypeByteSize;
		char *PtrVariableName;
		char *PtrVariableBaseTypeName;
		char *PtrCPURegisterName;
	}
	S_WatchInfo;

	public:
		LocalBrowserWindow(QWidget *parent = 0);
		~LocalBrowserWindow(void);

	public slots:
		void RefreshContents(void);
		bool UpdateInfos(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
#ifdef LOCAL_LAYOUTTEXTS
		QTextBrowser *text;
#else
		QTableView *TableView;
		QStandardItemModel *model;
#endif
		WatchInfo *LocalInfo;
		QStatusBar *statusbar;
		size_t NbLocal;
		char *FuncName;
};

#endif	// __LOCALBROWSER_H__
