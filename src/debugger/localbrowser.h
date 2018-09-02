//
// localbrowser.h: Local variables
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
	typedef struct WatchInfo
	{
		size_t Op;
		size_t Adr;
		int Offset;
		size_t TypeTag;
		size_t TypeEncoding;
		size_t TypeByteSize;
		char *PtrVariableName;
		char *PtrVariableBaseTypeName;
		char *PtrCPURegisterName;
	}S_WatchInfo;

	public:
		LocalBrowserWindow(QWidget *parent = 0);
		~LocalBrowserWindow(void);

	public slots:
		void RefreshContents(void);
		bool UpdateInfos(void);

	protected:
		void keyPressEvent(QKeyEvent *);

	private:
		QVBoxLayout *layout;
		QTextBrowser *text;
		WatchInfo *LocalInfo;
		size_t NbLocal;
		char *FuncName;
};

#endif	// __LOCALBROWSER_H__
