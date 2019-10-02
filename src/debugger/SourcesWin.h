//
// SourcesWin.h: Sources tracing window
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  08/23/2019  Created this file
//

#ifndef __SOURCESWIN_H__
#define __SOURCESWIN_H__

#include <QtWidgets>
//#include <stdint.h>

class SourceCWindow;

class SourcesWindow : public QWidget
{
	Q_OBJECT

	typedef struct S_SOURCESINFOS
	{
		int IndexTab;
		char *Filename;
		char **SourceText;
		size_t NbLinesText[2];
		size_t *NumLinesUsed;
		size_t Language;
		SourceCWindow *sourceCtab;
		int CurrentNumLineSrc;
	}
	SourcesInfos;

public:
	SourcesWindow(QWidget * parent = 0);
	void Init(void);
	void Close(void);
	void Reset(void);
	bool GetTraceStatus(void);
	bool CheckChangeLine(void);

public slots:
	void RefreshContents(void);
	void SelectTab(const int);
	void CloseTab(const int);

protected:
	void keyPressEvent(QKeyEvent * e);
	void CloseCurrentTab(void);

private:
	QVBoxLayout *layout;
	QTabWidget *sourcestabWidget;
	SourcesInfos *sourcesinfostab;
	size_t NbSourcesInfos;
	size_t CurrentTab;
	size_t OldCurrentNumLineSrc;
	size_t OldCurrentTab;
	int indexErrorTab;
	SourceCWindow *sourceErrorTab;
};

#endif	// __SOURCESWIN_H__
