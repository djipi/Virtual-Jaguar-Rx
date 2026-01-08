//
// vjrxprofilerwin.cpp: VJRx Profiler window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When (mm/dd/yy)  What
// ---  ---------------  -----------------------------------------------------------
// JPM  Dec./2025        Created this file
//

#include "profiler/vjrxprofilerwin.h"
#include "settings.h"


// Column indices
#define VJRXPW_COL_FUNCTION			0		// function name, or pointer address
#define VJRXPW_COL_CALLS			1		// call count
#define VJRXPW_COL_MINCYCLES		2		// minimum cycles
#define VJRXPW_COL_MAXCYCLES		3		// maximum cycles
//#define VJRXPW_COL_AVGMS			4		// average milliseconds
#define VJRXPW_COL_MAXMS			4		// maximum milliseconds
#define VJRXPW_COL_COUNT			5		// number of columns


//
VJRxProfilerWindow::VJRxProfilerWindow(QWidget* parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
TableView(new QTableView),
model(new QStandardItemModel),
proxyModel(new QSortFilterProxyModel)
{
	setWindowTitle(tr("VJRx Profiler"));

	// Set the font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);

	// Setup table columns
	model->setColumnCount(VJRXPW_COL_COUNT);
	model->setHeaderData(VJRXPW_COL_FUNCTION, Qt::Horizontal, QObject::tr("Function"));
	model->setHeaderData(VJRXPW_COL_CALLS, Qt::Horizontal, QObject::tr("Calls"));
	model->setHeaderData(VJRXPW_COL_MINCYCLES, Qt::Horizontal, QObject::tr("Min Cycles"));
	model->setHeaderData(VJRXPW_COL_MAXCYCLES, Qt::Horizontal, QObject::tr("Max Cycles"));
	//model->setHeaderData(VJRXPW_COL_AVGMS, Qt::Horizontal, QObject::tr("Avg. ms"));
	model->setHeaderData(VJRXPW_COL_MAXMS, Qt::Horizontal, QObject::tr("Max. ms"));

	// Configure proxy model for sorting
	proxyModel->setSourceModel(model);
	proxyModel->setSortRole(Qt::UserRole);  // Use UserRole for proper numeric sorting

	// Configure table view
	TableView->setModel(proxyModel);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setShowGrid(false);
	TableView->setFont(fixedFont);
	TableView->setSelectionBehavior(QAbstractItemView::SelectRows);
	TableView->setSelectionMode(QAbstractItemView::SingleSelection);
	TableView->setAlternatingRowColors(true);
	TableView->verticalHeader()->setDefaultSectionSize(TableView->verticalHeader()->minimumSectionSize());
	TableView->verticalHeader()->setDefaultAlignment(Qt::AlignRight);
	TableView->verticalHeader()->setVisible(false);
	TableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
	TableView->horizontalHeader()->setStretchLastSection(true);
	TableView->horizontalHeader()->setSectionsClickable(true);

	// Enable sorting
	TableView->setSortingEnabled(true);
	TableView->sortByColumn(VJRXPW_COL_FUNCTION, Qt::DescendingOrder);
	TableView->sortByColumn(VJRXPW_COL_CALLS, Qt::DescendingOrder);
	TableView->sortByColumn(VJRXPW_COL_MINCYCLES, Qt::DescendingOrder);
	TableView->sortByColumn(VJRXPW_COL_MAXCYCLES, Qt::DescendingOrder);
	//TableView->sortByColumn(VJRXPW_COL_AVGMS, Qt::DescendingOrder);
	TableView->sortByColumn(VJRXPW_COL_MAXMS, Qt::DescendingOrder);

	// Layout setup
	QGroupBox* groupBox = new QGroupBox(tr("68000 Statistics"));
	QVBoxLayout* groupLayout = new QVBoxLayout;
	groupLayout->addWidget(TableView);
	groupBox->setLayout(groupLayout);

	layout->addWidget(groupBox);
	setLayout(layout);

	// Set initial window size
	resize(800, 400);
}


// Helper to format numbers with thousands separators
QString VJRxProfilerWindow::FormatThousands(size_t value)
{
	QString s = QString::number(value);
	if (s.length() > 3)
	{
		for (int i = s.length() - 3; i > 0; i -= 3)
			s.insert(i, ",");
	}
	return s;
}


// Update a profiler row window content
void VJRxProfilerWindow::UpdateContent(size_t NumEntry, VJRxProfilerData* Entry)
{
	if (isVisible())
	{
		if (Entry)
		{
			// Ensure enough rows exist or create a new one
			int row = model->rowCount();
			(NumEntry >= row) ? model->insertRow(row) : row = (int)NumEntry;

			// Function name column
			QStandardItem* funcItem = new QStandardItem(Entry->FunctionName ? Entry->FunctionName : "");
			funcItem->setData(Entry->FunctionName ? QString(Entry->FunctionName) : "", Qt::UserRole);
			funcItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_FUNCTION, funcItem);

			// Call count column
			QStandardItem* callsItem = new QStandardItem(QString::number(Entry->CallCount));
			callsItem->setData(QVariant::fromValue(Entry->CallCount), Qt::UserRole);  // For numeric sorting
			callsItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_CALLS, callsItem);

			// minimum cycles column
			QStandardItem* minCyclesItem = new QStandardItem(FormatThousands(Entry->minCycles));
			minCyclesItem->setData(QVariant::fromValue(Entry->minCycles), Qt::UserRole);  // For numeric sorting
			minCyclesItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_MINCYCLES, minCyclesItem);

			// maximum cycles column
			QStandardItem* maxCyclesItem = new QStandardItem(FormatThousands(Entry->maxCycles));
			maxCyclesItem->setData(QVariant::fromValue(Entry->maxCycles), Qt::UserRole);  // For numeric sorting
			maxCyclesItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_MAXCYCLES, maxCyclesItem);

#if 0
			// average milliseconds column
			QStandardItem* avgMillisItem = new QStandardItem(QString::number(Entry->avgms));
			avgMillisItem->setData(QVariant::fromValue(Entry->avgms), Qt::UserRole);  // For numeric sorting
			avgMillisItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_AVGMS, avgMillisItem);
#endif

			// maximum milliseconds column
			QStandardItem* maxMillisItem = new QStandardItem(QString::number(Entry->maxms));
			maxMillisItem->setData(QVariant::fromValue(Entry->maxms), Qt::UserRole);  // For numeric sorting
			maxMillisItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_MAXMS, maxMillisItem);

			// Resize columns to fit content
			TableView->resizeColumnsToContents();
		}
	}
}


// Refresh the profiler window contents
void VJRxProfilerWindow::RefreshContents(size_t NbEntries, VJRxProfilerData* Ptrdata)
{
	if (isVisible())
	{
		// Clear existing data
		model->setRowCount(0);

		if (NbEntries && Ptrdata)
		{
			// Add rows for each profiler entry
			for (size_t i = 0; i < NbEntries; i++)
			{
				VJRxProfilerData* entry = &Ptrdata[i];
				if (entry && entry->FunctionName)
				{
					UpdateContent(i, entry);
				}
			}
		}
	}
}


// Handle key press events
void VJRxProfilerWindow::keyPressEvent(QKeyEvent* e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}


//
VJRxProfilerWindow::~VJRxProfilerWindow(void)
{
}
