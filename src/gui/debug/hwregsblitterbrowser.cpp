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
//

// STILL TO DO:
//

#include "hwregsbrowser.h"
#include "blitter.h"


//
struct BlitterInfoTable
{
	unsigned int Address;
	unsigned int NbBits;
	const char *Name;
	const char *Type;
}
S_BlitterInfoTable;

//
BlitterInfoTable TabBlitterInfoTable[] = {
											{	0xF02200, sizeof(long), "A1 base address", "32-bit register containing a pointer to the base of the window pointer to by A1.\nThis address must be phrase aligned."	},
											{	0xF02204, sizeof(long), "Flags Register", "A set of flags controlling various aspects of the A1 window and how addresses are updated."	},
											{	0xF02208, sizeof(long), "A1 Clipping Window Size", "This register contains the size in pixels, and may be used for clipping writes, so that if the pointer leaves the window bounds no write is performed."	},
											{	0xF0220C, sizeof(long), "A1 Window Pixel Pointer", "This register contains the X (low word) and Y (high word) pointers onto the window, and are the location where the next pixel will be written.\nThey are sixteen - bit signed values."	},
											{	0xF02210, sizeof(long), "A1 Step Value", "The step register contains two signed sixteen bit values, which are the X step (low word) and Y step (high word)."	},
											{	0xF02214, sizeof(long), "A1 Step Fraction Value", "The step fraction register may be added to the fractional parts of the A1 pointer in the same manner as the step value."	},
											{	0xF02218, sizeof(long), "A1 Window Pixel Pointer Fraction", ""	},
											{	0xF0221C, sizeof(long), "A1 Pixel Pointer Increment", ""	},
											{	0xF02220, sizeof(long), "A1 Pixel Pointer Increment Fraction", "This is the fractional parts of the increment described above."	},
											{	0xF02224, sizeof(long), "A2 Base Register", "32-bit register containing a pointer to the base of the window pointer to by A2.\nThis address must be phrase aligned."	},
											{	0xF02228, sizeof(long), "A2 Flags Register", "A set of flags controlling various aspects of the A2 window and how addresses are updated."	},
											{	0xF0222C, sizeof(long), "A2 Window Mask", ""	},
											{	0xF02230, sizeof(long), "A2 Window Pointer", ""	},
											{	0xF02234, sizeof(long), "A2 Step Value", ""	},
											{	0xF02238, sizeof(long), "Status Register", ""	},
											{	0xF0223C, sizeof(long), "Counters Register", ""	},
											{	0xF02240, sizeof(long long), "Source Data Register", ""	},
											{	0xF02248, sizeof(long long), "Destination Data Register", ""	},
											{	0xF02250, sizeof(long long), "Destination Z Register", ""	},
											{	0xF02258, sizeof(long long), "Source Z Register 1", "The source Z register 1 is also used to hold the four integer parts of computed Z."	},
											{	0xF02260, sizeof(long long), "Source Z Register 2", "The source Z register 2 is also used to hold the four fraction parts of computed Z."	},
											{	0xF02268, sizeof(long long), "Pattern Data Register", "The pattern data register also serves to hold the computed intensity integer parts and their associated colours."	},
											{	0xF02270, sizeof(long), "Intensity Increment", "This thirty-two bit register holds the integer and fractional parts of the intensity increment used for Gouraud shading."	},
											{	0xF02274, sizeof(long), "Z Increment", "This thirty-two bit register holds the integer and fractional parts of the Z increment used for computed Z polygon drawing."	},
											{	0xF02278, sizeof(long), "Collision control", ""	},
											{	0xF0227C, sizeof(long), "Intensity 0", ""	},
											{	0xF02280, sizeof(long), "Intensity 1", ""	},
											{	0xF02284, sizeof(long), "Intensity 2", ""	},
											{	0xF02288, sizeof(long), "Intensity 3", ""	},
											{	0xF0228C, sizeof(long), "Z 0", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
											{	0xF02290, sizeof(long), "Z 1", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
											{	0xF02294, sizeof(long), "Z 2", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
											{	0xF02298, sizeof(long), "Z 3", "These registers are analogous to the intensity registers, and are for Z buffer operation."	},
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
	model->setColumnCount(4);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Name"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Address"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("# bits"));
	model->setHeaderData(3, Qt::Horizontal, QObject::tr("Value"));
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
		model->setItem(i, 0, new QStandardItem(QString("%1").arg(TabBlitterInfoTable[i].Name)));
		model->setItem(i, 1, new QStandardItem(QString("0x%1").arg(TabBlitterInfoTable[i].Address, 4, 16, QChar('0'))));
		model->setItem(i, 2, new QStandardItem(QString("%1").arg(TabBlitterInfoTable[i].NbBits * 8)));
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


//
void HWRegsBlitterBrowserWindow::RefreshContents(void)
{
	char string[1024];
	unsigned int i;

	if (isVisible())
	{
		for (i = 0; i < (sizeof(TabBlitterInfoTable) / sizeof(struct BlitterInfoTable)); i++)
		{
			// Emulator handles the blitter in a separate array
			sprintf(string, "0x%08x", BlitterReadLong(TabBlitterInfoTable[i].Address));
			model->setItem(i, 3, new QStandardItem(QString("%1").arg(string)));
		}
	}
}
