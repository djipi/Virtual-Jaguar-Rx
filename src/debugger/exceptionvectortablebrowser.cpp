//
// exceptionvectortable.cpp - Exception Vector Table
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  05/09/2017  Created this file
// JPM  09/09/2018  Set information values in a tab
// JPM  09/09/2018  Added vectors in the table
//

// STILL TO DO:
// To set the information display at the right
//


#include "debugger/exceptionvectortablebrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"


//
struct ExceptionVectorTable
{
	size_t VectorNumber;
	size_t Address;
	const char *ExceptionName;
}
S_ExceptionVectorTable;

//
ExceptionVectorTable	TabExceptionVectorTable[] =	{
														{ 0, 0x000000, "RESET - Initial SSP" },
														{ 1, 0x000004, "RESET - Initial PC" },
														{ 2, 0x000008, "Bus error" },
														{ 3, 0x00000C, "Address error" },
														{ 4, 0x000010, "Illegal instruction" },
														{ 5, 0x000014, "Division by zero" },
														{ 6, 0x000018, "CHK instruction" },
														{ 7, 0x00001C, "TRAPV instruction" },
														{ 8, 0x000020, "Privilege violation" },
														{ 9, 0x000024, "Trace" },
														{ 10, 0x000028, "Unimplemented instruction"	},
														{ 11, 0x00002C, "Unimplemented instruction"	},
														{ 12, 0x000030, "Reserved by Motorola" },
														{ 13, 0x000034, "Reserved by Motorola" },
														{ 14, 0x000038, "Reserved by Motorola" },
														{ 15, 0x00003C, "Uninitialised interrupt vector" },
														{ 16, 0x000040, "Reserved by Motorola" },
														{ 17, 0x000044, "Reserved by Motorola" },
														{ 18, 0x000048, "Reserved by Motorola" },
														{ 19, 0x00004C, "Reserved by Motorola" },
														{ 20, 0x000050, "Reserved by Motorola" },
														{ 21, 0x000054, "Reserved by Motorola" },
														{ 22, 0x000058, "Reserved by Motorola" },
														{ 23, 0x00005C, "Reserved by Motorola" },
														{ 24, 0x000060, "Spurious interrupt" },
														{ 25, 0x000064, "Level 1 interrupt autovector" },
														{ 26, 0x000068, "Level 2 interrupt autovector" },
														{ 27, 0x00006C, "Level 3 interrupt autovector" },
														{ 28, 0x000070, "Level 4 interrupt autovector" },
														{ 29, 0x000074, "Level 5 interrupt autovector" },
														{ 30, 0x000078, "Level 6 interrupt autovector" },
														{ 31, 0x00007C, "Level 7 interrupt autovector" },
														{ 32, 0x000080, "TRAP #0 instruction" },
														{ 33, 0x000084, "TRAP #1 instruction" },
														{ 34, 0x000088, "TRAP #2 instruction" },
														{ 35, 0x00008C, "TRAP #3 instruction" },
														{ 36, 0x000090, "TRAP #4 instruction" },
														{ 37, 0x000094, "TRAP #5 instruction" },
														{ 38, 0x000098, "TRAP #6 instruction" },
														{ 39, 0x00009C, "TRAP #7 instruction" },
														{ 40, 0x0000A0, "TRAP #8 instruction" },
														{ 41, 0x0000A4, "TRAP #9 instruction" },
														{ 42, 0x0000A8, "TRAP #10 instruction" },
														{ 43, 0x0000AC, "TRAP #11 instruction" },
														{ 44, 0x0000B0, "TRAP #12 instruction" },
														{ 45, 0x0000B4, "TRAP #13 instruction" },
														{ 46, 0x0000B8, "TRAP #14 instruction" },
														{ 47, 0x0000BC, "TRAP #15 instruction" },
														{ 48, 0x0000C0, "Reserved by Motorola" },
														{ 49, 0x0000C4, "Reserved by Motorola" },
														{ 50, 0x0000C8, "Reserved by Motorola" },
														{ 51, 0x0000CC, "Reserved by Motorola" },
														{ 52, 0x0000D0, "Reserved by Motorola" },
														{ 53, 0x0000D4, "Reserved by Motorola" },
														{ 54, 0x0000D8, "Reserved by Motorola" },
														{ 55, 0x0000DC, "Reserved by Motorola" },
														{ 56, 0x0000E0, "Reserved by Motorola" },
														{ 57, 0x0000E4, "Reserved by Motorola" },
														{ 58, 0x0000E8, "Reserved by Motorola" },
														{ 59, 0x0000EC, "Reserved by Motorola" },
														{ 60, 0x0000F0, "Reserved by Motorola" },
														{ 61, 0x0000F4, "Reserved by Motorola" },
														{ 62, 0x0000F8, "Reserved by Motorola" },
														{ 63, 0x0000FC, "Reserved by Motorola" },
														{ 64, 0x000100, "User interrupt vectors" },
														{ 65, 0x000104, "User interrupt vectors" },
														{ 66, 0x000108, "User interrupt vectors" },
														{ 67, 0x00010C, "User interrupt vectors" },
														{ 68, 0x000200, "User interrupt vectors" },
														{ 69, 0x000204, "User interrupt vectors" },
														{ 70, 0x000208, "User interrupt vectors" },
														{ 71, 0x00020C, "User interrupt vectors" },
														{ 72, 0x000210, "User interrupt vectors" },
														{ 73, 0x000214, "User interrupt vectors" },
														{ 74, 0x000218, "User interrupt vectors" },
														{ 75, 0x00021C, "User interrupt vectors" },
														{ 252, 0x0003F0, "User interrupt vectors" },
														{ 253, 0x0003F4, "User interrupt vectors" },
														{ 254, 0x0003F8, "User interrupt vectors" },
														{ 255, 0x0003FC, "User interrupt vectors" }
													};


