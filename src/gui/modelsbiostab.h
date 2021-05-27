#ifndef __MODELSBIOSTAB_H__
#define __MODELSBIOSTAB_H__

#include <QtWidgets/QtWidgets>

class ModelsBiosTab: public QWidget
{
	Q_OBJECT

	public:
		ModelsBiosTab(QWidget * parent = 0);
		~ModelsBiosTab();
		void GetSettings(void);
		void SetSettings(void);

	protected slots:
		void CurrentIndexJaguarModel(int index);
		void stateChangedUseRetailBIOS(int useretailbios);
		void stateChangedUseDevBIOS(int usedevbios);
		void CurrentIndexDevBIOS(int index);

	private:
		int JaguarModel;
		int UseRetailBIOS;
		int UseDevBIOS;
		int BIOSValue;
		QComboBox *listJaguarModel;
		QComboBox *listRetailBIOS;
		QCheckBox *useRetailBIOS;
		QComboBox *listDevBIOS;
		QCheckBox *useDevBIOS;

	public:
};

#endif	// __MODELSBIOSTAB_H__
