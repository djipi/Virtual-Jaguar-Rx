//
// SourcesWin.h: Source C tracing window
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  08/23/2019  Created this file
//

#ifndef __SOURCECWIN_H__
#define __SOURCECWIN_H__

//#define SC_LAYOUTTEXTS						// Use a layout with just texts

#include <QtWidgets>
//#include <stdint.h>


class SourceCWindow : public QWidget
{
	Q_OBJECT

public:
	SourceCWindow(QWidget * parent = 0);
	void FillTab(size_t index, char **TextLines, size_t NbLines[], size_t *NumLinesUsed);
	void SetCursorTrace(int NumLineSrc, bool Remove);

public slots:
	void RefreshContents(void);

protected:

private:
	size_t FileIndex;
	int CurrentNumLineSrc;
	QVBoxLayout *layout;
#ifdef SC_LAYOUTTEXTS
	QTextBrowser *text;
#else
	QTableView *TableView;
	QStandardItemModel *model;
#endif
	char **PtrTextLines;
	size_t NbLinesText[2];
	size_t *PtrNumLinesUsed;
};

#endif
