//
// localbrowser.cpp - Local variables
//
// by Jean-Paul Mari
//
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  -----------------------------------------------------------
// JPM  11/03/2017  Created this file
//


#include "debugger/localbrowser.h"
#include "memory.h"
#include "debugger/DBGManager.h"
#include "settings.h"
#include "m68000/m68kinterface.h"


// 
LocalBrowserWindow::LocalBrowserWindow(QWidget * parent/*= 0*/) : QWidget(parent, Qt::Dialog),
	layout(new QVBoxLayout), text(new QTextBrowser),
//	layout(new QVBoxLayout), text(new QLabel),
//	refresh(new QPushButton(tr("Refresh"))),
//	address(new QLineEdit),
//	go(new QPushButton(tr("Go"))),
//	memBase(0),
	NbLocal(0),
	FuncName((char *)calloc(1, 1)),
	LocalInfo(NULL)
{
	setWindowTitle(tr("Local"));

//	address->setInputMask("hhhhhh");
//	QHBoxLayout * hbox1 = new QHBoxLayout;
//	hbox1->addWidget(refresh);
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
//	layout->addWidget(refresh);
//	layout->addLayout(hbox1);

//	connect(refresh, SIGNAL(clicked()), this, SLOT(RefreshContents()));
//	connect(go, SIGNAL(clicked()), this, SLOT(GoToAddress()));
}


//
LocalBrowserWindow::~LocalBrowserWindow(void)
{
	free(LocalInfo);
	free(FuncName);
//	NbLocal = 0;
}


//
bool LocalBrowserWindow::UpdateInfos(void)
{
	size_t Adr;
	char *Ptr;

	if (NbLocal = DBGManager_GetNbLocalVariables(Adr = m68k_get_reg(NULL, M68K_REG_PC)))
	{
		if (Ptr = DBGManager_GetFunctionName(Adr))
		{
			if (strcmp(FuncName, Ptr))
			{
				if (FuncName = (char *)realloc(FuncName, strlen(Ptr) + 1))
				{
					strcpy(FuncName, Ptr);

					if (LocalInfo = (WatchInfo *)realloc(LocalInfo, (sizeof(WatchInfo) * NbLocal)))
					{
						for (size_t i = 0; i < NbLocal; i++)
						{
							// Get local variable name and his information
							if (LocalInfo[i].PtrVariableName = DBGManager_GetLocalVariableName(Adr, i + 1))
							{
								LocalInfo[i].Op = DBGManager_GetLocalVariableOp(Adr, i + 1);
								LocalInfo[i].Adr = NULL;
								LocalInfo[i].PtrCPURegisterName = NULL;
								LocalInfo[i].TypeTag = DBGManager_GetLocalVariableTypeTag(Adr, i + 1);
								LocalInfo[i].PtrVariableBaseTypeName = DBGManager_GetLocalVariableTypeName(Adr, i + 1);
								LocalInfo[i].TypeEncoding = DBGManager_GetLocalVariableTypeEncoding(Adr, i + 1);
								LocalInfo[i].TypeByteSize = DBGManager_GetLocalVariableTypeByteSize(Adr, i + 1);
								LocalInfo[i].Offset = DBGManager_GetLocalVariableOffset(Adr, i + 1);
							}
						}
					}
				}
			}

			return true;
		}
	}

	*FuncName = 0;

	return false;
}


