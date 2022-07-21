#ifndef __GENERALTAB_H__
#define __GENERALTAB_H__

#include <QtWidgets/QtWidgets>

class GeneralTab: public QWidget
{
	Q_OBJECT

	public:
		GeneralTab(QWidget * parent = 0);
		~GeneralTab();
		void SetSettings(void);
		void GetSettings(void);

	private:
		QString CheckForTrailingSlash(QString s);

	public:
		QLineEdit *edit1;
		QLineEdit *edit2;
		QLineEdit *edit3;
		QLineEdit *edit4;
		QLineEdit *edit5;
		QLineEdit *edit6;
#ifndef NEWMODELSBIOSHANDLER
		QCheckBox *useBIOS;
#endif
		QCheckBox *useGPU;
		QCheckBox *useDSP;
//		QCheckBox *useHostAudio;
		QCheckBox *useFullScreen;
		QCheckBox *useUnknownSoftware;
		QCheckBox *useFastBlitter;
};

#endif	// __GENERALTAB_H__
