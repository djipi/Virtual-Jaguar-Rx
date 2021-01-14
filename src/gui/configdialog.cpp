//
// configdialog.cpp - Configuration dialog
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  01/29/2010  Created this file
// JLH  06/23/2011  Added initial implementation
// JLH  10/14/2011  Fixed possibly missing final slash in paths
// JPM  06/06/2016  Visual Studio support
// JPM  06/19/2016  Soft debugger support
// JPM  09/  /2017  Added a Keybindings tab
// JPM  09/03/2018  Added a Models & Bios tab
//

#include "configdialog.h"
#include "alpinetab.h"
#include "debugger/debuggertab.h"
#include "controllertab.h"
#include "controllerwidget.h"
#include "generaltab.h"
#include "modelsbiostab.h"
#include "keybindingstab.h"
#include "settings.h"


ConfigDialog::ConfigDialog(QWidget * parent/*= 0*/) : QDialog(parent),
tabWidget(new QTabWidget),
generalTab(new GeneralTab(this)),
#ifdef NEWMODELSBIOSHANDLER
modelsbiosTab(new ModelsBiosTab),
#endif
controllerTab1(new ControllerTab(this)),
keybindingsTab(new KeyBindingsTab(this))
{
//	tabWidget = new QTabWidget;
//	generalTab = new GeneralTab(this);
//	controllerTab1 = new ControllerTab(this);
////	controllerTab2 = new ControllerTab(this);

//	if (vjs.hardwareTypeAlpine)
//		alpineTab = new AlpineTab(this);

	tabWidget->addTab(generalTab, tr("General"));
#ifdef NEWMODELSBIOSHANDLER
	tabWidget->addTab(modelsbiosTab, tr("Models and BIOS"));
#endif
	tabWidget->addTab(controllerTab1, tr("Controllers"));
//	tabWidget->addTab(controllerTab2, tr("Controller #2"));
	tabWidget->addTab(keybindingsTab, tr("Key Bindings"));

	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		alpineTab = new AlpineTab(this);
		tabWidget->addTab(alpineTab, tr("Alpine"));
	}

	if (vjs.softTypeDebugger)
	{
		debuggerTab = new DebuggerTab(this);
		tabWidget->addTab(debuggerTab, tr("Debugger"));
	}

	buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);

	connect(buttonBox, SIGNAL(accepted()), this, SLOT(accept()));
	connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

	QVBoxLayout * mainLayout = new QVBoxLayout;
	mainLayout->addWidget(tabWidget);
	mainLayout->addWidget(buttonBox);
	setLayout(mainLayout);

	setWindowTitle(tr("Virtual Jaguar Settings"));
	LoadDialogFromSettings();
}


ConfigDialog::~ConfigDialog()
{
}


// Load & Update the tabs dialog from the settings
void ConfigDialog::LoadDialogFromSettings(void)
{
	// General & Keybindings tab settings
	generalTab->GetSettings();
	keybindingsTab->GetSettings();
#ifdef NEWMODELSBIOSHANDLER
	modelsbiosTab->GetSettings();
#endif

	// Alpine tab settings (also needed by the Debugger)
	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		alpineTab->GetSettings();
	}

	// Debugger tab settings
	if (vjs.softTypeDebugger)
	{
		debuggerTab->GetSettings();
	}

#ifdef _MSC_VER
#pragma message("Warning: !!! Need to load settings from controller profile !!!")
#else
#warning "!!! Need to load settings from controller profile !!!"
#endif // _MSC_VER
// We do this now, but not here. Need to fix this...
#if 0
	for(int i=0; i<21; i++)
	{
// We need to find the right profile and load it up here...
		controllerTab1->controllerWidget->keys[i] = vjs.p1KeyBindings[i];
//		controllerTab2->controllerWidget->keys[i] = vjs.p2KeyBindings[i];
	}
#endif
}


// Save & Update the settings from the tabs dialog
void ConfigDialog::UpdateVJSettings(void)
{
	generalTab->SetSettings();
	keybindingsTab->SetSettings();
#ifdef NEWMODELSBIOSHANDLER
	modelsbiosTab->SetSettings();
#endif

	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		alpineTab->SetSettings();
	}

	if (vjs.softTypeDebugger)
	{
		debuggerTab->SetSettings();
	}

#ifdef _MSC_VER
#pragma message("Warning: !!! Need to save settings to controller profile !!!")
#else
#warning "!!! Need to save settings to controller profile !!!"
#endif // _MSC_VER
// We do this now, but not here. Need to fix this...
#if 0
	for(int i=0; i<21; i++)
	{
// We need to find the right profile and load it up here...
		vjs.p1KeyBindings[i] = controllerTab1->controllerWidget->keys[i];
//		vjs.p2KeyBindings[i] = controllerTab2->controllerWidget->keys[i];
	}
#endif
}

