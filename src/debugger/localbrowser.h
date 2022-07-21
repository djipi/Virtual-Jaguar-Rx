//
// localbrowser.h: Local variables
//
// by Jean-Paul Mari
//

#ifndef __LOCALBROWSER_H__
#define __LOCALBROWSER_H__

//#define LOCAL_FONTS								// Support for fonts modifications

#include <QtWidgets/QtWidgets>
#include <stdint.h>

// UI positions
#define LOCAL_UINAME		0
#define LOCAL_UIVALUE		1
#define LOCAL_UITYPE		2
//#define LOCAL_UIA6OFFSET	3
#define LOCAL_NBUI			3		// Number of UI's column to display

// Error code definitions
#define	LOCAL_NOERROR		0x00
#define	LOCAL_WARNING		0x40
#define	LOCAL_ERROR			0x80
#define	LOCAL_NOLOCALS		(0x01 | LOCAL_WARNING)

// 
class LocalBrowserWindow: public QWidget
{
	Q_OBJECT

	//
	typedef struct LocalInfo
	{
		size_t Adr;
		char *PtrCPURegisterName;
		void *PtrVariable;
	}
	S_LocalInfo;

	public:
		LocalBrowserWindow(QWidget *parent = 0);
		~LocalBrowserWindow(void);

	public slots:
		void RefreshContents(void);
		bool UpdateInfos(void);

	protected:
		QList<QStandardItem *> prepareRow(void* Info);
		void setValueRow(QStandardItem *Row, size_t Adr, char* Value, void* Info);
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
		QTreeView *TableView;
		QStandardItemModel *model;
		S_LocalInfo *LocalInfo;
		QStatusBar *statusbar;
		size_t NbLocal;
		char *FuncName;
		size_t ExRegA6;
};

#endif	// __LOCALBROWSER_H__
