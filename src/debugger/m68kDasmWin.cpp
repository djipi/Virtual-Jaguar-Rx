//
//  m68kDasmWin.cpp - M68K disassembly window
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -------------------------------------------------------------
// JPM  06/27/2016  Created this file
// JPM  12/04/2016  Suport ELF debug information
// JPM              Replacing the ELF support by the debugger information manager calls
//

// STILL TO DO:
//

#include	<stdlib.h>
#include "debugger/m68kDasmWin.h"
//#include "memory.h"
#include "m68000/m68kinterface.h"
#include "dsp.h"
#include "gpu.h"
//#include "jaguar.h"
//#include "ELFManager.h"
#include "DBGManager.h"
//extern	jaguarRunAddress;
#include "settings.h"


// 
m68KDasmWindow::m68KDasmWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QTextBrowser),
//	layout(new QTabWidget), text(new QLabel),
#if 0
	refresh(new QPushButton(tr("Refresh"))),
	address(new QLineEdit),
	go(new QPushButton(tr("Go"))),
#endif
//	memBase(0x4000)
	//sb(new QScrollBar),
	memBase(0)
{
//	m68kDasmWindow *m68kDasmWin = new m68kDasmWindow();
//	setWindowTitle(tr("Jaguar Disassembly Window"));
//	layout->addTab(new m68KDasmWindow(), tr("M68K Disassembly"));
	//layout = new QTabWidget;
	//layout->QWidget();

#if 0
	address->setInputMask("hhhhhh");
	QHBoxLayout * hbox1 = new QHBoxLayout;
	hbox1->addWidget(refresh);
	hbox1->addWidget(address);
	hbox1->addWidget(go);
#endif

	// Need to set the size as well...
//	resize(560, 480);

	QFont fixedFont("Lucida Console", 8, QFont::Normal);
//	QFont fixedFont("", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::Monospace);   //TypeWriter
	fixedFont.setLetterSpacing(QFont::PercentageSpacing, 100);
	text->setFont(fixedFont);
//	sb = layout->text->verticalScrollBar();
//	text->setStyleSheet("background-color: DeepSkyBlue;");
////	layout->setSizeConstraint(QLayout::SetFixedSize);
	setLayout(layout);

	layout->addWidget(text);
//	layout->addWidget(refresh);
#if 0
	layout->addLayout(hbox1);
#endif

#if 0
	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
#endif
}


