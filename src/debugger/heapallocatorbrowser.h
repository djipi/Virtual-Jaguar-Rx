//
// memoryheapallocatorbrowser.h: All Watch
//
// by James Hammons
// (C) 2012 Underground Software
//

#ifndef __HEAPALLOCATORBROWSER_H__
#define __HEAPALLOCATORBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class HeapAllocatorBrowserWindow: public QWidget
{
	Q_OBJECT

	struct HeapAllocation
	{
		//UINT32 nextalloc;
		uint32_t nextalloc;
		//UINT32 size;
		uint32_t size;
		//UINT16 used;
		uint16_t used;
	}S_HeapAllocation;

	public:
		HeapAllocatorBrowserWindow(QWidget *parent = 0);
		~HeapAllocatorBrowserWindow(void);

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
//		QPushButton *refresh;
//		QLineEdit *address;
//		QPushButton *go;
//		WatchInfo *PtrWatchInfo;
//		int32_t memBase;
//		int32_t NbWatch;
		size_t Adr;
//		HeapAllocation HeapAllocation;
};

#endif	// __HEAPALLOCATORBROWSER_H__
