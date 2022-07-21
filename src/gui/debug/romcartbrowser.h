//
// romcartbrowser.h: Jaguar ROM cartridge browser
//

#ifndef __ROMCARTBROWSER_H__
#define __ROMCARTBROWSER_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class ROMCartBrowserWindow: public QWidget
{
	Q_OBJECT

	uint32_t TabMaxSize[4] = { 0x900000, 0xa00000, 0xc00000, 0xe00000 };

	enum { ROMCART_1MB, ROMCART_2MB, ROMCART_4MB, ROMCART_6MB };

	public:
		ROMCartBrowserWindow(QWidget * parent = 0);
		~ROMCartBrowserWindow(void);

	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		void ResetAddress(void);
		void GoToAddress(void);
		void CurrentIndexRomCartSize(int index);
		//void wheel(QWheelEvent* e);

	protected:
		void keyPressEvent(QKeyEvent *);
		//void wheelEvent(QMouseEvent*);
		void CheckMaxRomCartSize(void);

	private:
		QComboBox *listromcartsize;
		QVBoxLayout * layout;
//		QTextBrowser * text;
		QLabel * text;
		QPushButton * reset;
		QLineEdit * address;
		QPushButton * go;
		uint32_t memBase;
		size_t romcartsize;
};

#endif	// __ROMCARTBROWSER_H__