//
void m68KDasmWindow::RefreshContents(void)
{
	//char ;	//, buf[64];
	QString s;
	char buffer[1024], string[1024], adresse[16];
	size_t pc = memBase, oldpc;
	size_t m68kPC = m68k_get_reg(NULL, M68K_REG_PC);
	size_t m68KPCNbrDisasmLines = 0;
	char *Symbol = NULL, *LineSrc, *CurrentLineSrc = NULL;
	bool m68kPCShow = false;
	bool constant, adr, equal, Error;
	size_t j, i;
	size_t	nbr = vjs.nbrdisasmlines;
	char *PtrFullSource, *CurrentPtrFullSource = (char *)calloc(1, 1);
	size_t NumLine;	// , CurrentNumLine = 0;
//	size_t CurrentNumLine = DBGManager_GetNumLineFromAdr(pc, DBG_NO_TAG) - 1;
	size_t CurrentNumLine;
	char singleCharString[2] = { 0, 0 };

	//text->clear();

	for (i = 0; i < nbr; i++)
	{
		//sb->setValue(sb->maximum());
		//j = text->verticalScrollBar()->maximum();
		//text->verticalScrollBar()->setValue(text->verticalScrollBar()->maximum());
		//if (text->verticalScrollBar()->value())
		//{
		//	j = 1;
		//}

		oldpc = pc;
//		WriteLog("%06X: %s\n", oldpc, buffer);
		adr = constant = equal = false;

		// Display source filename based on the program address
		if (vjs.displayFullSourceFilename && (PtrFullSource = DBGManager_GetFullSourceFilenameFromAdr(oldpc, &Error)) && strcmp(CurrentPtrFullSource, PtrFullSource))
		{
#if 0
			if (strcmp(OldPtrFullSource, PtrFullSource))
#endif
			{
#if 1
				if (i)
				{
					nbr++;
					s += QString("<br>");
				}
#endif
				CurrentNumLine = DBGManager_GetNumLineFromAdr(pc, DBG_NO_TAG) - 1;
				nbr++;
				CurrentPtrFullSource = (char *)realloc(CurrentPtrFullSource, strlen(PtrFullSource) + 1);
				strcpy(CurrentPtrFullSource, PtrFullSource);
				if (!Error)
				{
					sprintf(string, "<font color='#ff0000'><b>%s</b></font><br>", PtrFullSource);
				}
				else
				{
					sprintf(string, "<font color='#00ff00'><b>%s</b></font><br>", PtrFullSource);
				}
				s += QString(string);
			}
		}

		// Display line number based on the program address
		if ((NumLine = DBGManager_GetNumLineFromAdr(oldpc, DBG_NO_TAG)) && ((signed)NumLine > (signed)CurrentNumLine))
		{
			if ((signed)CurrentNumLine < 0)
			{
				CurrentNumLine = NumLine - 1;
			}
			sprintf(string, "| <font color='#006400'>%5u</font> | ", (unsigned int)++CurrentNumLine);		// (CurrentNumLine = NumLine));
		}
		else
		{
			sprintf(string, "|      | ");
		}
		s += QString(string);

		// Display line source based on the program address
		if (((signed)CurrentNumLine > 0) && (LineSrc = DBGManager_GetLineSrcFromNumLineBaseAdr(oldpc, CurrentNumLine)) && (LineSrc != CurrentLineSrc))
		{
			sprintf(string, "<font color='#006400'>%s</font><br>", (CurrentLineSrc = LineSrc));
			s += QString(string);
			nbr++;
		}
		else
		{
			// Display symbol, or line source, based on the program address
			if (!CurrentLineSrc && !Symbol && (Symbol = DBGManager_GetSymbolNameFromAdr(oldpc)))
			{
				sprintf(string, "%s:<br>", Symbol);
				s += QString(string);
				nbr++;
			}
			// Display the assembly line based on the current PC
			else
			{
				pc += m68k_disassemble(buffer, (unsigned int)pc, 0, vjs.disasmopcodes);

				if (m68kPC == oldpc)
				{
					sprintf(string, "-> <u>%06X: %s</u><br>", (unsigned int)oldpc, buffer);
					m68kPCShow = true;
					m68KPCNbrDisasmLines = i;
				}
				else
				{
					sprintf(string, "   %06X: %s<br>", (unsigned int)oldpc, buffer);
				}

				buffer[0] = 0;	// Clear string
				//char singleCharString[2] = { 0, 0 };

				for (j = 0; j < strlen(string); j++)
				{
					if (string[j] == ' ')
					{
						strcat(buffer, "&nbsp;");
						adr = constant = false;
					}
					else
					{
						switch (string[j])
						{
						case	'#':
							constant = true;
							break;

						case	'$':
							adr = true;
							break;

						case	',':
							constant = adr = equal = false;
							break;

						case	'=':
							equal = true;
							break;
						}

						if (!constant && adr && !equal)
						{
							int l = 0;
							char *p;
							do
							{
								adresse[l++] = string[++j];
							} while ((string[(j + 1)] >= '0') && (string[(j + 1)] <= '9') || (string[(j + 1)] >= 'A') && (string[(j + 1)] <= 'F'));
							adresse[l] = 0;

							if (Symbol = DBGManager_GetSymbolNameFromAdr(strtoul(adresse, &p, 16)))
							{
								strcat(buffer, Symbol);
							}
							else
							{
								strcat(buffer, "$");
								strcat(buffer, adresse);
							}

							adr = false;
						}
						else
						{
							singleCharString[0] = string[j];
							strcat(buffer, singleCharString);
						}
					}
				}

				Symbol = NULL;
				//		s += QString(string);
				s += QString(buffer);
				//		text->setStyleSheet("QLabel { background-color : red; color : blue; }");
			}
		}
	}

	// Display generated text
	text->clear();
	if (m68kPCShow)
	{
		text->setText(s);
	}
	else
	{
		Use68KPCAddress();
		RefreshContents();
	}

	// Set the scrollbar position in accordance of the M68K PC pointer 
	if (m68KPCNbrDisasmLines > (nbr / 2))
	{
		text->verticalScrollBar()->setValue(text->verticalScrollBar()->maximum());
	}
	else
	{
		text->verticalScrollBar()->setValue(text->verticalScrollBar()->minimum());
	}

	free(CurrentPtrFullSource);
}


#if 0
// 
void m68KDasmWindow::keyPressEvent(QKeyEvent * e)
{
//	if (e->key() == Qt::Key_Escape || e->key() == Qt::Key_Return)
//		hide();
#if 0
	else if (e->key() == Qt::Key_PageUp)
	{
		memBase -= 64;

		if (memBase < 0)
			memBase = 0;

		RefreshContents();
	}
	else if (e->key() == Qt::Key_PageDown)
	{
		memBase += 64;

		if (memBase > (0xF00000 - 64))
			memBase = 0xF00000 - 64;

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

		if (memBase > (0xF00000 - 64))
			memBase = 0xF00000 - 64;

		RefreshContents();
	}
#endif
}
#endif


// Set mem base PC address using the 68K pc current address
void	m68KDasmWindow::Use68KPCAddress(void)
{
	memBase = m68k_get_reg(NULL, M68K_REG_PC);
}


// Set mem base PC address
void m68KDasmWindow::SetAddress(int address)
{
	memBase = address;
//	RefreshContents();
}


#if 0
// Set mem base PC address based on user interaction
void m68KDasmWindow::GoToAddress(void)
{
	bool ok;

	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
#endif
