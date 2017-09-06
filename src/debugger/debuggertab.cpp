//
// debuggertab.cpp: "Debugger" tab on the settings dialog
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  06/19/2016  Created this file
// JPM  06/19/2016  Soft debugger support

#include "debuggertab.h"


DebuggerTab::DebuggerTab(QWidget * parent/*= 0*/): QWidget(parent)
{
	QLabel * label3 = new QLabel("Disassembly lines:");
	edit3 = new QLineEdit("");
	edit3->setPlaceholderText("Number of disassembly lines");
	QVBoxLayout * layout1 = new QVBoxLayout;
	layout1->addWidget(label3);

	QVBoxLayout * layout2 = new QVBoxLayout;
	layout2->addWidget(edit3);

	QHBoxLayout * layout3 = new QHBoxLayout;
	layout3->addLayout(layout1);
	layout3->addLayout(layout2);

	QVBoxLayout * layout4 = new QVBoxLayout;
	layout4->addLayout(layout3);

	// Checkboxes...
	displayHWlabels = new QCheckBox(tr("Display HW labels"));
	disasmopcodes	= new QCheckBox(tr("Display M68000 opcodes"));
	displayFullSourceFilename = new QCheckBox(tr("Display source filename"));
	disasmopcodes->setDisabled(false);
	displayHWlabels->setDisabled(false);
	displayFullSourceFilename->setDisabled(false);

	layout4->addWidget(disasmopcodes);
	layout4->addWidget(displayHWlabels);
	layout4->addWidget(displayFullSourceFilename);

	setLayout(layout4);
}


DebuggerTab::~DebuggerTab()
{
}
