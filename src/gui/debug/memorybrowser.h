//
// memorybrowser.h: Jaguar memory browser
//
// by James Hammons
// (C) 2012 Underground Software
//
// Modified in March 2022, by Jean-Paul Mari
//

#ifndef __MEMORYBROWSER_H__
#define __MEMORYBROWSER_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>
#include "memory.h"
#include "dsp.h"
#include "gpu.h"

class MemoryBrowserWindow: public QWidget
{
	Q_OBJECT

	struct raminfo {
		char *WindowTitle;
		int32_t memmin, memmax;
		uint8_t* memzone;
	};

	enum { MAINDRAM, DSPSRAM, GPUSRAM };

	raminfo MemTypeInfo[3] = {
		{ (char*)"Main Memory Browser", 0x000000, 0x200000 /* 0x1fffff */, jaguarMainRAM },
		{ (char*)"DSP Memory Browser", 0xf1b000, 0xf1d000 /* 0xf1cfff */, dsp_ram_8  },
		{ (char*)"GPU Memory Browser", 0xf03000, 0xf04000 /* 0xf03fff */, gpu_ram_8  }
	};

	public:
		MemoryBrowserWindow(QWidget * parent /* = 0 */, int Type);
		~MemoryBrowserWindow();

	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		void GoToAddress(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		void CheckMemZone(void);

	private:
		QVBoxLayout *layout;
//		QTextBrowser *text;
		QLabel *text;
		QPushButton *refresh;
		QLineEdit *address;
		QPushButton *go;
		int32_t memmax;
		int32_t memmin;
		int32_t memBase;
		uint8_t *memzone;
		int memtype;
};

#endif	// __MEMORYBROWSER_H__
