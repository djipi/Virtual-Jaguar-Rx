//
// vjrxprofilerwin.cpp: VJRx Profiler window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  mm/dd/yyyy  What
// ---  ----------  -----------------------------------------------------------
// JPM   Dec./2025  Created this file
// JPM   Oct./2026  Added CSV and Callgrind export features
//

#include <stdio.h>
#include "profiler/vjrxprofilerwin.h"
#include "settings.h"


// Column indices
#define VJRXPW_COL_FUNCTION		0		// function name, or pointer address (on string format)
#define VJRXPW_COL_CALLS		1		// call count
#define VJRXPW_COL_MINCYCLES	2		// minimum cycles
#define VJRXPW_COL_MAXCYCLES	3		// maximum cycles
#if 0
#define VJRXPW_COL_AVGMS		4		// average milliseconds
#define VJRXPW_COL_MAXMS		4		// maximum milliseconds
#endif
#define VJRXPW_COL_FILENAME		4		// source file name
#define VJRXPW_COL_LINENUMBER	5		// line number in the source file
// number of columns
#define VJRXPW_COL_COUNT		6


// Constructor
VJRxProfilerWindow::VJRxProfilerWindow(QWidget* parent/*= 0*/) : QMainWindow(parent),
TableView(new QTableView),
model(new QStandardItemModel),
proxyModel(new QSortFilterProxyModel),
menuBar(nullptr),
fileMenu(nullptr),
exportMenu(nullptr)
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
#if 0
	model->setHeaderData(VJRXPW_COL_AVGMS, Qt::Horizontal, QObject::tr("Avg. ms"));
	model->setHeaderData(VJRXPW_COL_MAXMS, Qt::Horizontal, QObject::tr("Max. ms"));
#endif
	model->setHeaderData(VJRXPW_COL_FILENAME, Qt::Horizontal, QObject::tr("Source File"));
	model->setHeaderData(VJRXPW_COL_LINENUMBER, Qt::Horizontal, QObject::tr("Line Number"));

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
#if 0
	TableView->sortByColumn(VJRXPW_COL_AVGMS, Qt::DescendingOrder);
	TableView->sortByColumn(VJRXPW_COL_MAXMS, Qt::DescendingOrder);
#endif
	TableView->sortByColumn(VJRXPW_COL_FILENAME, Qt::DescendingOrder);
	TableView->sortByColumn(VJRXPW_COL_LINENUMBER, Qt::DescendingOrder);

	// Create menu bar
	menuBar = new QMenuBar(this);
	setMenuBar(menuBar);
	// File menu
	fileMenu = menuBar->addMenu(tr("&File"));
	// Export submenu
	exportMenu = fileMenu->addMenu(tr("&Export"));
	// Export to CSV connection
	QAction* exportCSVAction = exportMenu->addAction(tr("Export to CSV (.csv)"));
	connect(exportCSVAction, &QAction::triggered, this, &VJRxProfilerWindow::OnExportCSV);
	// Export to Callgrind connection
	QAction* exportCallgrindAction = exportMenu->addAction(tr("Export to Callgrind (.callgrind)"));
	connect(exportCallgrindAction, &QAction::triggered, this, &VJRxProfilerWindow::OnExportCallgrind);

	// Layout setup
	QWidget* centralWidget = new QWidget(this);
	QVBoxLayout* layout = new QVBoxLayout(centralWidget);

	QGroupBox* groupBox = new QGroupBox(tr("68000 flat profiling"));
	QVBoxLayout* groupLayout = new QVBoxLayout;
	groupLayout->addWidget(TableView);
	groupBox->setLayout(groupLayout);

	layout->addWidget(groupBox);
	centralWidget->setLayout(layout);

	setCentralWidget(centralWidget);

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

			// maximum milliseconds column
			QStandardItem* maxMillisItem = new QStandardItem(QString::number(Entry->maxms));
			maxMillisItem->setData(QVariant::fromValue(Entry->maxms), Qt::UserRole);  // For numeric sorting
			maxMillisItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_MAXMS, maxMillisItem);
#endif
			// source file name column
			QStandardItem* fileItem = new QStandardItem(Entry->SourceFileName ? Entry->SourceFileName : "");
			fileItem->setData(Entry->SourceFileName ? QString(Entry->SourceFileName) : "", Qt::UserRole);
			fileItem->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_FILENAME, fileItem);

			// line number column
			QStandardItem* lineItem = new QStandardItem(Entry->LineNumber ? QString::number(Entry->LineNumber) : "");
			lineItem->setData(QVariant::fromValue(Entry->LineNumber), Qt::UserRole);  // For numeric sorting
			lineItem->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
			model->setItem(row, VJRXPW_COL_LINENUMBER, lineItem);

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
	// hide the window on Escape key press
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
}


