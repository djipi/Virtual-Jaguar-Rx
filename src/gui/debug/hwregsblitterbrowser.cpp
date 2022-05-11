//
// hwregsbrowser.h: Hardware registers blitter browser
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  08/20/2019  Created this file
// JPM   Oct./2021  Added the register's name
// JPM   Jan./2022  Fix for the 64bits values
//

// STILL TO DO:
//

#include "hwregsbrowser.h"
#include "blitter.h"

#define INV64(a)	(((a) >> 56) | (((a) & 0x00ff000000000000) >> 40) | (((a) & 0x0000ff0000000000) >> 24) | (((a) & 0x000000ff00000000) >> 8 ) | (((a) & 0x00000000ff000000) << 8 ) | (((a) & 0x0000000000ff0000) << 24) | (((a) & 0x000000000000ff00) << 40) | (((a) & 0x00000000000000ff) << 56))
#define INV32(a)	((((a) & 0xff000000) >> 24) | (((a) & 0x00ff0000) >> 8) | (((a) & 0x0000ff00) << 8) | (((a) & 0xff) << 24)))

extern uint8_t blitter_ram[];


//
struct BlitterInfoTable
{
	const char *RegisterName;
	unsigned int Address;
	unsigned int NbBytes;
	const char *Name;
	const char *Type;
}
S_BlitterInfoTable;

