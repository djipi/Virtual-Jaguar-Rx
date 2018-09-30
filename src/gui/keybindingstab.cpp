//
// keybindingstab.cpp: Key Bindings tab on the settings dialog
//
// Part of the Virtual Jaguar Project
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// WHO  WHEN        WHAT
// ---  ----------  ------------------------------------------------------------
// JPM  09/10/2017  Created this file
// JPM  Sept./2018  Added screenshot and savestate key bindings
//


#include "keybindingstab.h"
#include "settings.h"


//
KeyBindings KeyBindingsTable[KB_END] =	{
											{ KB_TYPEGENERAL, "KB_Quit", "Quit", "Quit key binding", "Ctrl+Q", NULL, NULL	},
											{ KB_TYPEGENERAL, "KB_PickFile", "Pick file", "Pick file key binding", "Ctrl+I", NULL, NULL	},
											{ KB_TYPEGENERAL, "KB_Configure", "Configure", "Configure key binding", "Ctrl+C", NULL, NULL	},
											{ KB_TYPEGENERAL, "KB_EmuStatus", "Emulator Status", "Emulator status key binding", "Ctrl+S", NULL, NULL	},
											{ KB_TYPEGENERAL, "KB_Pause", "Pause", "Pause key binding", "Esc", NULL, NULL	},
											{ KB_TYPEGENERAL, "KB_FrameAdvance", "Frame Advance", "Frame advance key binding", "F7", NULL, NULL },
											{ KB_TYPEGENERAL, "KB_FullScreen", "Full Screen", "Full screen key binding", "F9", NULL, NULL	},
											{ KB_TYPEGENERAL, "KB_Screenshot", "Screenshot", "Screenshot key binding", "F8", NULL, NULL	},
											{ KB_TYPEGENERAL, "KB_Savestate", "Savestate", "Savestate key binding", "F6", NULL, NULL	},
											{ KB_TYPEDEBUGGER, "KB_Restart", "Restart", "Restart key binding", "Ctrl+Shift+F5", NULL, NULL	},
											{ KB_TYPEDEBUGGER, "KB_StepInto", "Step Into", "Step into key binding", "F11", NULL, NULL	},
											{ KB_TYPEDEBUGGER, "KB_StepOver", "Step Over", "Step over key binding", "F10", NULL, NULL	}
										};


//
#define	NBKEYBINDINGS	sizeof(KeyBindingsTable)/sizeof(KeyBindings)


// 
KeyBindingsTab::KeyBindingsTab(QWidget * parent/*= 0*/): QWidget(parent)
{
	size_t i;

	QVBoxLayout *layout1 = new QVBoxLayout;
	QVBoxLayout *layout2 = new QVBoxLayout;

	// Initialisation for each layout line
	for (i = 0; i < NBKEYBINDINGS; i++)
	{
		// Prepare the keybinding line
		layout1->addWidget(KeyBindingsTable[i].KBLabel = new QLabel(KeyBindingsTable[i].KBNameLabel));
		layout2->addWidget(KeyBindingsTable[i].KBLineEdit = new QLineEdit(""));
		KeyBindingsTable[i].KBLineEdit->setMaxLength(30);
		KeyBindingsTable[i].KBLineEdit->setPlaceholderText(KeyBindingsTable[i].KBPlaceholderText);

		// Check if keybinding can be editable
		if (KeyBindingsTable[i].KBType != KB_TYPEGENERAL)
		{
			if (vjs.hardwareTypeAlpine && (KeyBindingsTable[i].KBType & KB_TYPEALPINE))
			{
			}
			else
			{
				if (vjs.softTypeDebugger && (KeyBindingsTable[i].KBType & KB_TYPEDEBUGGER))
				{
				}
				else
				{
					KeyBindingsTable[i].KBLabel->hide();
					KeyBindingsTable[i].KBLineEdit->hide();
				}
			}
		}
	}

	// Layouts setup
	QHBoxLayout *layout3 = new QHBoxLayout;
	layout3->addLayout(layout1);
	layout3->addLayout(layout2);
	QVBoxLayout *layout4 = new QVBoxLayout;
	layout4->addLayout(layout3);
	setLayout(layout4);
}


KeyBindingsTab::~KeyBindingsTab()
{
}


// Load / Update the tabs dialog from the settings
void KeyBindingsTab::GetSettings(void)
{
	size_t i;

	for (i = 0; i < NBKEYBINDINGS; i++)
	{
		KeyBindingsTable[i].KBLineEdit->setText(vjs.KBContent[i].KBSettingValue);
	}
}


// Save / Update the settings from the tabs dialog
void KeyBindingsTab::SetSettings(void)
{
	size_t i;

	for (i = 0; i < NBKEYBINDINGS; i++)
	{
		//strcpy(vjs.KBContent[i].KBSettingName, KeyBindingsTable[i].KBNameSetting);
		strcpy(vjs.KBContent[i].KBSettingValue, KeyBindingsTable[i].KBLineEdit->text().toUtf8().data());
	}
}

