#ifndef __EXCEPTIONSTAB_H__
#define __EXCEPTIONSTAB_H__

#include <QtWidgets/QtWidgets>

class ExceptionsTab: public QWidget
{
	Q_OBJECT

	public:
		ExceptionsTab(QWidget * parent = 0);
		~ExceptionsTab();
		void SetSettings(void);
		void GetSettings(void);

	private:
//		QString CheckForSlashes(QString);

	public:
//		QLineEdit *edit1;
//		QLineEdit *edit2;
//		QLineEdit *edit3;
//		QLineEdit *edit3;
//		QLineEdit *edit4;
		QCheckBox *writeROM;
		QCheckBox *M68KExceptionCatch;
		QCheckBox *WriteUnknownMemoryLocation;
//		QCheckBox *useDSP;
//		QCheckBox *useHostAudio;
//		QCheckBox *useUnknownSoftware;
};

#endif	// __EXCEPTIONSTAB_H__
