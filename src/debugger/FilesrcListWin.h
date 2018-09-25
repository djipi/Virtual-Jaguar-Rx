//
// FilesrcListWin.h: List all source code filenames
//
// by Jean-Paul Mari
//

#ifndef __FILESRCLISTWIN_H__
#define __FILESRCLISTWIN_H__

#include <QtWidgets>
#include <stdint.h>

// Error code definitions
#define	FSL_NOERROR			0x00
#define	FSL_ERROR			0x80
#define	FSL_WARNING			0x40
#define	FSL_NOFILESRCLIST	(0x01 | FSL_WARNING)


// 
class FilesrcListWindow: public QWidget
{
	Q_OBJECT

	typedef struct _filesrcitem
	{
		size_t column;
		QStandardItem *PreviousItem;
		QStandardItem *Item;
	}Sfilesrcitem;

	public:
		FilesrcListWindow(QWidget *parent = 0);
		~FilesrcListWindow(void);
		void RefreshContents(void);
		void Reset(void);

	public slots:

	protected:
		void *AddItem(char *ItemName, size_t ItemPos);
		void AddFilename(char *FileName, QStandardItem *root, size_t ItemPos);
		size_t UpdateInfos(void);

	private:
		size_t nbItem;
		QVBoxLayout *layout;
		QTreeView *treeView;
		QStandardItemModel *standardModel;
		QStandardItem *rootNode;
		Sfilesrcitem *filesrcItems;
		QStatusBar *statusbar;
};

#endif	// __FILESRCLISTWIN_H__