// Information table for the Blitter
// References: Jaguar Software Reference Manual - Version 2.4
BlitterInfoTable TabBlitterInfoTable[] = {
	{	"A1_BASE", 0xF02200, sizeof(uint32_t), "A1 Base Register", "32-bit register containing a pointer to the base of the window pointer to by A1.\nThis address must be phrase aligned."	},
	{	"A1_FLAGS", 0xF02204, sizeof(uint32_t), "A1 Flags Register", "A set of flags controlling various aspects of the A1 window and how addresses are updated."	},
	{	"A1_CLIP", 0xF02208, sizeof(uint32_t), "A1 Clipping Size", "This register contains the size in pixels, and may be used for clipping writes, so that if the pointer leaves the window bounds no write is performed."	},
	{	"A1_PIXEL", 0xF0220C, sizeof(uint32_t), "A1 Pixel Pointer", "This register contains the X (low word) and Y (high word) pointers onto the window, and are the location where the next pixel will be written.\nThey are sixteen - bit signed values."	},
	{	"A1_STEP", 0xF02210, sizeof(uint32_t), "A1 Step Value", "The step register contains two signed sixteen bit values, which are the X step (low word) and Y step (high word)."	},
	{	"A1_FSTEP", 0xF02214, sizeof(uint32_t), "A1 Step Fraction Value", "The step fraction register may be added to the fractional parts of the A1 pointer in the same manner as the step value."	},
	{	"A1_FPIXEL", 0xF02218, sizeof(uint32_t), "A1 Pixel Pointer Fraction", ""	},
	{	"A1_INC", 0xF0221C, sizeof(uint32_t), "A1 Increment", ""	},
	{	"A1_FINC", 0xF02220, sizeof(uint32_t), "A1 Increment Fraction", "This is the fractional parts of the increment described above."	},
	{	"A2_BASE", 0xF02224, sizeof(uint32_t), "A2 Base Register", "32-bit register containing a pointer to the base of the window pointer to by A2.\nThis address must be phrase aligned."	},
	{	"A2_FLAGS", 0xF02228, sizeof(uint32_t), "A2 Flags Register", "A set of flags controlling various aspects of the A2 window and how addresses are updated."	},
	{	"A2_MASK", 0xF0222C, sizeof(uint32_t), "A2 Window Mask", ""	},
	{	"A2_PIXEL", 0xF02230, sizeof(uint32_t), "A2 Pixel Pointer", ""	},
	{	"A2_STEP", 0xF02234, sizeof(uint32_t), "A2 Step Value", ""	},
	{	"B_CMD", 0xF02238, sizeof(uint32_t), "Command Register", ""	},
	{	"B_COUNT", 0xF0223C, sizeof(uint32_t), "Counters Register", ""	},
	{	"B_SRCD", 0xF02240, sizeof(uint64_t), "Source Data Register", ""	},
	{	"B_DSTD", 0xF02248, sizeof(uint64_t), "Destination Data Register", ""	},
	{	"B_DSTZ", 0xF02250, sizeof(uint64_t), "Destination Z Register", ""	},
	{	"B_SRCZ1", 0xF02258, sizeof(uint64_t), "Source Z Register 1", "The source Z register 1 is also used to hold the four integer parts of computed Z."	},
	{	"B_SRCZ2", 0xF02260, sizeof(uint64_t), "Source Z Register 2", "The source Z register 2 is also used to hold the four fraction parts of computed Z."	},
	{	"B_PATD", 0xF02268, sizeof(uint64_t), "Pattern Data Register", "The pattern data register also serves to hold the computed intensity integer parts and their associated colours."	},
	{	"B_IINC", 0xF02270, sizeof(uint32_t), "Intensity Increment", "This thirty-two bit register holds the integer and fractional parts of the intensity increment used for Gouraud shading."	},
	{	"B_ZINC", 0xF02274, sizeof(uint32_t), "Z Increment", "This thirty-two bit register holds the integer and fractional parts of the Z increment used for computed Z polygon drawing."	},
	{	"B_STOP", 0xF02278, sizeof(uint32_t), "Collision control", ""	},
	{	"B_I3", 0xF0227C, sizeof(uint32_t), "Intensity 3", ""	},
	{	"B_I2", 0xF02280, sizeof(uint32_t), "Intensity 2", ""	},
	{	"B_I1", 0xF02284, sizeof(uint32_t), "Intensity 1", ""	},
	{	"B_I0", 0xF02288, sizeof(uint32_t), "Intensity 0", ""	},
	{	"B_Z3", 0xF0228C, sizeof(uint32_t), "Z3", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
	{	"B_Z2", 0xF02290, sizeof(uint32_t), "Z2", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
	{	"B_Z1", 0xF02294, sizeof(uint32_t), "Z1", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
	{	"B_Z0", 0xF02298, sizeof(uint32_t), "Z0", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
};


// 
HWRegsBlitterBrowserWindow::HWRegsBlitterBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
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
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Address"));
	model->setHeaderData(3, Qt::Horizontal, QObject::tr("# bits"));
	model->setHeaderData(4, Qt::Horizontal, QObject::tr("Value"));
	//model->setHeaderData(4, Qt::Horizontal, QObject::tr("Reference"));
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(0);
	TableView->setFont(fixedFont);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);

	// Set basic infos in the layout
	for (i = 0; i < (sizeof(TabBlitterInfoTable) / sizeof(struct BlitterInfoTable)); i++)
	{
		model->insertRow(i);
		model->setItem(i, 0, new QStandardItem(QString("%1").arg(TabBlitterInfoTable[i].RegisterName)));
		model->setItem(i, 1, new QStandardItem(QString("%1").arg(TabBlitterInfoTable[i].Name)));
		model->setItem(i, 2, new QStandardItem(QString("0x%1").arg(TabBlitterInfoTable[i].Address, 4, 16, QChar('0'))));
		model->setItem(i, 3, new QStandardItem(QString("%1").arg(TabBlitterInfoTable[i].NbBytes * 8)));
		//model->setItem(i, 4, new QStandardItem(QString("%1").arg(TabBlitterInfoTable[i].Type)));
	}

	// Set the layout
	layout->addWidget(TableView);
	setLayout(layout);
}


//
HWRegsBlitterBrowserWindow::~HWRegsBlitterBrowserWindow(void)
{
	Reset();
}


//
void HWRegsBlitterBrowserWindow::Reset(void)
{
}


// Display the value from each Blitter's register
void HWRegsBlitterBrowserWindow::RefreshContents(void)
{
	char string[1024];
	unsigned int i;
	/* ULONGLONG */ unsigned long long value;

	if (isVisible())
	{
		for (i = 0; i < (sizeof(TabBlitterInfoTable) / sizeof(struct BlitterInfoTable)); i++)
		{
			// Emulator handles the blitter in a separate array
			//sprintf(string, "0x%08x", BlitterReadLong(TabBlitterInfoTable[i].Address));
			(TabBlitterInfoTable[i].NbBytes == 4) ? (value = *(uint32_t*)&blitter_ram[TabBlitterInfoTable[i].Address & 0xff]) : (value = *(uint64_t*)&blitter_ram[TabBlitterInfoTable[i].Address & 0xff]);
			(TabBlitterInfoTable[i].NbBytes == 4) ? sprintf(string, "0x%08x", INV32(value) : sprintf(string, "0x%016x", INV64(value));
			model->setItem(i, 4, new QStandardItem(QString("%1").arg(string)));
		}
	}
}
