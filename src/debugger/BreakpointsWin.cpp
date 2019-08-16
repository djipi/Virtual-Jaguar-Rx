//
// BreakpointsWin.cpp - Breakpoints
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  30/08/2017  Created this file
// JPM   Oct./2018  Added the breakpoints features
//

// STILL TO DO:
//

#include "debugger/BreakpointsWin.h"
#include "jaguar.h"
#include "debugger/DBGManager.h"


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
	model->setColumnCount(3);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("Status"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("Name"));
#ifdef BRK_HITCOUNTS
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("Hit Count"));
#endif
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
#ifdef BRK_HITCOUNTS
	model->setItem(0, 2, new QStandardItem(QString("%1").arg(bpmHitCounts)));
#endif

	// Display all user breakpoints
	for (size_t i = 0; i < brkNbr; i++)
	{
		if (brkInfo[i].Used)
		{
			model->setItem((i + 1), 0, new QStandardItem(QString("%1").arg(brkInfo[i].Active ? "On" : "Off")));
			sprintf(Addresse, "0x%06X", brkInfo[i].Adr);
			model->setItem((i + 1), 1, new QStandardItem(QString("%1").arg((FuncName = brkInfo[i].Name) ? FuncName : Addresse)));
			model->setItem((i + 1), 2, new QStandardItem(QString("%1").arg(brkInfo[i].HitCounts)));
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


//
void BreakpointsWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}

