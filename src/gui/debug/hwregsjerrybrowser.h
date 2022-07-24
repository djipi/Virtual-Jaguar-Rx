//
// hwregsjerrybrowser.h: Hardware registers Jerry browser
//
// by Jean-Paul Mari
//

#ifndef __HWREGSJERRYBROWSER_H__
#define __HWREGSJERRYBROWSER_H__

#include <QtWidgets/QtWidgets>
#include <stdint.h>


// 
class HWRegsJerryBrowserWindow : public QWidget
{
	Q_OBJECT

public:
	HWRegsJerryBrowserWindow(QWidget *parent = 0);
	~HWRegsJerryBrowserWindow(void);
	void Reset(void);

public slots:
	void RefreshContents(void);

protected:

private:
	QVBoxLayout *layout;
	QTableView *TableView;
	QStandardItemModel *model;
};


#endif
