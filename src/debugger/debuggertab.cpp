//
// debuggertab.cpp: "Debugger" tab on the settings dialog
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  Sept./2016  Created this file, and added Soft debugger support
// JPM  10/09/2018  Added source file search paths
// JPM  04/06/2019  Added ELF sections check
//

#include "debuggertab.h"
#include "settings.h"


// 
DebuggerTab::DebuggerTab(QWidget * parent/*= 0*/): QWidget(parent)
{
	// Number of disassembly lines
	QLabel *label3 = new QLabel("Disassembly lines:");
	QVBoxLayout *layout1 = new QVBoxLayout;
	layout1->addWidget(label3);
	QVBoxLayout *layout2 = new QVBoxLayout;
	nbrdisasmlines = new QLineEdit("");
	nbrdisasmlines->setPlaceholderText("Number of disassembly lines");
	layout2->addWidget(nbrdisasmlines);

	// Sources code paths
	QLabel *label4 = new QLabel("Source file search paths:");
	QVBoxLayout *layout5 = new QVBoxLayout;
	layout5->addWidget(label4);
	QVBoxLayout *layout6 = new QVBoxLayout;
	sourcefilesearchpaths = new QLineEdit("");
	sourcefilesearchpaths->setMaxLength(sizeof(vjs.sourcefilesearchPaths));
	sourcefilesearchpaths->setPlaceholderText("Each path must be separate by a ';', search is recursive and based on each path");
	layout6->addWidget(sourcefilesearchpaths);

	QHBoxLayout *layout3 = new QHBoxLayout;
	layout3->addLayout(layout1);
	layout3->addLayout(layout2);
	QHBoxLayout *layout7 = new QHBoxLayout;
	layout7->addLayout(layout5);
	layout7->addLayout(layout6);

	QVBoxLayout *layout4 = new QVBoxLayout;
	layout4->addLayout(layout3);
	layout4->addLayout(layout7);

	// Checkboxes
	displayHWlabels = new QCheckBox(tr("Display HW labels"));
	disasmopcodes	= new QCheckBox(tr("Display M68000 opcodes"));
	displayFullSourceFilename = new QCheckBox(tr("Display source filename"));
	ELFSectionsCheck = new QCheckBox(tr("ELF sections check"));
	disasmopcodes->setDisabled(false);
	displayHWlabels->setDisabled(false);
	displayFullSourceFilename->setDisabled(false);
	ELFSectionsCheck->setDisabled(false);

	layout4->addWidget(disasmopcodes);
	layout4->addWidget(displayHWlabels);
	layout4->addWidget(displayFullSourceFilename);
	layout4->addWidget(ELFSectionsCheck);

	setLayout(layout4);
}


// 
DebuggerTab::~DebuggerTab()
{
}


// Save / Update the settings from the tabs dialog
void DebuggerTab::SetSettings(void)
{
	bool ok;

	strcpy(vjs.debuggerROMPath, vjs.alpineROMPath);
	strcpy(vjs.sourcefilesearchPaths, CheckForTrailingSlash(sourcefilesearchpaths->text()).toUtf8().data());
	vjs.nbrdisasmlines = nbrdisasmlines->text().toUInt(&ok, 10);
	vjs.displayHWlabels = displayHWlabels->isChecked();
	vjs.disasmopcodes = disasmopcodes->isChecked();
	vjs.displayFullSourceFilename = displayFullSourceFilename->isChecked();
	vjs.ELFSectionsCheck = ELFSectionsCheck->isChecked();
}


// Load / Update the tabs dialog from the settings
void DebuggerTab::GetSettings(void)
{
	QVariant v(vjs.nbrdisasmlines);
	nbrdisasmlines->setText(v.toString());
	sourcefilesearchpaths->setText(vjs.sourcefilesearchPaths);
	displayHWlabels->setChecked(vjs.displayHWlabels);
	disasmopcodes->setChecked(vjs.disasmopcodes);
	displayFullSourceFilename->setChecked(vjs.displayFullSourceFilename);
	ELFSectionsCheck->setChecked(vjs.ELFSectionsCheck);
}


// Remove the last character if slash or backslash at the end of each string
// Depend the platform transform slashes or backslashes
QString DebuggerTab::CheckForTrailingSlash(QString s)
{
	if (s.endsWith('/') || s.endsWith('\\'))
	{
		s.remove(s.length() - 1, 1);
	}
#ifdef _WIN32
	s.replace(QString("/"), QString("\\"));
	s.replace(QString("\\;"), QString(";"));
#else
	s.replace(QString("\\"), QString("/"));
	s.replace(QString("/;"), QString(";"));
#endif
	return s;
}