//
void LocalBrowserWindow::RefreshContents(void)
{
	char string[1024];
//	char buf[64];
	QString Local;
	char Value[100];
	char *PtrValue;
//	size_t NbWatch, Adr;
//	WatchInfo PtrLocalInfo;

	const char *CPURegName[] = { "D0", "D1", "D2", "D3", "D4", "D5", "D6", "D7", "A0", "A1", "A2", "A3", "A4", "A5", "A6", "A7" };

	if (isVisible())
	{
		if (UpdateInfos())
		{
//#ifdef _MSC_VER
			//#pragma message("Warning: !!! Need to check the memory desalocation for LocalInfo !!!")
//#else
			//#warning "!!! Need to do the memory desalocation for LocalInfo !!!"
//#endif // _MSC_VER
//#ifdef _MSC_VER
			//#pragma message("Warning: !!! Need to check the memory desalocation for FuncName !!!")
//#else
			//#warning "!!! Need to do the memory desalocation for FuncName !!!"
//#endif // _MSC_VER

			for (size_t i = 0; i < NbLocal; i++)
			{
				if (LocalInfo[i].PtrVariableName)
				{
					// Local or parameters variables
					if (((LocalInfo[i].Op >= DBG_OP_breg0) && (LocalInfo[i].Op <= DBG_OP_breg31)) || (LocalInfo[i].Op == DBG_OP_fbreg))
					{
						LocalInfo[i].Adr = m68k_get_reg(NULL, M68K_REG_A6) + LocalInfo[i].Offset;

						if ((LocalInfo[i].Op == DBG_OP_fbreg))
						{
							LocalInfo[i].Adr += 8;
						}

						if ((LocalInfo[i].Adr >= 0) && (LocalInfo[i].Adr < vjs.DRAM_size))
						{
							PtrValue = DBGManager_GetVariableValueFromAdr(LocalInfo[i].Adr, LocalInfo[i].TypeEncoding, LocalInfo[i].TypeByteSize);
						}
						else
						{
							PtrValue = NULL;
						}
					}
					else
					{
						// Value from CPU register
						if ((LocalInfo[i].Op >= DBG_OP_reg0) && (LocalInfo[i].Op <= DBG_OP_reg31))
						{
							LocalInfo[i].PtrCPURegisterName = (char *)CPURegName[(LocalInfo[i].Op - DBG_OP_reg0)];
							PtrValue = itoa(m68k_get_reg(NULL, (m68k_register_t)((size_t)M68K_REG_D0 + (LocalInfo[i].Op - DBG_OP_reg0))), Value, 10);
						}
						else
						{
							PtrValue = NULL;
						}
					}

					if (!LocalInfo[i].Op)
					{
						sprintf(string, "<font color='#A52A2A'>%i : %s | %s | [Not used]</font>", (i + 1), (LocalInfo[i].PtrVariableBaseTypeName ? LocalInfo[i].PtrVariableBaseTypeName : (char *)"<font color='#ff0000'>N/A</font>"), LocalInfo[i].PtrVariableName);
					}
					else
					{
						sprintf(string, "%i : %s | %s | ", (i + 1), (LocalInfo[i].PtrVariableBaseTypeName ? LocalInfo[i].PtrVariableBaseTypeName : (char *)"<font color='#ff0000'>N/A</font>"), LocalInfo[i].PtrVariableName);
						Local += QString(string);
						if ((unsigned int)LocalInfo[i].Adr)
						{
							sprintf(string, "0x%06X", (unsigned int)LocalInfo[i].Adr);
						}
						else
						{
							if (LocalInfo[i].PtrCPURegisterName)
							{
								sprintf(string, "<font color='#0000FF'>%s</font>", LocalInfo[i].PtrCPURegisterName);
							}
							else
							{
								sprintf(string, "%s", (char *)"<font color='#ff0000'>N/A</font>");
							}
						}
						Local += QString(string);
						sprintf(string, " | %s", (!PtrValue ? (char *)"<font color='#ff0000'>N/A</font>" : PtrValue));
					}
					Local += QString(string);
					sprintf(string, "<br>");
					Local += QString(string);
				}
			}

			text->clear();
			text->setText(Local);
		}
		else
		{
			text->clear();
		}
	}
}


// 
void LocalBrowserWindow::keyPressEvent(QKeyEvent * e)
{
	if (e->key() == Qt::Key_Escape)
	{
		hide();
	}
#if 0
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
#endif
}


#if 0
void LocalBrowserWindow::GoToAddress(void)
{
	bool ok;
	QString newAddress = address->text();
	memBase = newAddress.toUInt(&ok, 16);
	RefreshContents();
}
#endif

