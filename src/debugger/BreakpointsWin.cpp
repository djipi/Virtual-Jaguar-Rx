//
// BreakpointsWin.cpp - Breakpoints
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  mm/dd/yyyy  What
// ---  ----------  -----------------------------------------------------------
// JPM  30/08/2017  Created this file
// JPM   Oct./2018  Added the breakpoints features
// JPM    May/2026  Added breakpoint deletion and on/off toggle from the breakpoints window
// JPM   July/2026  Added columns for the access & type of the breakpoint
//

// STILL TO DO:
//

#include "debugger/BreakpointsWin.h"
#include "jaguar.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"


//
BreakpointsWindow::BreakpointsWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
TableView(new QTableView),
model(new QStandardItemModel),
#ifdef BRK_STATUSBAR
statusbar(new QStatusBar),
#endif
#ifdef BRK_REFRESHBUTTON
refresh(new QPushButton(tr("Refresh"))),
#endif
layout(new QVBoxLayout)
{
	setWindowTitle(tr("Breakpoints"));

	// Refresh feature
#ifdef BRK_REFRESH
	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->addWidget(refresh);
#endif

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

	// Set the new layout with proper identation and readibility
	model->setColumnCount(5);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Status"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Address"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Hit Count"));
	model->setHeaderData(3, Qt::Horizontal, QObject::tr("Type"));
	model->setHeaderData(4, Qt::Horizontal, QObject::tr("Access"));

	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(0);
	TableView->setFont(fixedFont);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	layout->addWidget(TableView);

	// Status bar
#ifdef BRK_STATUSBAR
	layout->addWidget(statusbar);
#endif
	// Set layouts
#ifdef BRK_REFRESHBUTTON
	layout->addLayout(hbox1);
#endif	
	setLayout(layout);
	// Event setup
#ifdef BRK_REFRESHBUTTON
	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
#endif
	connect(TableView, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(OnTableDoubleClicked(const QModelIndex &)));
}


//
BreakpointsWindow::~BreakpointsWindow(void)
{
}


// 
void BreakpointsWindow::UpdateInfos(void)
{
	char *FuncName;
	bool ok;
	char Addresse[100];

	// Display the BPM as first breakpoint
	model->setItem(0, 0, new QStandardItem(QString("%1").arg(bpmSaveActive ? "BPM On" : "BPM Off")));
	if (bpmAddress1)
	{
		sprintf(Addresse, "0x%06X", bpmAddress1);
	}
	else
	{
		strcpy(Addresse, "(null)");
	}
	model->setItem(0, 1, new QStandardItem(QString("%1").arg((bpmAddress1 && (FuncName = DBGManager_GetSymbolNameFromAdr(bpmAddress1))) ? FuncName : Addresse)));
	model->setItem(0, 2, new QStandardItem(QString("%1").arg(bpmHitCounts)));

	// Display all user breakpoints
	for (size_t i = 0; i < brkNbr; i++)
	{
		if (brkInfo[i].Used)
		{
			model->setItem((i + 1), 0, new QStandardItem(QString("%1").arg(brkInfo[i].Active ? "On" : "Off")));
			sprintf(Addresse, "0x%06X", brkInfo[i].Adr);
			model->setItem((i + 1), 1, new QStandardItem(QString("%1").arg((FuncName = brkInfo[i].Name) ? FuncName : Addresse)));
			model->setItem((i + 1), 2, new QStandardItem(QString("%1").arg(brkInfo[i].HitCounts)));
			model->setItem((i + 1), 3, new QStandardItem(QString("%1").arg(brkInfo[i].IsCode ? "Code" : "Data")));
			model->setItem((i + 1), 4, new QStandardItem(QString("%1").arg((brkInfo[i].Access == 1) ? "Read" : ((brkInfo[i].Access == 2) ? "Write" : "Read/Write"))));
		}
	}
}


// 
void BreakpointsWindow::Reset(void)
{
	UpdateTable(true);
}


// 
void BreakpointsWindow::UpdateTable(bool refresh)
{
	if (refresh)
	{
		model->setRowCount(0);
		model->insertRow(brkNbr + 1);
	}
}


//
void BreakpointsWindow::RefreshContents(void)
{
	if (isVisible())
	{
		UpdateTable(true);
		UpdateInfos();
	}
}


// Toggle the breakpoint status on double-click on the status cell
void BreakpointsWindow::OnTableDoubleClicked(const QModelIndex& index)
{
	// the row 0 is the BPM entry and cannot be toggled; only column 0 (Status) is actionable
	if ((index.row() > 0) && (index.column() == 0))
	{
		size_t brkIndex = (size_t)(index.row() - 1);
		if (brkInfo[brkIndex].Used)
		{
			m68k_brk_toggle_status((unsigned int)index.row());
			model->setItem(index.row(), 0, new QStandardItem(QString("%1").arg(brkInfo[brkIndex].Active ? "On" : "Off")));
		}
	}
}


// Handle key events for the breakpoints window
void BreakpointsWindow::keyPressEvent(QKeyEvent * e)
{
	// close the window on Escape key press
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		// delete the selected breakpoint on Delete key press
		if (e->key() == Qt::Key_Delete)
		{
			QModelIndexList selection = TableView->selectionModel()->selectedRows();
			if (!selection.isEmpty())
			{
				// the row 0 is the BPM entry and cannot be deleted
				int row = selection.first().row();
				if (row > 0)
				{
					m68k_brk_del((unsigned int)row);
					RefreshContents();
				}
			}
		}
	}
}