// Handle export to CSV
void VJRxProfilerWindow::OnExportCSV()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export to CSV"), "", tr("CSV Files (*.csv);;All Files (*)"));
	if (!fileName.isEmpty())
	{
		// Open the file for writing
		FILE* f = fopen(fileName.toStdString().c_str(), "w");
		if (f)
		{
			// Write the CSV's header
			fprintf(f, "Function,Source File,Line Number,# Calls,Cycles_Min,Cycles_Max\n");
			// Write each row of data
			for (int row = 0; row < model->rowCount(); ++row)
			{
				QString functionName = model->item(row, VJRXPW_COL_FUNCTION)->data(Qt::UserRole).toString();
				QString sourceFileName = model->item(row, VJRXPW_COL_FILENAME)->data(Qt::UserRole).toString();
				size_t lineNumber = model->item(row, VJRXPW_COL_LINENUMBER)->data(Qt::UserRole).toULongLong();
				size_t callCount = model->item(row, VJRXPW_COL_CALLS)->data(Qt::UserRole).toULongLong();
				size_t minCycles = model->item(row, VJRXPW_COL_MINCYCLES)->data(Qt::UserRole).toULongLong();
				size_t maxCycles = model->item(row, VJRXPW_COL_MAXCYCLES)->data(Qt::UserRole).toULongLong();
				// Write the row to the CSV file
				char buf[32];
				lineNumber ? snprintf(buf, sizeof(buf), "%zu", lineNumber) : buf[0] = '\0';
				fprintf(f, "\"%s\",%s,%s,%llu,%llu,%llu\n", functionName.toStdString().c_str(), sourceFileName.toStdString().c_str(), buf, callCount, minCycles, maxCycles);
			}

			// close the file
			fclose(f);
		}
	}
}


// Handle export to Callgrind
void VJRxProfilerWindow::OnExportCallgrind()
{
	QString fileName = QFileDialog::getSaveFileName(this, tr("Export to Callgrind"), "", tr("Callgrind Files (*.callgrind);;All Files (*)"));
	if (!fileName.isEmpty())
	{
		// Open the file for writing
		FILE* f = fopen(fileName.toStdString().c_str(), "w");
		if (f)
		{
			// Write a basic flat Callgrind profile
			fprintf(f, "version: 1\n");
			fprintf(f, "creator: Virtual Jaguar Rx\n");
			fprintf(f, "cmd: 68000 flat profiling\n");
			fprintf(f, "part: 1\n");
			fprintf(f, "positions: line\n");
			fprintf(f, "events: Cycles Calls MinCycles\n");

			size_t totalCycles = 0;
			size_t totalCalls = 0;
			size_t totalMinCycles = 0;
#if 0
			size_t totalMaxMsX1000 = 0;
#endif
			for (int row = 0; row < model->rowCount(); ++row)
			{
				totalCycles += model->item(row, VJRXPW_COL_MAXCYCLES)->data(Qt::UserRole).toULongLong();
				totalCalls += model->item(row, VJRXPW_COL_CALLS)->data(Qt::UserRole).toULongLong();
				totalMinCycles += model->item(row, VJRXPW_COL_MINCYCLES)->data(Qt::UserRole).toULongLong();
#if 0
				double maxMs = model->item(row, VJRXPW_COL_MAXMS)->data(Qt::UserRole).toDouble();
				totalMaxMsX1000 += (size_t)(maxMs * 1000.0);
#endif
			}

			// Write the summary line
			fprintf(f, "summary: %llu %llu %llu\n", totalCycles, totalCalls, totalMinCycles);

			// Write each function's data
			for (int row = 0; row < model->rowCount(); ++row)
			{
				// Write the function's filename
				QString sourceFileName = model->item(row, VJRXPW_COL_FILENAME)->data(Qt::UserRole).toString();				
				fprintf(f, "fl=%s\n", sourceFileName.toStdString().c_str());

				// Write the function's name
				QString functionName = model->item(row, VJRXPW_COL_FUNCTION)->data(Qt::UserRole).toString();
				fprintf(f, "fn=%s\n", functionName.toStdString().c_str());

				// Write the function's max cycles, call count, min cycles
				size_t callCount = model->item(row, VJRXPW_COL_CALLS)->data(Qt::UserRole).toULongLong();
				size_t minCycles = model->item(row, VJRXPW_COL_MINCYCLES)->data(Qt::UserRole).toULongLong();
				size_t maxCycles = model->item(row, VJRXPW_COL_MAXCYCLES)->data(Qt::UserRole).toULongLong();
#if 0
				double maxMs = model->item(row, VJRXPW_COL_MAXMS)->data(Qt::UserRole).toDouble();
				size_t maxMsX1000 = (size_t)(maxMs * 1000.0);
#endif
				// Get the line number for the function
				size_t lineNumber = model->item(row, VJRXPW_COL_LINENUMBER)->data(Qt::UserRole).toULongLong();
				// Write the function's data line
				fprintf(f, "%llu %llu %llu %llu\n", lineNumber, maxCycles, callCount, minCycles);
			}

			// close the file
			fclose(f);
		}
	}
}


//
VJRxProfilerWindow::~VJRxProfilerWindow(void)
{
}
