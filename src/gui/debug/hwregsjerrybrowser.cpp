//
// hwregsjerrybrowser.cpp: Hardware registers Jerry browser
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM   July/2022  Created this file
//

// STILL TO DO:
// To add the remaining registers
//

#include "hwregsbrowser.h"
#include "jerry.h"

#define INV16(a)	((((a) & 0xff00) >> 8) | (((a) & 0x00ff) << 8))


//
struct JerryInfoTable
{
	const char *RegisterName;
	hwregsaccess Access;
	const char *AccessName;
	unsigned int Offset;
	const char *Name;
}
S_JerryInfoTable;

// Information table for the Jerry
// References: N/A
JerryInfoTable TabJerryInfoTable[] = {
	{ "JPIT1", hwregswo, "WO", 0x0000, "Timer 1 Pre-scaler" },
	{ "JPIT2", hwregswo, "WO", 0x0002, "Timer 1 Divider" },
	{ "JPIT3", hwregswo, "WO", 0x0004, "Timer 2 Pre-scaler" },
	{ "JPIT4", hwregswo, "WO", 0x0006, "Timer 2 Divider" },
	{ "CLK1", hwregswo, "WO", 0x0010, "Processor clock divider" },
	{ "CLK2", hwregswo, "WO", 0x0012, "Video clock divider" },
	{ "CLK3", hwregswo, "WO", 0x0014, "Chroma clock divider" },
	{ "JINTCTRL", hwregsrw, "-W", 0x0020, "Interrupt control Register" },
#if 0
	{ "ASIDATA", hwregsrw, "RW", 0x0030, "Asynchronous Serial Data", "" },
	{ "ASICTRL", hwregswo, "WO", 0x0032, "Asynchronous Serial Control", "" },
	{ "ASISTAT", hwregsro, "RO", 0x0032, "Asynchronous Serial Status", "" },
	{ "ASICLK", hwregsrw, "RW", 0x0034, "Asynchronous Serial Interface Clock", "" },
	{ "JPIT1", hwregsro, "RO", 0x0036, "Timer 1 Pre-scaler", "." },
	{ "JPIT2", hwregsro, "RO", 0x0038, "Timer 1 Divider", "" },
	{ "JPIT3", hwregsro, "RO", 0x003A, "Timer 2 Pre-scaler", "" },
	{ "JPIT4", hwregsro, "RO", 0x003C, "Timer 2 Divider", "" }
#endif
};


// 
HWRegsJerryBrowserWindow::HWRegsJerryBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
TableView(new QTableView),
model(new QStandardItemModel)
{
	unsigned int i;

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

	// Set the new layout with proper identation and readibility
	model->setColumnCount(5);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Register"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Access"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Name"));
	model->setHeaderData(3, Qt::Horizontal, QObject::tr("Address"));
	model->setHeaderData(4, Qt::Horizontal, QObject::tr("Value"));
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(0);
	TableView->setFont(fixedFont);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);

	// Set basic infos in the layout
	for (i = 0; i < (sizeof(TabJerryInfoTable) / sizeof(struct JerryInfoTable)); i++)
	{
		model->insertRow(i);
		model->setItem(i, 0, new QStandardItem(QString("%1").arg(TabJerryInfoTable[i].RegisterName)));
		model->setItem(i, 1, new QStandardItem(QString("%1").arg(TabJerryInfoTable[i].AccessName)));
		model->setItem(i, 2, new QStandardItem(QString("%1").arg(TabJerryInfoTable[i].Name)));
		model->setItem(i, 3, new QStandardItem(QString("0xf1%1").arg(TabJerryInfoTable[i].Offset, 4, 16, QChar('0'))));
	}

	// Set the layout
	layout->addWidget(TableView);
	setLayout(layout);
}


//
HWRegsJerryBrowserWindow::~HWRegsJerryBrowserWindow(void)
{
	Reset();
}


//
void HWRegsJerryBrowserWindow::Reset(void)
{
}


// Display the value from each Jerry's register
void HWRegsJerryBrowserWindow::RefreshContents(void)
{
	char string[1024];
	unsigned int i;
	uint16_t value;

	if (isVisible())
	{
		for (i = 0; i < (sizeof(TabJerryInfoTable) / sizeof(struct JerryInfoTable)); i++)
		{
			// Emulator handles the Jerry in a separate array
			value = *(uint16_t*)&jerry_ram_8[TabJerryInfoTable[i].Offset];
			sprintf(string, "0x%04x", INV16(value));
			model->setItem(i, 4, new QStandardItem(QString("%1").arg(string)));
		}
	}
}
