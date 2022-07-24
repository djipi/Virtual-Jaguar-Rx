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

	public:
		QCheckBox *writeROM;
		QCheckBox *M68KExceptionCatch;
		QCheckBox *WriteUnknownMemoryLocation;
		QCheckBox *JERRYUnkwnRegsCatch;
		QCheckBox *JERRYAllowWriteWaveTable;
};

#endif	// __EXCEPTIONSTAB_H__
