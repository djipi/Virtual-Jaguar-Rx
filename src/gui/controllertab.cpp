//
// controllertab.cpp: "Controller" tab on the config dialog
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
// JLH  07/20/2011  Fixed a bunch of stuff
// JLH  10/02/2014  Fixed even more stuff, related to the profile system
//

/*
To really fix this shit, we have to straighten out some stuff. So here goes:

We have a database of profiles consisting of a device list (devices that have
been seen already) and map list (consisting of a key into the device list, a
human readable name, a preferred slot #, and a key/button mapping). This is a
list that can hold up to 64 different profiles.

We have a a list of attached gamepads in Gamepad::. There can be 8 at most
attached any one time.

There are two game port slots that a controller can be hooked into.

So, what we need to do when configuring and/or using this system is this.

 - Populate the device combobox with the list of attached devices from the
   profile database.
 - Populate the map to combobox with the profiles associated with that profile
   device number.
 - Save stuff when the user changes stuff (this happens already)
*/

#include "controllertab.h"

#include "controllerwidget.h"
#include "gamepad.h"
#include "joystick.h"
#include "keygrabber.h"
#include "profile.h"


ControllerTab::ControllerTab(QWidget * parent/*= 0*/): QWidget(parent),
	label1(new QLabel(tr("Host Device:"))),
	label2(new QLabel(tr("Map Name:"))),
	label3(new QLabel(tr("Maps to:"))),
	deviceList(new QComboBox(this)),
	mapNameList(new QComboBox(this)),
	mapToList(new QComboBox(this)),
	addMapName(new QPushButton(tr("+"))),
	deleteMapName(new QPushButton(tr("-"))),
	redefineAll(new QPushButton(tr("Define All Inputs"))),
	controllerWidget(new ControllerWidget(this))
{
	QVBoxLayout * layout = new QVBoxLayout;
	QHBoxLayout * top = new QHBoxLayout;
	QVBoxLayout * left = new QVBoxLayout;
	QVBoxLayout * right = new QVBoxLayout;
	QHBoxLayout * middle = new QHBoxLayout;
	top->addLayout(left, 0);
	top->addLayout(right, 1);
	layout->addLayout(top);
	left->addWidget(label1, 0, Qt::AlignRight);
	left->addWidget(label2, 0, Qt::AlignRight);
	left->addWidget(label3, 0, Qt::AlignRight);
	right->addWidget(deviceList);

	right->addLayout(middle);
	middle->addWidget(mapNameList, 1);
	middle->addWidget(addMapName, 0);
	middle->addWidget(deleteMapName, 0);

	right->addWidget(mapToList);
	layout->addWidget(controllerWidget);
	layout->addWidget(redefineAll, 0, Qt::AlignHCenter);
	setLayout(layout);
	// At least by doing this, it keeps the QComboBox from resizing itself too
	// large and breaking the layout. :-P
	setFixedWidth(sizeHint().width());

	connect(redefineAll, SIGNAL(clicked()), this, SLOT(DefineAllKeys()));
	connect(deviceList, SIGNAL(activated(int)), this, SLOT(ChangeDevice(int)));
	connect(mapNameList, SIGNAL(activated(int)), this, SLOT(ChangeMapName(int)));
	connect(addMapName, SIGNAL(clicked()), this, SLOT(AddMapName()));
	connect(deleteMapName, SIGNAL(clicked()), this, SLOT(DeleteMapName()));
	connect(controllerWidget, SIGNAL(KeyDefined(int, uint32_t)), this, SLOT(UpdateProfileKeys(int, uint32_t)));
	connect(mapToList, SIGNAL(activated(int)), this, SLOT(UpdateProfileConnections(int)));

	// Set up the device combobox (Keyboard is the default, and always
	// present)
	deviceList->addItem(tr("Keyboard"), 0);

	for(int i=0; i<Gamepad::numJoysticks; i++)
	{
		int deviceNum = FindDeviceNumberForName(Gamepad::GetJoystickName(i));
		deviceList->addItem(Gamepad::GetJoystickName(i), deviceNum);
	}

	// Set up "Map To" combobox
	mapToList->addItem(tr("None"), 0);
	mapToList->addItem(tr("Controller #1"), CONTROLLER1);
	mapToList->addItem(tr("Controller #2"), CONTROLLER2);
	mapToList->addItem(tr("Either one that's free"), CONTROLLER1 | CONTROLLER2);
}


ControllerTab::~ControllerTab()
{
}


