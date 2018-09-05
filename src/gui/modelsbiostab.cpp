//
// modelsbiostab.cpp: Models & Bios tab on the settings dialog
//
// Part of the Virtual Jaguar Project
/// See the README and GPLv3 files for licensing and warranty information
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  09/03/2018  Created this file
//

#include "configdialog.h"
#include "modelsbiostab.h"
#include "settings.h"


// 
ModelsBiosTab::ModelsBiosTab(QWidget * parent/*= 0*/): QWidget(parent),
listJaguarModel(new QComboBox()),
listRetailBIOS(new QComboBox()),
listDevBIOS(new QComboBox()),
JaguarModel(JAG_NULL_SERIES),
UseRetailBIOS(0),
UseDevBIOS(0),
BIOSValue(BT_NULL)
{
#ifdef NEWMODELSBIOSHANDLER
	// Jaguar model
	QLabel *labelModel = new QLabel("Jaguar model");
	QHBoxLayout *layoutModel = new QHBoxLayout;
	layoutModel->addWidget(labelModel);
	listJaguarModel->addItem("Model K", QVariant(JAG_K_SERIES));
	listJaguarModel->addItem("Model M", QVariant(JAG_M_SERIES));
	layoutModel->addWidget(listJaguarModel);
	QVBoxLayout *layout1 = new QVBoxLayout;
	layout1->addLayout(layoutModel);

	// Retail BIOS select
	QHBoxLayout *layoutRetailBIOS = new QHBoxLayout;
	useRetailBIOS = new QCheckBox(tr("Retail BIOS"));
	layoutRetailBIOS->addWidget(useRetailBIOS);
	QVBoxLayout *layout2 = new QVBoxLayout;
	layout2->addLayout(layoutRetailBIOS);
	// BIOS selection
	layoutRetailBIOS->addWidget(listRetailBIOS);

	// Developer BIOS select
	QHBoxLayout *layoutDevBIOS = new QHBoxLayout;
	useDevBIOS = new QCheckBox(tr("Developer BIOS"));
	layoutDevBIOS->addWidget(useDevBIOS);
	QVBoxLayout *layout3 = new QVBoxLayout;
	layout3->addLayout(layoutDevBIOS);
	// BIOS selection
	listDevBIOS->hide();
	listDevBIOS->addItem("Stubulator '93", QVariant(BT_STUBULATOR_1));
	listDevBIOS->addItem("Stubulator '94", QVariant(BT_STUBULATOR_2));
	layoutDevBIOS->addWidget(listDevBIOS);

	// Set layouts
	layout1->addLayout(layout2);
	layout1->addLayout(layout3);
	setLayout(layout1);

	// Connections
	connect(useRetailBIOS, SIGNAL(stateChanged(int)), this, SLOT(stateChangedUseRetailBIOS(int)));
	connect(useDevBIOS, SIGNAL(stateChanged(int)), this, SLOT(stateChangedUseDevBIOS(int)));
	connect(listJaguarModel, SIGNAL(currentIndexChanged(int)), this, SLOT(CurrentIndexJaguarModel(int)));
	connect(listDevBIOS, SIGNAL(currentIndexChanged(int)), this, SLOT(CurrentIndexDevBIOS(int)));
#endif
}


//
void ModelsBiosTab::stateChangedUseDevBIOS(int usedevbios)
{
#ifdef NEWMODELSBIOSHANDLER
	if ((UseDevBIOS = usedevbios))
	{
		stateChangedUseRetailBIOS(0);
		BIOSValue = listDevBIOS->itemData(listDevBIOS->currentIndex()).toInt();
		listDevBIOS->show();
	}
	else
	{
		useDevBIOS->setChecked(false);
		listDevBIOS->hide();
	}
#endif
}


//
void ModelsBiosTab::stateChangedUseRetailBIOS(int useretailbios)
{
#ifdef NEWMODELSBIOSHANDLER
	if ((UseRetailBIOS = useretailbios))
	{
		stateChangedUseDevBIOS(0);

		switch (JaguarModel)
		{
		case JAG_K_SERIES:
			listRetailBIOS->clear();
			listRetailBIOS->addItem("Model K", QVariant(BT_K_SERIES));
			BIOSValue = BT_K_SERIES;
			break;

		case JAG_M_SERIES:
			listRetailBIOS->clear();
			listRetailBIOS->addItem("Model M", QVariant(BT_M_SERIES));
			BIOSValue = BT_M_SERIES;
			break;

		default:
			break;
		}

		listRetailBIOS->show();
	}
	else
	{
		useRetailBIOS->setChecked(false);
		listRetailBIOS->hide();
	}
#endif
}


// Get the index from the Jaguar models list
void ModelsBiosTab::CurrentIndexJaguarModel(int index)
{
#ifdef NEWMODELSBIOSHANDLER
	JaguarModel = listJaguarModel->itemData(index).toInt();
	stateChangedUseRetailBIOS(UseRetailBIOS);
#endif
}


// Get the index from the developer BIOS list
void ModelsBiosTab::CurrentIndexDevBIOS(int index)
{
#ifdef NEWMODELSBIOSHANDLER
	BIOSValue = listDevBIOS->itemData(index).toInt();
#endif
}


// 
ModelsBiosTab::~ModelsBiosTab()
{
#ifdef NEWMODELSBIOSHANDLER
#endif
}


// Load & Update the tabs dialog from the settings
void ModelsBiosTab::GetSettings(void)
{
#ifdef NEWMODELSBIOSHANDLER
	listJaguarModel->setCurrentIndex(listJaguarModel->findData((JaguarModel = vjs.jaguarModel)));
	listDevBIOS->setCurrentIndex(listDevBIOS->findData((BIOSValue = vjs.biosType)));
	useRetailBIOS->setChecked((UseRetailBIOS = vjs.useRetailBIOS));
	useDevBIOS->setChecked((UseDevBIOS = vjs.useDevBIOS));
#endif
}


// Save & Update the settings from the tabs dialog
void ModelsBiosTab::SetSettings(void)
{
#ifdef NEWMODELSBIOSHANDLER
	vjs.jaguarModel = JaguarModel;
	if (!(vjs.useJaguarBIOS = (UseRetailBIOS | UseDevBIOS)))
	{
		vjs.biosType = 0;
	}
	else
	{
		vjs.biosType = BIOSValue;
	}
	vjs.useRetailBIOS = UseRetailBIOS;
	vjs.useDevBIOS = UseDevBIOS;
#endif
}
