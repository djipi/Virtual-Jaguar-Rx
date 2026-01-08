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

// Error & Warning code definitions
#define	HA_NOERROR							0x00
#define	HA_ERROR							0x80
#define	HA_WARNING							0x40
#define	HA_UNABLENEXTMEMORYALLOC			(0x01 | HA_ERROR)
#define HA_UNABLEALLOCATEMEMORYUSAGE		(0x02 | HA_ERROR)
#define HA_MEMORYBLOCKSIZEPROBLEM			(0x03 | HA_ERROR)
#define HA_MEMORYALLOCATIONPROBLEM			(0x04 | HA_ERROR)
#define HA_HAANDSPSHARESPACE				(0x05 | HA_ERROR)
#define HA_UNABLEUSEDMEMORYMALLOC			(0x06 | HA_ERROR)
#define	HA_MEMORYALLOCATORNOTEXIST			(0x07 | HA_WARNING)
#define	HA_MEMORYALLOCATORNOTCOMPATIBLE		(0x08 | HA_WARNING)
#define	HA_MEMORYALLOCATORNOTINITIALIZED	(0x09 | HA_WARNING)

// Defines for the library's malloc
#define HA_MALLOC_NONE		0
#define HA_MALLOC_CALYPSI	1
#define HA_MALLOC_VCLIB		2
#define HA_MALLOC_LIBM68K	3				// __HeapBase: must always be at the end of list
#define HA_MALLOC_NULL		4


// 
class HeapAllocatorBrowserWindow: public QWidget
{
	Q_OBJECT

		// Calypsi malloc structure
	typedef	struct __allocnode_s
	{
		uint32_t size;           /* Size of this chunk */
		uint32_t preceding;      /* Size of the preceding chunk */
	}S___allocnode_s;

	typedef struct __freenode_s
	{
		uint32_t size;              /* Size of this chunk */
		uint32_t preceding;         /* Size of the preceding chunk */
		uint32_t flink;				/* Supports a doubly linked list */		// struct __freenode_s*
		uint32_t blink;														// struct __freenode_s*
	}S___freenode_s;

	typedef struct __heap_s
	{
		uint32_t heapsize;
		uint32_t heapstart;				// struct __allocnode_s*
		uint32_t heapend;				// struct __allocnode_s*
		S___freenode_s nodelist[2];		// struct __freenode_s
	}S__heap_s;

		// Vclib malloc structure
	typedef struct Vclib_memblock
	{
		uint32_t used;
		uint32_t size;
		uint32_t next;					// struct Vclib_memblock*
		uint32_t prev;					// struct Vclib_memblock*
		uint8_t filler[16];				/* keep qphrase (32 bytes) aligned */
	}S_Vclib_memblock;

		// LibM68K malloc structure
	typedef struct HeapAllocation
	{
		uint32_t nextalloc;
		uint32_t size;
		uint16_t used;
	}	S_HeapAllocation;

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
		size_t CodeMalloc;
		const char* MallocNames[5] = { "", "__default_heap", "___heapptr", "__HeapBase", NULL };		// __HeapBase: must always be at the end of list
		const char* MallocLibNames[5] = { "", "Calypsi", "VBcc", "Lib-M68K", "Unknown" };
};

#endif	// __HEAPALLOCATORBROWSER_H__
