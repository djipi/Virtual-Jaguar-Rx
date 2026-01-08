//
// filepicker.cpp - A ROM chooser
//
// by James Hammons
// (C) 2010 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JLH  01/22/2010  Created this file
// JLH  02/06/2010  Modified to use Qt model/view framework
// JLH  03/08/2010  Added large cart view and info text
// JPM  06/16/2016  ELF format support
// JPM  09/25/2024  Added .J64 homebrew format detection
// JPM  10/04/2024  Display full filename in cartridge's information
//

#include "filepicker.h"
#include "file.h"
#include "filedb.h"
#include "filelistmodel.h"
#include "filethread.h"
#include "imagedelegate.h"
#include "settings.h"


//
FilePickerWindow::FilePickerWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Window),
currentFile("")
{
	// window's title depend on the emulation mode
	if (!vjs.softTypeDebugger)
	{
		setWindowTitle(tr("Insert Cartridge..."));
	}
	else
	{
		setWindowTitle(tr("Load executable file..."));
	}
	// files list as images
	model = new FileListModel;
	fileList = new QListView;
	fileList->setModel(model);
	fileList->setItemDelegate(new ImageDelegate());
	// set the files list vertical scroll bar
	QScrollBar * vsb = new QScrollBar(Qt::Vertical, this);
	int sbWidth2 = vsb->sizeHint().width();
	delete vsb;
	// force width for the files list
	int sbWidth5 = fileList->frameWidth();
	fileList->setFixedWidth((488/4) + 4 + sbWidth2 + sbWidth5 + 1);
	fileList->setUniformItemSizes(true);
	// set filepicker's layout
	QHBoxLayout * layout = new QHBoxLayout;
	setLayout(layout);
	layout->addWidget(fileList);
	QVBoxLayout * vLayout = new QVBoxLayout;
	layout->addLayout(vLayout);
	// set the cardridge image by default
	cartImage = new QLabel;
	QImage cartImg(":/res/cart-blank.png");
	QPainter painter(&cartImg);
	painter.drawPixmap(23, 87, QPixmap(":/res/label-blank.png"));
	painter.end();
	cartImage->setPixmap(QPixmap::fromImage(cartImg));
	cartImage->setMargin(4);
	vLayout->addWidget(cartImage);
	// set the cardridge name by default
	title = new QLabel(QString(tr("<h2>...</h2>")));
	title->setMargin(6);
#if 0
	title->setAlignment(Qt::AlignCenter);
	title->setFixedWidth(cartImage->sizeHint().width());
#endif
	vLayout->addWidget(title);
	// set the cardridge information by default
	QHBoxLayout * dataLayout = new QHBoxLayout;
	vLayout->addLayout(dataLayout);
	QLabel * labels = new QLabel(QString(tr(
		"<b>Type: </b><br>"
		"<b>CRC32: </b><br>"
		"<b>Compatibility: </b><br>"
		"<b>Notes:</b>"
	)));
	labels->setAlignment(Qt::AlignRight);
	labels->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	dataLayout->addWidget(labels);
	data = new QLabel(QString(tr(
		"?MB Cartridge<br>"
		"00000000<br>"
		"?<br>"
		"?"
	)));
	data->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
	dataLayout->addWidget(data);
	// insert's button default
	insertCart = new QPushButton(this);
	insertCart->setIconSize(QSize(40, 40));
	insertCart->setIcon(QIcon(":/res/insert.png"));
	insertCart->setDefault(true);
	insertCart->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Preferred);
	dataLayout->addWidget(insertCart);
	// make proper connection
	fileThread = new FileThread(this);
	connect(fileThread, SIGNAL(FoundAFile3(unsigned long, QString, QImage *, unsigned long, bool, unsigned long, unsigned long)), this, SLOT(AddFileToList3(unsigned long, QString, QImage *, unsigned long, bool, unsigned long, unsigned long)));
	connect(fileList->selectionModel(), SIGNAL(currentChanged(const QModelIndex &, const QModelIndex &)), this, SLOT(UpdateSelection(const QModelIndex &, const QModelIndex &)));
	connect(insertCart, SIGNAL(clicked()), this, SLOT(LoadButtonPressed()));
	connect(fileList, SIGNAL(doubleClicked(const QModelIndex &)), this, SLOT(CatchDoubleClick(const QModelIndex &)));
}

void FilePickerWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
		emit(FilePickerHiding());
	}
	else
	{
		if (e->key() == Qt::Key_Return)
		{
			LoadButtonPressed();
		}
	}
}

void FilePickerWindow::CatchDoubleClick(const QModelIndex &)
{
	LoadButtonPressed();
}

QString FilePickerWindow::GetSelectedPrettyName(void)
{
	return prettyFilename;
}

void FilePickerWindow::ScanSoftwareFolder(bool allow/*= false*/)
{
	// "allow" is whether or not to allow scanning for unknown software.
	model->ClearData();
	fileThread->Go(allow);
}


void FilePickerWindow::AddFileToList3(unsigned long index, QString str, QImage * img, unsigned long size, bool haveUniversalHeader, unsigned long fileType, unsigned long crc)
{
//if (index != 0xFFFFFFFF)
//	printf("FilePickerWindow(3): Found match [%s]...\n", romList[index].name);

	if (img)
	{
		model->AddData(index, str, *img, size, haveUniversalHeader, fileType, crc);
//It would be better to pass the pointer into the model though...
		delete img;
	}
	else
		model->AddData(index, str, QImage(), size, haveUniversalHeader, fileType, crc);
}

void FilePickerWindow::LoadButtonPressed(void)
{
	// TODO: Get the text of the current selection, call the MainWin slot for loading
	emit(RequestLoad(currentFile));
	hide();
}


