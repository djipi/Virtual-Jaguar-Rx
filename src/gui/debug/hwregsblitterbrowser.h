//
// hwregsblitterbrowser.h: Hardware registers blitter browser
//
// by Jean-Paul Mari
//

#ifndef __HWREGSBLITTERBROWSER_H__
#define __HWREGSBLITTERBROWSER_H__

#include <QtWidgets>
#include <stdint.h>


// 
class HWRegsBlitterBrowserWindow : public QWidget
{
	Q_OBJECT

public:
	HWRegsBlitterBrowserWindow(QWidget *parent = 0);
	~HWRegsBlitterBrowserWindow(void);
	void Reset(void);

public slots:
	void RefreshContents(void);

protected:
	//void keyPressEvent(QKeyEvent *);

private:
	QVBoxLayout *layout;
	QTableView *TableView;
	QStandardItemModel *model;
};


#endif
