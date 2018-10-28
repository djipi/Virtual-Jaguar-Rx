//
// CartFilesListWin.h: List files from directory
//
// by Jean-Paul Mari
//

#ifndef __CARTFILESLISTWIN_H__
#define __CARTFILESLISTWIN_H__

#include <QtWidgets>
#include <stdint.h>

//
//#define CFL_BUFFERTREAM				// Display the buffer stream

// Error code definitions
#define	CFL_NOERROR				0x00
#define	CFL_ERROR				0x80
#define	CFL_WARNING				0x40
#define	CFL_NOFILESLIST			(0x01 | CFL_WARNING)
#define	CFL_NODIRECTORYLIST		(0x02 | CFL_WARNING)
#define CFL_NODIRUSE			(0x04 | CFL_WARNING)

// Cart directory type
#define CFL_NOTYPE		0x00
#define CFL_OSJAGTYPE	0x01


// 
class CartFilesListWindow: public QWidget
{
	Q_OBJECT

	typedef struct _fileitem
	{
		size_t column;
		QStandardItem *PreviousItem;
		QStandardItem *Item;
	}Sfileitem;

	typedef struct S_CARTDIRINFO
	{
		bool valid;
		char *PtrFilename;
		size_t SizeFile;
		size_t PtrDataFile;
		size_t CurrentSeek;
		size_t PtrBufferStream;
	}
	CARTDIRINFO;

	typedef struct S_OSJAGDir
	{
		long PtrDataFile;
		long SizeFile;
		char Filename[];
	}
	OSJAGDir;

	public:
		CartFilesListWindow(QWidget *parent = 0);
		~CartFilesListWindow(void);
		void RefreshContents(void);
		void Reset(void);

	private:
		void *AddItem(char *ItemName, size_t ItemPos);
		void AddFilename(char *FileName, QStandardItem *root, size_t ItemPos);
		void UpdateInfos(void);
		void *CreateInfos(void);
		size_t GetDirType(void);
		size_t GetNbrFiles(void);
		void GetFileInfos(CARTDIRINFO *Ptr, size_t index);

	protected:
		void keyPressEvent(QKeyEvent * e);

	private:
		size_t nbItem, CartDirType, CartNbrFiles, CartUsedBytes;
		QVBoxLayout *layout;
		QTreeView *treeView;
		QStandardItemModel *standardModel;
		QStandardItem *rootNode;
		CARTDIRINFO *CartDirectory;
		QStatusBar *statusbar;
		Sfileitem *fileItems;
		QStandardItemModel *model;
		QTableView *TableView;
};

#endif	// __CARTFILESLISTWIN_H__
