//
// exceptionvectortable.cpp - All Watch
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  05/09/2017  Created this file
//

// STILL TO DO:
//

#include "debugger\exceptionvectortablebrowser.h"
#include "memory.h"
#include "debugger\DBGManager.h"


//
struct ExceptionVectorTable
{
	size_t VectorNumber;
	size_t Address;
	const char *ExceptionName;
}S_ExceptionVectorTable;


ExceptionVectorTable	TabExceptionVectorTable[] =	{
														{	0, 0x000000, "RESET - Initial SSP"				},
														{	1, 0x000004, "RESET - Initial PC"				},
														{	2, 0x000008, "Bus error"						},
														{	3, 0x00000C, "Address error"					},
														{	4, 0x000010, "Illegal instruction"				},
														{	5, 0x000014, "Division by zero"					},
														{	6, 0x000018, "CHK instruction"					},
														{	7, 0x00001C, "TRAPV instruction"				},
														{	8, 0x000020, "Privilege violation"				},
														{	9, 0x000024, "Trace"							},
														{	10, 0x000028, "Unimplemented instruction"		},
														{	11, 0x00002C, "Unimplemented instruction"		},
														{	12, 0x000030, "Reserved by Motorola"			},
														{	13, 0x000034, "Reserved by Motorola"			},
														{	14, 0x000038, "Reserved by Motorola"			},
														{	15, 0x00003C, "Uninitialised interrupt vector"	},
														{	16, 0x000040, "Reserved by Motorola"			},
														{	17, 0x000044, "Reserved by Motorola"			},
														{	18, 0x000048, "Reserved by Motorola"			},
														{	19, 0x00004C, "Reserved by Motorola"			},
														{	20, 0x000050, "Reserved by Motorola"			},
														{	21, 0x000054, "Reserved by Motorola"			},
														{	22, 0x000058, "Reserved by Motorola"			},
														{	23, 0x00005C, "Reserved by Motorola"			},
														{	24, 0x000060, "Spurious interrupt"				},
														{	25, 0x000064, "Level 1 interrupt autovector"	},
														{	26, 0x000068, "Level 2 interrupt autovector"	},
														{	27, 0x00006C, "Level 3 interrupt autovector"	},
														{	28, 0x000070, "Level 4 interrupt autovector"	},
														{	29, 0x000074, "Level 5 interrupt autovector"	},
														{	30, 0x000078, "Level 6 interrupt autovector"	},
														{	31, 0x00007C, "Level 7 interrupt autovector"	},
														{	32, 0x000080, "TRAP #0 instruction"				},
														{	33, 0x000084, "TRAP #1 instruction"				},
														{	34, 0x000088, "TRAP #2 instruction"				},
														{	35, 0x00008C, "TRAP #3 instruction"				},
														{	36, 0x000090, "TRAP #4 instruction"				},
														{	37, 0x000094, "TRAP #5 instruction"				},
														{	38, 0x000098, "TRAP #6 instruction"				},
														{	39, 0x00009C, "TRAP #7 instruction"				},
														{	40, 0x0000A0, "TRAP #8 instruction"				},
														{	41, 0x0000A4, "TRAP #9 instruction"				},
														{	42, 0x0000A8, "TRAP #10 instruction"			},
														{	43, 0x0000AC, "TRAP #11 instruction"			},
														{	44, 0x0000B0, "TRAP #12 instruction"			},
														{	45, 0x0000B4, "TRAP #13 instruction"			},
														{	46, 0x0000B8, "TRAP #14 instruction"			},
														{	47, 0x0000BC, "TRAP #15 instruction"			},
														{	48, 0x0000C0, "Reserved by Motorola"			},
														{	49, 0x0000C4, "Reserved by Motorola"			},
														{	50, 0x0000C8, "Reserved by Motorola"			},
														{	51, 0x0000CC, "Reserved by Motorola"			},
														{	52, 0x0000D0, "Reserved by Motorola"			},
														{	53, 0x0000D4, "Reserved by Motorola"			},
														{	54, 0x0000D8, "Reserved by Motorola"			},
														{	55, 0x0000DC, "Reserved by Motorola"			},
														{	56, 0x0000E0, "Reserved by Motorola"			},
														{	57, 0x0000E4, "Reserved by Motorola"			},
														{	58, 0x0000E8, "Reserved by Motorola"			},
														{	59, 0x0000EC, "Reserved by Motorola"			},
														{	60, 0x0000F0, "Reserved by Motorola"			},
														{	61, 0x0000F4, "Reserved by Motorola"			},
														{	62, 0x0000F8, "Reserved by Motorola"			},
														{	63, 0x0000FC, "Reserved by Motorola"			},
														{	64, 0x000100, "User interrupt vectors"			},
														{	255, 0x0003FC, "User interrupt vectors"			}
													};


ExceptionVectorTableBrowserWindow::ExceptionVectorTableBrowserWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QTextBrowser),
//	layout(new QVBoxLayout), text(new QLabel),
	refresh(new QPushButton(tr("Refresh")))
//	address(new QLineEdit),
//	go(new QPushButton(tr("Go"))),
//	memBase(0),
//	NbWatch(0),
//	PtrWatchInfo(NULL)
{
	setWindowTitle(tr("Exception Vector Table"));

//	address->setInputMask("hhhhhh");
	QHBoxLayout *hbox1 = new QHBoxLayout;
	hbox1->addWidget(refresh);
//	hbox1->addWidget(address);
//	hbox1->addWidget(go);

	// Need to set the size as well...
//	resize(560, 480);

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
//	QFont fixedFont("", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::TypeWriter);
	text->setFont(fixedFont);
////	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	layout->addWidget(text);
	layout->addWidget(refresh);
//	layout->addLayout(hbox1);

	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
//	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
}


//
ExceptionVectorTableBrowserWindow::~ExceptionVectorTableBrowserWindow(void)
{
//	NbWatch = 0;
//	free(PtrWatchInfo);
}


//
void ExceptionVectorTableBrowserWindow::RefreshContents(void)
{
	char string[1024];
	QString ExceptionVector;
	size_t i;

	if (isVisible())
	{
		for (i = 0; i < (sizeof(TabExceptionVectorTable) / sizeof(ExceptionVectorTable)); i++)
		{
			sprintf(string, "%03i : 0x%06X | 0x%06X | %s<br>", (unsigned int)TabExceptionVectorTable[i].VectorNumber, (unsigned int)TabExceptionVectorTable[i].Address, GET32(jaguarMainRAM, TabExceptionVectorTable[i].Address), TabExceptionVectorTable[i].ExceptionName);
			ExceptionVector += QString(string);
		}

		text->clear();
		text->setText(ExceptionVector);
	}
}


#if 0
void ExceptionVectorTableBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
		hide();
	else if (e->key() == Qt::Key_PageUp)
	{
		memBase -= 480;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_PageDown)
	{
		memBase += 480;

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Up || e->key() == Qt::Key_Minus)
	{
		memBase -= 16;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_Down || e->key() == Qt::Key_Equal)
	{
		memBase += 16;

		if (memBase > (0x200000 - 480))
			memBase = 0x200000 - 480;

		RefreshContents();
	}
}
#endif


#if 0
void ExceptionVectorTableBrowserWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
#endif

