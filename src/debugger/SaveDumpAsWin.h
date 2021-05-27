//
// SaveDumpAsWin.h: Save Dump function
//
// by Jean-Paul Mari
//

#ifndef __SAVEDUMPASWIN_H__
#define __SAVEDUMPASWIN_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>

class SaveDumpAsWindow : public QWidget
{
	Q_OBJECT

	typedef struct SaveDumpAsInfo
	{
		size_t Size;
		size_t Adr;
	}
	S_SaveDumpAsInfo;

	public:
		SaveDumpAsWindow(QWidget * parent = 0);
		~SaveDumpAsWindow(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	protected slots:
		void SaveDumpAs(void);

	private:
		bool SelectAddress(void);
		bool SelectSize(void);

	private:
		QVBoxLayout *layout;
		QLineEdit *maddress;
		QLineEdit *msize;
		QPushButton *save;
		SaveDumpAsInfo *savedump;
};

#endif	// __SAVEDUMPASWIN_H__
