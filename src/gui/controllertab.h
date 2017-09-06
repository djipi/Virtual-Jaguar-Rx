#ifndef __CONTROLLERTAB_H__
#define __CONTROLLERTAB_H__

#include <QtWidgets>
#include <stdint.h>

class ControllerWidget;

class ControllerTab: public QWidget
{
	Q_OBJECT

	public:
		ControllerTab(QWidget * parent = 0);
		~ControllerTab();

		void SetupLastUsedProfile(void);

	protected slots:
		void DefineAllKeys(void);
		void UpdateProfileKeys(int, uint32_t);
		void UpdateProfileConnections(int);
		void ChangeDevice(int);
		void ChangeMapName(int);
		void AddMapName(void);
		void DeleteMapName(void);

	private:
		QLabel * label1;
		QLabel * label2;
		QLabel * label3;
		QComboBox * deviceList;
		QComboBox * mapNameList;
		QComboBox * mapToList;
		QCheckBox * controller1;
		QCheckBox * controller2;
		QPushButton * addMapName;
		QPushButton * deleteMapName;
		QPushButton * redefineAll;

	public:
		ControllerWidget * controllerWidget;
		int profileNum;
};

#endif	// __CONTROLLERTAB_H__