// Updates the cart graphic and accompanying text.
void FilePickerWindow::UpdateSelection(const QModelIndex & current, const QModelIndex &/*previous*/)
{
	// get information from the file's index
	currentFile = current.model()->data(current, FLM_FILENAME).toString();
	unsigned long i = current.model()->data(current, FLM_INDEX).toUInt();
	QImage label = current.model()->data(current, FLM_LABEL).value<QImage>();
	unsigned long fileSize = current.model()->data(current, FLM_FILESIZE).toUInt();
	bool haveUniversalHeader = current.model()->data(current, FLM_UNIVERSALHDR).toBool();
	unsigned long fileType = current.model()->data(current, FLM_FILETYPE).toUInt();
	(fileType == JST_NONE) ? fileType = ParseFileExt(current.model()->data(current, FLM_FILETYPE).toString().toLocal8Bit().constData()) : false;
	uint32_t crc = (uint32_t)current.model()->data(current, FLM_CRC).toUInt();
	bool haveUnknown = (i == 0xFFFFFFFF ? true : false);

	// disable loading completely unknown files, but allow all others
	insertCart->setEnabled(haveUnknown && (fileType == JST_NONE) ? false : true);

	// handle cart with a label
	if (!label.isNull())
	{
		QImage cart(":/res/cart-blank.png");
		QPainter painter(&cart);
		painter.drawPixmap(27, 89, QPixmap::fromImage(label));
		painter.drawPixmap(27, 89, QPixmap::fromImage(QImage(":/res/upper-left.png")));
		painter.drawPixmap(27+355, 89, QPixmap::fromImage(QImage(":/res/upper-right.png")));
		painter.end();
		cartImage->setPixmap(QPixmap::fromImage(cart));
	}
	else
	{
		// no label has been found for the ron, a dedicated label will be used depend the headers and/or file type
		QImage cart;

		// ROM with an 'official' header, or considered as such
		if ((!haveUnknown && (romList[i].flags & FF_ROM)) || (haveUnknown && (fileType == JST_ROM) && !haveUniversalHeader))
		{
			cart = QImage(":/res/cart-blank.png");
			QPainter painter(&cart);
			painter.drawPixmap(27, 89, QPixmap::fromImage(QImage(":/res/label-blank.png")));
			painter.end();
		}
		else
		{
			// Alpine header or a ROM with an universal / modified header
			if ((!haveUnknown && (romList[i].flags & FF_ALPINE)) || (haveUnknown && ((fileType == JST_ALPINE) || ((fileType == JST_ROM) && haveUniversalHeader))))
			{
				if (haveUniversalHeader)
				{
					cart = QImage(":/res/skunkboard-file.png");
				}
				else
				{
					cart = QImage(":/res/alpine-file.png");
				}
			}
			else
			{
				// ELF header is dedicated to homebrew development
				if (haveUnknown && (fileType == JST_ELF32))
				{
					cart = QImage(":/res/ELF-file.png");
				}
				else
				{
					// COFF/ABS & JAG headers, or a J64 type, are usualy for homebrew
					if (haveUnknown && ((fileType == JST_ABS_TYPE1) || (fileType == JST_ABS_TYPE2) || (fileType == JST_JAGSERVER)) || (fileType == JST_WTFOMGBBQ) || (fileType == JST_J64))
					{
						cart = QImage(":/res/homebrew-file.png");
					}
					else
					{
						// type is unknow
						cart = QImage(":/res/unknown-file.png");
					}
				}
			}
		}

		cartImage->setPixmap(QPixmap::fromImage(cart));
	}

	// set the rom's title
	if (!haveUnknown)
	{
		prettyFilename = romList[i].name;
	}
	else
	{
		int lastSlashPos = currentFile.lastIndexOf('/');
		prettyFilename = "\"" + currentFile.mid(lastSlashPos + 1) + "\"";
	}
	title->setText(QString("<h2>%1</h2>").arg(prettyFilename));

	// get file type
	QString fileTypeString;
	if ((!haveUnknown && (romList[i].flags & FF_ROM)) || (haveUnknown && (fileType == JST_ROM) && !haveUniversalHeader))
	{
		fileTypeString = QString(tr("%1MB Cartridge")).arg(fileSize / 1048576);
	}
	else
	{
		if ((!haveUnknown && (romList[i].flags & FF_ALPINE)) || (haveUnknown && ((fileType == JST_ALPINE) || ((fileType == JST_ROM) && haveUniversalHeader))))
		{
			if (haveUniversalHeader)
			{
				fileTypeString = QString(tr("%1MB Alpine ROM w/Universal Header"));
			}
			else
			{
				fileTypeString = QString(tr("%1MB Alpine ROM"));
			}
			fileTypeString = fileTypeString.arg((fileSize + 8192) / 1048576);
		}
		else
		{
			if (haveUnknown && (fileType == JST_ELF32))
			{
				fileTypeString = QString(tr("ELF 32bits Executable (%1 bytes)")).arg(fileSize);
			}
			else
			{
				if (haveUnknown && (fileType == JST_ABS_TYPE1 || fileType == JST_ABS_TYPE2))
				{
					fileTypeString = QString(tr("ABS/COF Executable (%1 bytes)")).arg(fileSize);
				}
				else
				{
					if (haveUnknown && (fileType == JST_JAGSERVER))
					{
						fileTypeString = QString(tr("Jaguar Server Executable (%1 bytes)")).arg(fileSize);
					}
					else
					{
						fileTypeString = QString(tr("*** UNKNOWN *** (%1 bytes)")).arg(fileSize);
					}
				}
			}
		}
	}

	// crc
	QString crcString = QString("%1").arg(crc, 8, 16, QChar('0')).toUpper();

	// compatibility
	QString compatibility;
	if (!haveUnknown && (romList[i].flags & FF_NON_WORKING))
	{
		compatibility = "DOES NOT WORK";
	}
	else
	{
		compatibility = "Unknown";
	}

	// notes
	QString notes;
	if (!haveUnknown && (romList[i].flags & FF_BAD_DUMP))
	{
		notes = "<b>BAD DUMP</b>";
	}

//	if (haveUniversalHeader)
//		notes += " Universal Header detected";

	if (!haveUnknown && (romList[i].flags & FF_REQ_BIOS))
	{
		notes += " Requires BIOS";
	}

	if (!haveUnknown && (romList[i].flags & FF_REQ_DSP))
	{
		notes += " Requires DSP";
	}

	if (!haveUnknown && (romList[i].flags & FF_VERIFIED))
	{
		notes += " <i>(Verified)</i>";
	}

	// display the file info: type, crc value, compatibility and notes
	data->setText(QString("%1<br>%2<br>%3<br>%4").arg(fileTypeString).arg(crcString).arg(compatibility).arg(notes));
}

/*
    Super Duper Awesome Guy (World)

         Type: 4MB Cartridge
        CRC32: FEDCBA98
Compatibility: DOES NOT WORK
        Notes: Universal Header detected; Requires DSP


    Stupid Homebrew Game That Sux

         Type: ABS/COF Executable (43853 bytes)
        CRC32: 76543210
Compatibility: Unknown
        Notes: $4000 Load, $4000 Run


    Action Hopscotch Plus (Prototype)

         Type: 2MB Alpine ROM
        CRC32: 44889921
Compatibility: 80% (or ****)
        Notes: EEPROM available
*/
