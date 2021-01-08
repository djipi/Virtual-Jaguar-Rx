//
// heapallocatorbrowser.h: Memory heap allocation
//
// by Jean-Paul Mari
//

#ifndef __HEAPALLOCATORBROWSER_H__
#define __HEAPALLOCATORBROWSER_H__

//#define HA_LAYOUTTEXTS						// Use a layout with just texts

#include <QtWidgets/QtWidgets>
#include <stdint.h>

// Error code definitions
#define	HA_NOERROR							0x00
#define	HA_ERROR							0x80
#define	HA_WARNING							0x40
#define	HA_UNABLENEXTMEMORYALLOC			(0x01 | HA_ERROR)
#define HA_UNABLEALLOCATEMEMORYUSAGE		(0x02 | HA_ERROR)
#define HA_MEMORYBLOCKSIZEPROBLEM			(0x03 | HA_ERROR)
#define HA_MEMORYALLOCATIONPROBLEM			(0x04 | HA_ERROR)
#define HA_HAANDSPSHARESPACE				(0x05 | HA_ERROR)
#define	HA_MEMORYALLOCATORNOTEXIST			(0x06 | HA_WARNING)
#define	HA_MEMORYALLOCATORNOTCOMPATIBLE		(0x07 | HA_WARNING)
#define	HA_MEMORYALLOCATORNOTINITIALIZED	(0x08 | HA_WARNING)


// 
class HeapAllocatorBrowserWindow: public QWidget
{
	Q_OBJECT

	typedef struct HeapAllocation
	{
		uint32_t nextalloc;
		uint32_t size;
		uint16_t used;
	}
	S_HeapAllocation;

	public:
		HeapAllocatorBrowserWindow(QWidget *parent = 0);
		~HeapAllocatorBrowserWindow(void);

	public slots:
		void RefreshContents(void);
		void Reset(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
#ifdef HA_LAYOUTTEXTS
		QTextBrowser *text;
#else
		QTableView *TableView;
		QStandardItemModel *model;
		QSortFilterProxyModel *proxyModel;
#endif
		QStatusBar *statusbar;
		size_t Adr;
};

#endif	// __HEAPALLOCATORBROWSER_H__
