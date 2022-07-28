//
// exceptionstab.cpp: "Exceptions" tab on the settings dialog
//
// Part of the Virtual Jaguar Rx Project
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  March/2022  Created this file based from the alpinetab source code
// JPM   July/2022  Layout changes, added JERRY settings
//

// STILL TO DO:
// To arrange the checkbox positions
// 

#include "exceptionstab.h"
#include "settings.h"


// 
ExceptionsTab::ExceptionsTab(QWidget * parent/*= 0*/): QWidget(parent)
{
	// create layout
	QVBoxLayout *layout4 = new QVBoxLayout;
	// General
	QGroupBox *box1 = new QGroupBox("General");
	QVBoxLayout *boxGeneral = new QVBoxLayout(box1);
	writeROM = new QCheckBox(tr("Allow writes to cartridge ROM"));
	WriteUnknownMemoryLocation = new QCheckBox(tr("Allow writes to unknown memory location"));
	// M68K
	QGroupBox *box2 = new QGroupBox("M68000");
	QVBoxLayout *boxM68000 = new QVBoxLayout(box2);
	M68KExceptionCatch = new QCheckBox(tr("Allow M68000 exception catch"));
	// JERRY
	QGroupBox *box3 = new QGroupBox("JERRY");
	QVBoxLayout *boxJERRY = new QVBoxLayout(box3);
	JERRYAllowWriteWaveTable = new QCheckBox(tr("Allow writes to the JERRY's wave table"));
	JERRYAllowWriteWaveTable->setDisabled(true);
	JERRYUnkwnRegsCatch = new QCheckBox(tr("Allow unsupported JERRY's registers catch"));
	JERRYUnkwnRegsCatch->setDisabled(true);

	// add checkboxes to the layouts
	boxGeneral->addWidget(writeROM);
	boxGeneral->addWidget(WriteUnknownMemoryLocation);
	boxM68000->addWidget(M68KExceptionCatch);
	boxJERRY->addWidget(JERRYAllowWriteWaveTable);
	boxJERRY->addWidget(JERRYUnkwnRegsCatch);

	// set layout
	layout4->addWidget(box1, 0, 0);
	layout4->addWidget(box2, 0, 0);
	layout4->addWidget(box3, 0, 0);
	setLayout(layout4);
}


//
ExceptionsTab::~ExceptionsTab()
{
}


// Load / Update the tabs dialog from the settings
void ExceptionsTab::GetSettings(void)
{
	writeROM->setChecked(vjs.allowWritesToROM);
	M68KExceptionCatch->setChecked(vjs.allowM68KExceptionCatch);
	WriteUnknownMemoryLocation->setChecked(vjs.allowWritesToUnknownLocation);
	JERRYAllowWriteWaveTable->setChecked(vjs.AllowJERRYWriteWaveTable);
	JERRYUnkwnRegsCatch->setChecked(vjs.AllowJERRYUnkwnRegsCatch);
}


// Save / Update the settings from the tabs dialog
void ExceptionsTab::SetSettings(void)
{
	vjs.allowWritesToROM = writeROM->isChecked();
	vjs.allowM68KExceptionCatch = M68KExceptionCatch->isChecked();
	vjs.allowWritesToUnknownLocation = WriteUnknownMemoryLocation->isChecked();
	vjs.AllowJERRYWriteWaveTable = JERRYAllowWriteWaveTable->isChecked();
	vjs.AllowJERRYUnkwnRegsCatch = JERRYUnkwnRegsCatch->isChecked();
}

