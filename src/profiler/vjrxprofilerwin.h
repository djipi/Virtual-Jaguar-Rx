//
// vjrxprofilerwin.h: VJRx Profiler window
//
// by Jean-Paul Mari
//

#ifndef __VJRXPROFILERWIN_H__
#define __VJRXPROFILERWIN_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>


// Profiler data structure for each function
typedef struct S_VJRxProfilerData
{
	char* FunctionName;
	size_t CallCount;
	size_t minCycles;
	size_t maxCycles;
	//double avgms;				// average time in milliseconds
	double maxms;				// maximum time in milliseconds
	size_t id;
}
VJRxProfilerData;


//
class VJRxProfilerWindow : public QWidget
{
	Q_OBJECT

public:
	VJRxProfilerWindow(QWidget* parent = 0);
	void RefreshContents(size_t NbEntries, VJRxProfilerData* Ptrdata);
	void UpdateContent(size_t NumEntry, VJRxProfilerData* Ptrdata);
	~VJRxProfilerWindow(void);

protected:
	void keyPressEvent(QKeyEvent*);

private:
	QString FormatThousands(size_t value);

	QVBoxLayout* layout;
	QTableView* TableView;
	QStandardItemModel* model;
	QSortFilterProxyModel* proxyModel;
};

#endif