void ControllerTab::SetupLastUsedProfile(void)
{
	int deviceNumIndex = deviceList->findData(profile[profileNum].device);
	int mapNumIndex = mapNameList->findText(profile[profileNum].mapName);

	if (deviceNumIndex == -1 || mapNumIndex == -1)
	{
		// We're doing the default, so set it up...
		deviceNumIndex = 0;
		mapNumIndex = 0;
		profileNum = 0;
	}

	deviceList->setCurrentIndex(deviceNumIndex);
	mapNameList->setCurrentIndex(mapNumIndex);

	int controllerIndex = mapToList->findData(profile[profileNum].preferredSlot);
	mapToList->setCurrentIndex(controllerIndex);

	// We have to do this manually, since it's no longer done automagically...
	ChangeDevice(deviceNumIndex);
	ChangeMapName(mapNumIndex);
}


void ControllerTab::DefineAllKeys(void)
{
//	char jagButtonName[21][10] = { "Up", "Down", "Left", "Right",
//		"*", "7", "4", "1", "0", "8", "5", "2", "#", "9", "6", "3",
//		"A", "B", "C", "Option", "Pause" };
	int orderToDefine[21] = { 0, 1, 2, 3, 18, 17, 16, 20, 19, 7, 11, 15, 6, 10, 14, 5, 9, 13, 8, 4, 12 };
	KeyGrabber keyGrab(this);

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		keyGrab.SetKeyText(orderToDefine[i]);
		keyGrab.exec();
		int key = keyGrab.key;

		if (key == Qt::Key_Escape)
			break;

		// Otherwise, populate the appropriate spot in the settings & update
		// the screen...
		controllerWidget->keys[orderToDefine[i]] = key;
		controllerWidget->update();
		profile[profileNum].map[orderToDefine[i]] = key;
	}
}


void ControllerTab::UpdateProfileKeys(int mapPosition, uint32_t key)
{
	profile[profileNum].map[mapPosition] = key;
}


void ControllerTab::UpdateProfileConnections(int selection)
{
	profile[profileNum].preferredSlot = mapToList->itemData(selection).toInt();
}


void ControllerTab::ChangeDevice(int selection)
{
	int deviceNum = deviceList->itemData(selection).toInt();
	mapNameList->clear();
	int numberOfMappings = FindMappingsForDevice(deviceNum, mapNameList);
	// Make sure to disable the "-" button is there's only one mapping for this
	// device...
	deleteMapName->setDisabled(numberOfMappings == 1 ? true : false);
	// Set up new profile #...
	ChangeMapName(0);
}


void ControllerTab::ChangeMapName(int selection)
{
	profileNum = mapNameList->itemData(selection).toInt();

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
		controllerWidget->keys[i] = profile[profileNum].map[i];

	controllerWidget->update();
	mapToList->setCurrentIndex(mapToList->findData(profile[profileNum].preferredSlot));
}


void ControllerTab::AddMapName(void)
{
	int freeProfile = GetFreeProfile();

	if (freeProfile == -1)
	{
		// Oh crap, we're out of room! Alert the media!
		QMessageBox::warning(this, tr("Houston, we have a problem..."), tr("Can't create any more profiles!"));

		return;
	}

	QString text = QInputDialog::getText(this, tr("Add Map Name"), tr("Map name:"), QLineEdit::Normal);

	if (text.isEmpty())
		return;

	// Add mapping...
	profileNum = freeProfile;
	profile[profileNum].device = deviceList->itemData(deviceList->currentIndex()).toInt();
	strncpy(profile[profileNum].mapName, text.toUtf8().data(), 31);
	profile[profileNum].mapName[31] = 0;
	profile[profileNum].preferredSlot = CONTROLLER1;

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
		profile[profileNum].map[i] = '*';

	mapNameList->addItem(text, profileNum);
#if 0
	mapNameList->setCurrentIndex(mapNameList->count() - 1);
#else
	int selection = mapNameList->count() - 1;
	mapNameList->setCurrentIndex(selection);
	ChangeMapName(selection);
	// We just added a new mapping, so enable the delete button!
	deleteMapName->setDisabled(false);
#endif
}


void ControllerTab::DeleteMapName(void)
{
	QString msg = QString("Map name: %1\n\nAre you sure you want to remove this mapping?").arg(profile[profileNum].mapName);

//	QMessageBox::StandardButton retVal = QMessageBox::question(this, tr("Remove Mapping"), tr("Are you sure you want to remove this mapping?"), QMessageBox::No | QMessageBox::Yes, QMessageBox::No);
	QMessageBox::StandardButton retVal = QMessageBox::question(this, tr("Remove Mapping"), msg, QMessageBox::No | QMessageBox::Yes, QMessageBox::No);

	if (retVal == QMessageBox::No)
		return;

	int index = mapNameList->currentIndex();
	int profileToRemove = profileNum;
	mapNameList->removeItem(index);
	DeleteProfile(profileToRemove);
	// We need to reload the profile that we move to after deleting the current
	// one...
	ChangeMapName(mapNameList->currentIndex());
	// If we get down to one profile left for the device, we need to make sure
	// that the user can't delete it!
	deleteMapName->setDisabled(mapNameList->count() == 1 ? true : false);
}

