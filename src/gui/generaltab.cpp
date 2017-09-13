//
// generaltab.cpp: "General" tab on the settings dialog
//
// Part of the Virtual Jaguar Project
// (C) 2011 Underground Software
// See the README and GPLv3 files for licensing and warranty information
//
// JLH = James Hammons <jlhamm@acm.org>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JLH  06/23/2011  Created this file

#include "generaltab.h"
#include "settings.h"


GeneralTab::GeneralTab(QWidget * parent/*= 0*/): QWidget(parent)
{
// I'm thinking we should scan the bios folder for the 5 known BIOSes, and
// just present a radio button to choose between them...
// (BIOS is built-in now...)
//	QLabel * label1 = new QLabel("Boot ROM:");
//	QLabel * label2 = new QLabel("CD Boot ROM:");
	QLabel * label3 = new QLabel("EEPROMs:");
	QLabel * label4 = new QLabel("Software:");

//	edit1 = new QLineEdit("");
//	edit2 = new QLineEdit("");
	edit3 = new QLineEdit("");
	edit4 = new QLineEdit("");
//	edit1->setPlaceholderText("Boot ROM location");
//	edit2->setPlaceholderText("CD Boot ROM location");
	edit3->setPlaceholderText("EEPROM path");
	edit4->setPlaceholderText("Software path");

	QVBoxLayout * layout1 = new QVBoxLayout;
//	layout1->addWidget(label1);
//	layout1->addWidget(label2);
	layout1->addWidget(label3);
	layout1->addWidget(label4);

	QVBoxLayout * layout2 = new QVBoxLayout;
//	layout2->addWidget(edit1);
//	layout2->addWidget(edit2);
	layout2->addWidget(edit3);
	layout2->addWidget(edit4);

	QHBoxLayout * layout3 = new QHBoxLayout;
	layout3->addLayout(layout1);
	layout3->addLayout(layout2);

	QVBoxLayout * layout4 = new QVBoxLayout;
	layout4->addLayout(layout3);

	// Checkboxes...
	useBIOS            = new QCheckBox(tr("Enable Jaguar BIOS"));
	useGPU             = new QCheckBox(tr("Enable GPU"));
	useDSP             = new QCheckBox(tr("Enable DSP"));
	useFullScreen      = new QCheckBox(tr("Start Virtual Jaguar in full screen"));
//	useHostAudio       = new QCheckBox(tr("Enable audio playback (requires DSP)"));
	useUnknownSoftware = new QCheckBox(tr("Show all files in file chooser"));
	useFastBlitter     = new QCheckBox(tr("Use fast blitter"));

	layout4->addWidget(useBIOS);
	layout4->addWidget(useGPU);
	layout4->addWidget(useDSP);
	layout4->addWidget(useFullScreen);
//	layout4->addWidget(useHostAudio);
	layout4->addWidget(useUnknownSoftware);
	layout4->addWidget(useFastBlitter);

	setLayout(layout4);
}


GeneralTab::~GeneralTab()
{
}


// Load / Update the tabs dialog from the settings
void GeneralTab::GetSettings(void)
{
	//	generalTab->edit1->setText(vjs.jagBootPath);
	//	generalTab->edit2->setText(vjs.CDBootPath);
	edit3->setText(vjs.EEPROMPath);
	edit4->setText(vjs.ROMPath);
	useBIOS->setChecked(vjs.useJaguarBIOS);
	useGPU->setChecked(vjs.GPUEnabled);
	useDSP->setChecked(vjs.DSPEnabled);
	useFullScreen->setChecked(vjs.fullscreen);
	//	generalTab->useHostAudio->setChecked(vjs.audioEnabled);
	useFastBlitter->setChecked(vjs.useFastBlitter);
}


// Save / Update the settings from the tabs dialog
void GeneralTab::SetSettings(void)
{
	//	strcpy(vjs.jagBootPath, generalTab->edit1->text().toAscii().data());
	//	strcpy(vjs.CDBootPath,  generalTab->edit2->text().toAscii().data());
	strcpy(vjs.EEPROMPath, CheckForTrailingSlash(edit3->text()).toUtf8().data());
	strcpy(vjs.ROMPath, CheckForTrailingSlash(edit4->text()).toUtf8().data());

	vjs.useJaguarBIOS = useBIOS->isChecked();
	vjs.GPUEnabled = useGPU->isChecked();
	vjs.DSPEnabled = useDSP->isChecked();
	vjs.fullscreen = useFullScreen->isChecked();
	//	vjs.audioEnabled   = generalTab->useHostAudio->isChecked();
	vjs.useFastBlitter = useFastBlitter->isChecked();
}


// Append a slash or a backslash at the end of the string
QString GeneralTab::CheckForTrailingSlash(QString s)
{
	if (!s.endsWith('/') && !s.endsWith('\\'))
		s.append('/');

	return s;
}


#if 0
	vjs.useJoystick      = settings.value("useJoystick", false).toBool();
	vjs.joyport          = settings.value("joyport", 0).toInt();
	vjs.frameSkip        = settings.value("frameSkip", 0).toInt();
	vjs.useJaguarBIOS    = settings.value("useJaguarBIOS", false).toBool();
	vjs.DSPEnabled       = settings.value("DSPEnabled", false).toBool();
	vjs.usePipelinedDSP  = settings.value("usePipelinedDSP", false).toBool();
	vjs.fullscreen       = settings.value("fullscreen", false).toBool();
	vjs.renderType       = settings.value("renderType", 0).toInt();
	strcpy(vjs.jagBootPath, settings.value("JagBootROM", "./bios/[BIOS] Atari Jaguar (USA, Europe).zip").toString().toAscii().data());
	strcpy(vjs.CDBootPath, settings.value("CDBootROM", "./bios/jagcd.rom").toString().toAscii().data());
	strcpy(vjs.EEPROMPath, settings.value("EEPROMs", "./eeproms").toString().toAscii().data());
	strcpy(vjs.ROMPath, settings.value("ROMs", "./software").toString().toAscii().data());
#endif
