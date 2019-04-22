//
// SaveDumpAsWin.cpp - Save Dump function
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  04/10/2019  Created this file
//

// STILL TO DO:
//

#include "debugger/SaveDumpAsWin.h"
#include "jaguar.h"
#include "debugger/DBGManager.h"
#include "m68000/m68kinterface.h"
#include "settings.h"


//
SaveDumpAsWindow::SaveDumpAsWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
maddress(new QLineEdit),
msize(new QLineEdit),
savedump(new SaveDumpAsInfo),
save(new QPushButton(tr("Save")))
{
	setWindowTitle(tr("Save Dump As..."));

	maddress->setPlaceholderText("0x<value>, decimal value or symbol name");
	msize->setPlaceholderText("0x<value>, or decimal value");

	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(maddress);
	hbox1->addWidget(msize);
	hbox1->addWidget(save);

	layout->addLayout(hbox1);
	setLayout(layout);

	connect(save, SIGNAL(clicked()), this, SLOT(SaveDumpAs()));
}


//
SaveDumpAsWindow::~SaveDumpAsWindow(void)
{
}


//
void SaveDumpAsWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
	else
	{
		if (e->key() == Qt::Key_Return)
		{
			SaveDumpAs();
		}
	}
}


// Save Dump
void SaveDumpAsWindow::SaveDumpAs(void)
{
	if (SelectAddress() && SelectSize())
	{
		QString fileName = QFileDialog::getSaveFileName(this, tr("Save dump"), "", tr("Save dump files (*.bin)"));

		if (fileName.size())
		{
			if (FILE *File = fopen((char *)fileName.toUtf8().data(), "wb"))
			{
				if (fwrite((void *)&jagMemSpace[savedump->Adr], savedump->Size, 1, File) == 1)
				{
					fclose(File);
				}
			}
		}
	}
}


// Select size
bool SaveDumpAsWindow::SelectSize(void)
{
	bool ok = false;
	size_t len;
	QString newSize;
	size_t s;

	QPalette p = msize->palette();
	newSize = msize->text();

	if ((len = newSize.size()))
	{
		if ((len > 1) && (newSize.at(0) == QChar('0')) && (newSize.at(1) == QChar('x')))
		{
			s = newSize.toUInt(&ok, 16);
		}
		else
		{
			s = newSize.toUInt(&ok, 10);
		}

		// Check validity size
		if (ok && s && (s < 0xffffff))
		{
			// In all cases, consider size as valid
			savedump->Size = s;
			p.setColor(QPalette::Text, Qt::darkYellow);
		}
		else
		{
			// Size is not valid
			ok = false;
			p.setColor(QPalette::Text, Qt::red);
		}
	}
	else
	{
		// Size has not be set
		p.setColor(QPalette::Text, Qt::darkRed);
	}

	msize->setPalette(p);
	return ok ? true : false;
}


// Select address
// Address can be an hexa, decimal or a symbol name
bool SaveDumpAsWindow::SelectAddress(void)
{
	bool ok = false;
	size_t len;
	QString newAddress;
	size_t adr;

	QPalette p = maddress->palette();
	newAddress = maddress->text();

	if ((len = newAddress.size()))
	{
		if ((len > 1) && (newAddress.at(0) == QChar('0')) && (newAddress.at(1) == QChar('x')))
		{
			adr = newAddress.toUInt(&ok, 16);
		}
		else
		{
			if (!(adr = DBGManager_GetAdrFromSymbolName(newAddress.toLatin1().data())))
			{
				adr = newAddress.toUInt(&ok, 10);
			}
			else
			{
				ok = true;
			}
		}

		// Check validity address
		if (ok && (adr < 0xffffff))
		{
			// In all cases, consider address as valid
			savedump->Adr = adr;
			p.setColor(QPalette::Text, Qt::darkYellow);
		}
		else
		{
			// Address is not valid
			p.setColor(QPalette::Text, Qt::red);
		}
	}
	else
	{
		// Address has not be set
		p.setColor(QPalette::Text, Qt::darkRed);
	}

	maddress->setPalette(p);
	return ok ? true : false;
}
