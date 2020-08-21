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
// JPM   Aug./2020  Display only the code related to the traced function, added different layouts & a status bar, Qt/HTML text format support
//

// STILL TO DO:
//

#include <stdlib.h>
#include "debugger/m68kDasmWin.h"
#include "m68000/m68kinterface.h"
#include "dsp.h"
#include "gpu.h"
#include "DBGManager.h"
#include "settings.h"


// 
m68KDasmWindow::m68KDasmWindow(QWidget * parent/*= 0*/): QWidget(parent, Qt::Dialog),
layout(new QVBoxLayout),
#if MD_LAYOUTFILE == 1
statusbar(new QStatusBar),
#endif
#ifdef MD_LAYOUTTEXTS
text(new QTextBrowser),
#endif
memBase(0)
{
	// Set font
	QFont fixedFont("Lucida Console", 8, QFont::Normal);
	fixedFont.setStyleHint(QFont::Monospace);   //TypeWriter
	fixedFont.setLetterSpacing(QFont::PercentageSpacing, 100);

	// Set text in layout
#ifdef MD_LAYOUTTEXTS
	text->setFont(fixedFont);
	layout->addWidget(text);
#endif

	// Status bar
#if MD_LAYOUTFILE == 1
	layout->addWidget(statusbar);
	setLayout(layout);
#endif

	// Set layout
	setLayout(layout);
}


//
void m68KDasmWindow::RefreshContents(void)
{
	QString s;
	char buffer[1024], string[1024], adresse[16];
	size_t pc = memBase, oldpc;
	size_t m68kPC = m68k_get_reg(NULL, M68K_REG_PC);
	size_t m68KPCNbrDisasmLines = 0;
	char *Symbol = NULL, *LineSrc, *CurrentLineSrc = NULL;
	bool m68kPCShow = false;
	bool constant, adr, equal;
	DBGstatus Status;
	size_t j, i;
	size_t	nbr = vjs.nbrdisasmlines;
	char *PtrFullSource, *CurrentPtrFullSource = (char *)calloc(1, 1);
	size_t NumLine;	// , CurrentNumLine = 0;
	size_t CurrentNumLine;
	char singleCharString[2] = { 0, 0 };
#if MD_LAYOUTFILE == 1
	bool In = true;
#else
#define In	true
#endif

	for (i = 0; (i < nbr) && In; i++)
	{
		oldpc = pc;
		adr = constant = equal = false;

		// Display source filename based on the program address
		if (vjs.displayFullSourceFilename && (PtrFullSource = DBGManager_GetFullSourceFilenameFromAdr(oldpc, &Status)) && strcmp(CurrentPtrFullSource, PtrFullSource))
		{
#if defined(MD_LAYOUTFILE)
			if (i)
#if MD_LAYOUTFILE != 1
			{
				// add an empty line for the display of the new filename
				nbr++;
				s += QString("<br>");
			}
#else
			{
				In = false;
			}
			else
#endif
#endif
			{
				CurrentNumLine = DBGManager_GetNumLineFromAdr(pc, DBG_NO_TAG) - 1;
				CurrentPtrFullSource = (char *)realloc(CurrentPtrFullSource, strlen(PtrFullSource) + 1);
				strcpy(CurrentPtrFullSource, PtrFullSource);
#if defined(MD_LAYOUTFILE)
				if (Status)
				{
					// Referenced filename doesn't exist
					sprintf(string, "<font color='#ff0000'><b>%s</b></font><br>", PtrFullSource);
#if MD_LAYOUTFILE == 1
					// Display status bar
					if ((Status & DBGSTATUS_OUTDATEDFILE))
					{
						statusbar->setStyleSheet("background-color: lightyellow; font: bold");
						statusbar->showMessage(QString("Outdated source file"));
					}
					else
					{
						statusbar->setStyleSheet("background-color: tomato; font: bold");
						statusbar->showMessage(QString("Unavailable source file"));
					}
#endif
				}
				else
				{
					// Referenced filename does exist
					sprintf(string, "<font color='#00ff00'><b>%s</b></font><br>", PtrFullSource);
#if MD_LAYOUTFILE == 1
					// Display status bar
					statusbar->setStyleSheet("background-color: transparent; font: bold");
					statusbar->showMessage(QString(""));
#endif
				}
				nbr++;
				s += QString(string);
#endif
			}
		}
		else
		{
			// Display line number based on the program address
			if ((NumLine = DBGManager_GetNumLineFromAdr(oldpc, DBG_NO_TAG)) && ((signed)NumLine > (signed)CurrentNumLine) && !Status)
			{
#if MD_LAYOUTFILE != 1
				if ((signed)CurrentNumLine < 0)
				{
					CurrentNumLine = NumLine - 1;
				}
#endif
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
#if 0
				// add a color on the line text
				sprintf(string, "<font color='#006400'>%s</font><br>", (CurrentLineSrc = LineSrc));
				s += QString(string);
#else
				// add a color on the line text with HTML encoding
				s += QString("<font color='#006400'>");
				s += QString(QString((CurrentLineSrc = LineSrc)).toHtmlEscaped());
				s += QString("</font><br>");
#endif
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
					s += QString(buffer);
				}
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


// Set mem base PC address using the 68K pc current address
void m68KDasmWindow::Use68KPCAddress(void)
{
	memBase = m68k_get_reg(NULL, M68K_REG_PC);
}


// Set mem base PC address
void m68KDasmWindow::SetAddress(int address)
{
	memBase = address;
}