// 
ExceptionVectorTableBrowserWindow::ExceptionVectorTableBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
#ifdef EV_LAYOUTTEXTS
text(new QTextBrowser),
#else
TableView (new QTableView),
model (new QStandardItemModel),
#endif
refresh(new QPushButton(tr("Refresh")))
{
	setWindowTitle(tr("Exception Vector Table"));

	// Refresh feature
	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->addWidget(refresh);

	// Setup font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

#ifdef EV_LAYOUTTEXTS
	text->setFont(fixedFont);
	layout->addWidget(text);
#else
	// Set the new layout with proper identation and readibility
	model->setColumnCount(3);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Vector"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Pointer"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Use"));
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(0);
	TableView->setFont(fixedFont);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	layout->addWidget(TableView);
#endif

	layout->addWidget(refresh);
	setLayout(layout);

	// Event setup
	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
}


//
ExceptionVectorTableBrowserWindow::~ExceptionVectorTableBrowserWindow(void)
{
}


//
void ExceptionVectorTableBrowserWindow::RefreshContents(void)
{
	char string[1024];
#ifdef EV_LAYOUTTEXTS
	QString ExceptionVector;
#endif
	size_t i;

	if (isVisible())
	{
#ifndef HA_LAYOUTTEXTS
		model->setRowCount(0);
#endif
		for (i = 0; i < (sizeof(TabExceptionVectorTable) / sizeof(ExceptionVectorTable)); i++)
		{
#ifdef EV_LAYOUTTEXTS
			if (i)
			{
				ExceptionVector += QString("<br>");
			}
			sprintf(string, "%03i : 0x%06X | 0x%06X | %s", (unsigned int)TabExceptionVectorTable[i].VectorNumber, (unsigned int)TabExceptionVectorTable[i].Address, GET32(jaguarMainRAM, TabExceptionVectorTable[i].Address), TabExceptionVectorTable[i].ExceptionName);
			ExceptionVector += QString(string);
#else
			model->insertRow(i);
			model->setItem(i, 0, new QStandardItem(QString("0x%1").arg(TabExceptionVectorTable[i].Address, 4, 16, QChar('0'))));
			sprintf(string, "0x%06x", (unsigned int)GET32(jaguarMainRAM, TabExceptionVectorTable[i].Address));
			model->setItem(i, 1, new QStandardItem(QString("%1").arg(string)));
			model->setItem(i, 2, new QStandardItem(QString("%1").arg(TabExceptionVectorTable[i].ExceptionName)));
#endif
		}
#ifdef EV_LAYOUTTEXTS
		text->clear();
		text->setText(ExceptionVector);
#endif
	}
}


// 
void ExceptionVectorTableBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}
