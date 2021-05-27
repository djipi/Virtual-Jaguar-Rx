//
//  SourceCWin.cpp - Source C tracing window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  08/23/2019  Created this file

// STILL TO DO:
//

#include <stdlib.h>
#include "DBGManager.h"
#include "debugger/SourceCWin.h"
#include "debugger/SourcesWin.h"
//#include "m68000/m68kinterface.h"


// 
SourceCWindow::SourceCWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
FileIndex(0),
CurrentNumLineSrc(0),
#ifdef SC_LAYOUTTEXTS
text(new QTextBrowser)
#else
TableView(new QTableView),
model(new QStandardItemModel)
#endif
{
	// Set font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::Monospace);   //TypeWriter
	fixedFont.setLetterSpacing(QFont::PercentageSpacing, 100);

	// Set text in layout
#ifdef SC_LAYOUTTEXTS
	text->setFont(fixedFont);
	layout->addWidget(text);
#else
	// Set the new layout with proper identation and readibility
	model->setColumnCount(3);
	model->setHeaderData(0, Qt::Horizontal, QObject::tr("LineStatus"));
	model->setHeaderData(1, Qt::Horizontal, QObject::tr("LineNumber"));
	model->setHeaderData(2, Qt::Horizontal, QObject::tr("LineSrc"));
	// Information table
	TableView->setModel(model);
	TableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
	TableView->setSelectionMode(QAbstractItemView::NoSelection);
	TableView->setShowGrid(false);
	TableView->horizontalHeader()->hide();
	TableView->verticalHeader()->hide();
	TableView->setFont(fixedFont);
	layout->addWidget(TableView);
#endif

	// Set layout
	setLayout(layout);
}


//
void SourceCWindow::SetCursorTrace(int NumLineSrc, bool Remove)
{
	// Check valid line number
	if (NumLineSrc && (NbLinesText[0] >= NumLineSrc))
	{
		// Set the trace cursor
		if (!Remove)
		{
			CurrentNumLineSrc = NumLineSrc;
#ifndef SC_LAYOUTTEXTS
			model->item((NumLineSrc - 1), 0)->setText(">");
			model->item((NumLineSrc - 1), 0)->setBackground(QColor(0xff, 0xfa, 0xcd));
			model->item((NumLineSrc - 1), 1)->setBackground(QColor(0xff, 0xfa, 0xcd));
			model->item((NumLineSrc - 1), 2)->setBackground(QColor(0xff, 0xfa, 0xcd));
#endif
		}
		else
		{
			// Remove the trace cursor
#ifndef SC_LAYOUTTEXTS
			model->item((NumLineSrc - 1), 0)->setText(" ");
			model->item((NumLineSrc - 1), 0)->setBackground(QColor(173, 216, 230));
			model->item((NumLineSrc - 1), 1)->setBackground(QColor(255, 255, 255));
			model->item((NumLineSrc - 1), 2)->setBackground(QColor(255, 255, 255));
#endif
		}
	}
}


//
void SourceCWindow::RefreshContents(void)
{
	if (isVisible())
	{
		// Get the scroll bar information
		int MaxSlider = TableView->verticalScrollBar()->maximum();							// Get the slider maximum position
		int DeltaSlider = (int)NbLinesText[0] - MaxSlider;									// Number of items displayed in the scroll bar slider
		//int PosSlider = TableView->verticalScrollBar()->sliderPosition();					// Slider position

		// Check visibility in the scroll bar
		//if ((CurrentNumLineSrc > PosSlider) && ((CurrentNumLineSrc + (DeltaSlider / 2)) < MaxSlider))
		{
			// Set the scroll bar position 
			TableView->verticalScrollBar()->setSliderPosition(CurrentNumLineSrc - (DeltaSlider / 2) - 1);
		}
	}
}


// Fill the tab with the source text
void SourceCWindow::FillTab(size_t index, char **TextLines, size_t NbLines[], size_t *NumLinesUsed)
{
	int i, j, k;
#ifdef SC_LAYOUTTEXTS
	QString s;
	char string[1024];
#endif

	// Save information
	FileIndex = index;
	for (i = 0; i < 2; i++)
	{
		NbLinesText[i] = NbLines[i];
	}
	PtrTextLines = TextLines;
	PtrNumLinesUsed = NumLinesUsed;

	// Set columns
#ifndef SC_LAYOUTTEXTS
	model->insertRow((int)NbLinesText[0]);
#endif

	// Set text lines
	for (i = j = 0; i < NbLinesText[0]; i++, j = 0)
	{
		// prepare space for the line status
#ifndef SC_LAYOUTTEXTS
		model->setItem(i, 0, new QStandardItem(QString("%1").arg(" ")));
		model->item(i, 0)->setTextAlignment(Qt::AlignCenter);
		model->item(i, 0)->setBackground(QColor(173, 216, 230));
#endif
		// display line number
#ifdef SC_LAYOUTTEXTS
		sprintf(string, "| <font color='#006400'>%5u</font> | ", (i + 1));
		s += QString(string);
#else
		model->setItem(i, 1, new QStandardItem(QString(" %1 ").arg((i + 1))));
		model->item(i, 1)->setTextAlignment(Qt::AlignRight | Qt::AlignVCenter);
		model->item(i, 1)->setForeground(QColor(0, 0x64, 0));
#endif
		// display source code line
#ifndef SC_LAYOUTTEXTS
		model->setItem(i, 2, new QStandardItem(QString("%1").arg(PtrTextLines[i])));
		model->item(i, 2)->setTextAlignment(Qt::AlignLeft | Qt::AlignVCenter);
#endif
		// Check line used by code
		while ((j < NbLinesText[1]) && !(k = ((PtrNumLinesUsed[j++] != i) - 1)));
		if (k)
		{
			// Line is used by code
#ifdef SC_LAYOUTTEXTS
			sprintf(string, "<font color='#000000'><b>%s</b></font>", PtrTextLines[i]);
#else
			model->item(i, 2)->setForeground(QColor(0, 0, 0));
#endif
		}
		else
		{
			// Line is not used by code (such as comments)
#ifdef SC_LAYOUTTEXTS
			sprintf(string, "<font color='#C8C8C8'>%s</font>", PtrTextLines[i]);
#else
			model->item(i, 2)->setForeground(QColor(0xc8, 0xc8, 0xc8));
#endif
		}
#ifdef SC_LAYOUTTEXTS
		s += QString(string);
		s += QString("<br>");
#endif
	}

	// Display text
#ifdef SC_LAYOUTTEXTS
	text->setText(s);
#else
	TableView->resizeColumnsToContents();
	TableView->resizeRowsToContents();
#endif
}
