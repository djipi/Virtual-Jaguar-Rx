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
//

#include "exceptionstab.h"
#include "settings.h"


ExceptionsTab::ExceptionsTab(QWidget * parent/*= 0*/): QWidget(parent)
{
//	QLabel * label1 = new QLabel("ROM to load:");
//	QLabel * label2 = new QLabel("ABS to load:");
//	QLabel * label3 = new QLabel("Windows refresh:");
//	QLabel * label3 = new QLabel("EEPROMs:");
//	QLabel * label4 = new QLabel("Software:");

//	edit1 = new QLineEdit("");
//	edit2 = new QLineEdit("");
//	edit3 = new QLineEdit("");
//	edit3 = new QLineEdit("");
//	edit4 = new QLineEdit("");
//	edit1->setPlaceholderText("ROM to load when Virtual Jaguar loads");
//	edit2->setPlaceholderText("ABS to load when Virtual Jaguar loads");
//	edit3->setPlaceholderText("Windows refresh rate");
//	edit3->setPlaceholderText("EEPROM path");
//	edit4->setPlaceholderText("Software path");

//	QVBoxLayout * layout1 = new QVBoxLayout;
//	layout1->addWidget(label1);
//	layout1->addWidget(label2);
//	layout1->addWidget(label3);
//	layout1->addWidget(label3);
//	layout1->addWidget(label4);

//	QVBoxLayout * layout2 = new QVBoxLayout;
//	layout2->addWidget(edit1);
//	layout2->addWidget(edit2);
//	layout2->addWidget(edit3);
//	layout2->addWidget(edit3);
//	layout2->addWidget(edit4);

//	QHBoxLayout * layout3 = new QHBoxLayout;
//	layout3->addLayout(layout1);
//	layout3->addLayout(layout2);

	QVBoxLayout * layout4 = new QVBoxLayout;
//	layout4->addLayout(layout3);

	// Checkboxes...
	writeROM = new QCheckBox(tr("Allow writes to cartridge ROM"));
	M68KExceptionCatch = new QCheckBox(tr("Allow M68000 exception catch"));
	WriteUnknownMemoryLocation = new QCheckBox(tr("Allow writes to unknown memory location"));
//	useDSP             = new QCheckBox(tr("Enable DSP"));
//	useHostAudio       = new QCheckBox(tr("Enable audio playback"));
//	useUnknownSoftware = new QCheckBox(tr("Allow unknown software in file chooser"));
// Currently, this is unused, so let's signal this to the user:
	//writeROM->setDisabled(true);

	layout4->addWidget(writeROM);
	layout4->addWidget(M68KExceptionCatch);
	layout4->addWidget(WriteUnknownMemoryLocation);
//	layout4->addWidget(useDSP);
//	layout4->addWidget(useHostAudio);
//	layout4->addWidget(useUnknownSoftware);

	setLayout(layout4);
}


//
ExceptionsTab::~ExceptionsTab()
{
}


// Load / Update the tabs dialog from the settings
void ExceptionsTab::GetSettings(void)
{
//	QVariant v(vjs.refresh);
//	edit1->setText(vjs.alpineROMPath);
//	edit2->setText(vjs.absROMPath);
//	edit3->setText(v.toString());
	writeROM->setChecked(vjs.allowWritesToROM);
	M68KExceptionCatch->setChecked(vjs.allowM68KExceptionCatch);
	WriteUnknownMemoryLocation->setChecked(vjs.allowWritesToUnknownLocation);
}


// Save / Update the settings from the tabs dialog
void ExceptionsTab::SetSettings(void)
{
//	bool ok;

//	strcpy(vjs.alpineROMPath, CheckForSlashes(edit1->text()).toUtf8().data());
//	strcpy(vjs.absROMPath, CheckForSlashes(edit2->text()).toUtf8().data());
//	vjs.refresh = edit3->text().toUInt(&ok, 10);
	vjs.allowWritesToROM = writeROM->isChecked();
	vjs.allowM68KExceptionCatch = M68KExceptionCatch->isChecked();
	vjs.allowWritesToUnknownLocation = WriteUnknownMemoryLocation->isChecked();
}


// Depend the platform transform slashes or backslashes
//QString AlpineTab::CheckForSlashes(QString s)
//{
//#ifdef _WIN32
//	s.replace(QString("/"), QString("\\"));
//#else
//	s.replace(QString("\\"), QString("/"));
//#endif
//	return s;
//
