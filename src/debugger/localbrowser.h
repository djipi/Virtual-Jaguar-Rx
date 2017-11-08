//
// localbrowser.h: All Watch
//
// by Jean-Paul Mari
//

#ifndef __LOCALBROWSER_H__
#define __LOCALBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class LocalBrowserWindow: public QWidget
{
	Q_OBJECT

	//
	struct WatchInfo
	{
		//size_t TypeEncoding;
		//size_t TypeByteSize;
		size_t Op;
		size_t Adr;
		int Offset;
		size_t TypeTag;
		size_t TypeEncoding;
		size_t TypeByteSize;
		char *PtrVariableName;
		char *PtrVariableBaseTypeName;
	}S_WatchInfo;

	public:
		LocalBrowserWindow(QWidget *parent = 0);
		~LocalBrowserWindow(void);

	public slots:
//		void DefineAllKeys(void);
		void RefreshContents(void);
		bool UpdateInfos(void);
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
		WatchInfo *LocalInfo;
//		int32_t memBase;
		size_t NbLocal;
		char *FuncName;
};

#endif	// __LOCALBROWSER_H__
