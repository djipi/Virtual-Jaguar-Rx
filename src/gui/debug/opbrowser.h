//
// opbrowser.h: Jaguar memory browser
//
// by James Hammons
// (C) 2012 Underground Software
//

#ifndef __OPBROWSER_H__
#define __OPBROWSER_H__

#include <QtWidgets>
#include <stdint.h>

class OPBrowserWindow: public QWidget
{
	Q_OBJECT

	public:
		OPBrowserWindow(QWidget * parent = 0);


	public slots:
		void RefreshContents(void);

	protected:
		void keyPressEvent(QKeyEvent *);

		bool ObjectExists(uint32_t address);
		void DiscoverObjects(uint32_t address);
		void DumpObjectList(QString &);
		void DumpScaledObject(QString &, uint64_t p0, uint64_t p1, uint64_t p2);
		void DumpFixedObject(QString &, uint64_t p0, uint64_t p1);
		void DumpBitmapCore(QString &, uint64_t p0, uint64_t p1);

	private:
		QVBoxLayout * layout;
//		QTextBrowser * text;
		QLabel * text;
		QPushButton * refresh;

//		int32_t memBase;
		uint32_t object[8192];
		uint32_t numberOfObjects;
};

#endif	// __OPBROWSER_H__
