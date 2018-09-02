//
// heapallocatorbrowser.h: Memory heap allocation
//
// by Jean-Paul Mari
//

#ifndef __HEAPALLOCATORBROWSER_H__
#define __HEAPALLOCATORBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class HeapAllocatorBrowserWindow: public QWidget
{
	Q_OBJECT

	typedef struct HeapAllocation
	{
		uint32_t nextalloc;
		uint32_t size;
		uint16_t used;
	}S_HeapAllocation;

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
		QTextBrowser *text;
		size_t Adr;
};

#endif	// __HEAPALLOCATORBROWSER_H__
