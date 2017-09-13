#ifndef __ALPINETAB_H__
#define __ALPINETAB_H__

#include <QtWidgets>

class AlpineTab: public QWidget
{
	Q_OBJECT

	public:
		AlpineTab(QWidget * parent = 0);
		~AlpineTab();
		void SetSettings(void);
		void GetSettings(void);

	public:
		QLineEdit * edit1;
		QLineEdit * edit2;
		QLineEdit * edit3;
//		QLineEdit * edit3;
//		QLineEdit * edit4;

		QCheckBox * writeROM;
//		QCheckBox * useDSP;
//		QCheckBox * useHostAudio;
//		QCheckBox * useUnknownSoftware;
};

#endif	// __ALPINETAB_H__
