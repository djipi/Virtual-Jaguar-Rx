//
// filethread.h: File discovery thread class definition
//

#ifndef __FILETHREAD_H__
#define __FILETHREAD_H__

#include <QtCore>
#include <QImage>
#include <stdint.h>

class FileThread: public QThread
{
	Q_OBJECT

	public:
		FileThread(QObject * parent = 0);
		~FileThread();
		void Go(bool allowUnknown = false);

	signals:
//		void FoundAFile(unsigned long index);														// JPM: Not used
//		void FoundAFile2(unsigned long index, QString filename, QImage * label, unsigned long);		// JPM: Not used
		void FoundAFile3(unsigned long index, QString filename, QImage * label, unsigned long, bool, unsigned long, unsigned long);

	protected:
		void run(void);
		void HandleFile(QFileInfo);
		uint32_t FindCRCIndexInFileList(uint32_t);

	private:
		QMutex mutex;
		QWaitCondition condition;
		bool abort;
		bool allowUnknownSoftware;
};

#endif	// __FILETHREAD_H__
