//
// brkWin.h: Breakpoints
//
// by Jean-Paul Mari
//

#ifndef __BRKWIN_H__
#define __BRKWIN_H__

#include <QtWidgets>
#include <stdint.h>


class BrkWindow: public QWidget
{
	Q_OBJECT

	//
	struct BrkInfo
	{
		size_t Adr;
		bool IsActive;
		bool IsReached;
		size_t NbrHit;
	}S_BrkInfo;

	public:
		BrkWindow(QWidget *parent = 0);
		~BrkWindow(void);

	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		void RefreshBrkList(size_t Adress);
		void GoToAddress(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
//		QTextBrowser * text;
//		QLabel *text;
		QTextBrowser *text;
//		QPushButton *refresh;
		QLineEdit *address;
//		QPushButton *go;
//		BrkInfo *PtrBrkInfo;
//		int32_t memBase;
//		size_t NbBrk;
};

#endif	// __BRKWIN_H__
