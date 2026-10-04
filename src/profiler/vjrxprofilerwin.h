//
// vjrxprofilerwin.h: VJRx Profiler window
//
// by Jean-Paul Mari
//

#ifndef __VJRXPROFILERWIN_H__
#define __VJRXPROFILERWIN_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>

//
class VJRxProfilerWindow : public QMainWindow
{
	Q_OBJECT

public:
	// Profiler data structure for each function
	typedef struct S_VJRxProfilerData
	{
		char* SourceFileName;		// source file name
		char* FunctionName;			// function name
		size_t LineNumber;			// line number in the source file
		size_t CallCount;			// number of times the function was called
		size_t minCycles;			// minimum time in cycles
		size_t maxCycles;			// maximum time in cycles
#if 0
		double avgms;				// average time in milliseconds
		double maxms;				// maximum time in milliseconds
#endif
		size_t id;					// unique id
	}
	VJRxProfilerData;

public:
	VJRxProfilerWindow(QWidget* parent = 0);
	void RefreshContents(size_t NbEntries, VJRxProfilerData* Ptrdata);
	void UpdateContent(size_t NumEntry, VJRxProfilerData* Ptrdata);
	~VJRxProfilerWindow(void);

protected:
	void keyPressEvent(QKeyEvent*);

private slots:
	void OnExportCSV();
	void OnExportCallgrind();

private:
	QString FormatThousands(size_t value);
	QTableView* TableView;
	QStandardItemModel* model;
	QSortFilterProxyModel* proxyModel;
	QMenuBar* menuBar;
	QMenu* fileMenu;
	QMenu* exportMenu;
};

#endif
