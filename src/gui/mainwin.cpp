//
// mainwin.cpp - Qt-based GUI for Virtual Jaguar: Main Application Window
// by James Hammons
// (C) 2009 Underground Software
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  12/23/2009  Created this file
// JLH  12/20/2010  Added settings, menus & toolbars
// JLH  07/05/2011  Added CD BIOS functionality to GUI
// JPM  06/06/2016  Visual Studio support
// JPM  06/19/2016  Soft debugger integration
// JPM  01/11/2017  Added stack browser
// JPM  01/02/2017  Added GPU disassembly
// JPM  02/02/2017  Added DSP disassembly
// JPM  07/12/2017  Added all Watch window
// JPM  08/01/2017  Added heap allocator window
// JPM  08/07/2017  Added memories window
// JPM  08/10/2017  Added a restart feature
// JPM  08/31/2017  Added breakpoints window [Not Supported]
// JPM  09/01/2017  Save position & visibility windows status in the settings
// JPM  09/02/2017  Save size windows in the settings
// JPM  09/05/2017  Added Exception Vector Table window
// JPM  09/06/2017  Added the 'Rx' word to the emulator window name
// JPM  09/12/2017  Added the keybindings in the settings
// JPM  11/04/2017  Added the local window
// JPM  08/31/2018  Added the call stack window
//

// FIXED:
//
// - Add dbl click/enter to select in cart list, ESC to dimiss [DONE]
// - Autoscan/autoload all available BIOS from 'software' folder [DONE]
// - Add 1 key jumping in cartridge list (press 'R', jumps to carts starting
//   with 'R', etc) [DONE]
// - Controller configuration [DONE]
//
// STILL TO BE DONE:
//
// - Fix bug in switching between PAL & NTSC in fullscreen mode.
// - Remove SDL dependencies (sound, mainly) from Jaguar core lib
// - Fix inconsistency with trailing slashes in paths (eeproms needs one,
//   software doesn't)
//
// SFDX CODE: S1E9T8H5M23YS

// Uncomment this for debugging...
//#define DEBUG
//#define DEBUGFOO			// Various tool debugging... but not used
//#define DEBUGTP			// Toolpalette debugging... but not used

#include "mainwin.h"

#include "SDL.h"
#include "app.h"
#include "about.h"
#include "configdialog.h"
#include "controllertab.h"
#include "keybindingstab.h"
#include "filepicker.h"
#include "gamepad.h"
#include "generaltab.h"
#include "glwidget.h"
#include "help.h"
#include "profile.h"
#include "settings.h"
#include "version.h"
#include "emustatus.h"
#include "debug/cpubrowser.h"
#include "debug/m68kdasmbrowser.h"
#include "debug/memorybrowser.h"
#include "debug/stackbrowser.h"
#include "debug/opbrowser.h"
#include "debug/riscdasmbrowser.h"

#include "dac.h"
#include "jaguar.h"
#include "log.h"
#include "file.h"
#include "jagbios.h"
#include "jagbios2.h"
#include "jagcdbios.h"
#include "jagstub2bios.h"
#include "joystick.h"
#include "m68000/m68kinterface.h"

//#include "debugger/VideoWin.h"
#include "debugger/DasmWin.h"
#include "debugger/m68KDasmWin.h"
#include "debugger/GPUDasmWin.h"
#include "debugger/DSPDasmWin.h"
#include "debugger/memory1browser.h"
#include "debugger/brkWin.h"
#include "debugger/exceptionvectortablebrowser.h"
#include "debugger/allwatchbrowser.h"
#include "debugger/localbrowser.h"
#include "debugger/heapallocatorbrowser.h"
#include "debugger/callstackbrowser.h"


// According to SebRmv, this header isn't seen on Arch Linux either... :-/
//#ifdef __GCCWIN32__
// Apparently on win32, usleep() is not pulled in by the usual suspects.
#ifndef _MSC_VER
#include <unistd.h>
#else
#include "_MSC_VER/unistd.h"
#endif // !_MSC_VER
//#endif

// The way BSNES controls things is by setting a timer with a zero
// timeout, sleeping if not emulating anything. Seems there has to be a
// better way.

// It has a novel approach to plugging-in/using different video/audio/input
// methods, can we do something similar or should we just use the built-in
// QOpenGL?

// We're going to try to use the built-in OpenGL support and see how it goes.
// We'll make the VJ core modular so that it doesn't matter what GUI is in
// use, we can drop it in anywhere and use it as-is.

MainWin::MainWin(bool autoRun): running(true), powerButtonOn(false),
	showUntunedTankCircuit(true), cartridgeLoaded(false), CDActive(false),
	pauseForFileSelector(false), loadAndGo(autoRun), scannedSoftwareFolder(false), plzDontKillMyComputer(false)
{
	ReadSettings();

	debugbar = NULL;

	for(int i=0; i<8; i++)
		keyHeld[i] = false;

	// FPS management
	for(int i=0; i<RING_BUFFER_SIZE; i++)
		ringBuffer[i] = 0;

	ringBufferPointer = RING_BUFFER_SIZE - 1;

	// main window
	//if (vjs.softTypeDebugger)
	//{
	//	mainWindowCentrale = new QMdiArea;
	//	setCentralWidget(mainWindowCentrale);
	//}

	WriteLog("Window creation start\n");

	// video output
	videoWidget = new GLWidget(this);
	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	// set central output window
	if (!vjs.softTypeDebugger)
	{
		setCentralWidget(videoWidget);
	}
	else
	{
		mainWindowCentrale = new QMdiArea(this);
		setCentralWidget(mainWindowCentrale);
	}

	setWindowIcon(QIcon(":/res/vj-icon.png"));

	QString title = QString(tr("Virtual Jaguar " VJ_RELEASE_VERSION " Rx"));

	if (vjs.hardwareTypeAlpine)
		title += QString(tr(" - Alpine Mode"));

	if (vjs.softTypeDebugger)
		title += QString(tr(" - Debugger Mode"));

	setWindowTitle(title);

	aboutWin = new AboutWindow(this);
	helpWin = new HelpWindow(this);
	filePickWin = new FilePickerWindow(this);
	memBrowseWin = new MemoryBrowserWindow(this);
	stackBrowseWin = new StackBrowserWindow(this);
	emuStatusWin = new EmuStatusWindow(this);
	cpuBrowseWin = new CPUBrowserWindow(this);
	opBrowseWin = new OPBrowserWindow(this);
	m68kDasmBrowseWin = new M68KDasmBrowserWindow(this);
	riscDasmBrowseWin = new RISCDasmBrowserWindow(this);
	if (vjs.softTypeDebugger)
	{
		//VideoOutputWin = new VideoOutputWindow(this);
		//VideoOutputWin->setCentralWidget()
		//DasmWin = new DasmWindow();
		DasmWin = new DasmWindow(this);
		allWatchBrowseWin = new AllWatchBrowserWindow(this);
		LocalBrowseWin = new LocalBrowserWindow(this);
		heapallocatorBrowseWin = new HeapAllocatorBrowserWindow(this);
		brkWin = new BrkWindow(this);
		exceptionvectortableBrowseWin = new ExceptionVectorTableBrowserWindow(this);
		CallStackBrowseWin = new CallStackBrowserWindow(this);

		mem1BrowseWin = (Memory1BrowserWindow **)calloc(vjs.nbrmemory1browserwindow, sizeof(Memory1BrowserWindow));
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to do the memory desalocation for mem1BrowseWin !!!")
#else
		#warning "!!! Need to do the memory desalocation for mem1BrowseWin !!!"
#endif // _MSC_VER
		for (size_t i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			mem1BrowseWin[i] = new Memory1BrowserWindow(this);
		}

		dasmtabWidget = new QTabWidget(this);
		dasmtabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		dasmtabWidget->addTab(m68kDasmWin = new m68KDasmWindow(this), tr("M68000"));
		dasmtabWidget->addTab(GPUDasmWin = new GPUDasmWindow(this), tr("GPU"));
		dasmtabWidget->addTab(DSPDasmWin = new DSPDasmWindow(this), tr("DSP"));
		////dasmtabWidget->addTab(m68kDasmBrowseWin, tr("M68000"));
		setCentralWidget(dasmtabWidget);

#if 0
		QDockWidget *shapesDockWidget = new QDockWidget(tr("Shapes"));
		shapesDockWidget->setObjectName("shapesDockWidget");
		shapesDockWidget->setWidget(m68kDasmWin);
		//shapesDockWidget->setWidget(treeWidget);
		shapesDockWidget->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		addDockWidget(Qt::LeftDockWidgetArea, shapesDockWidget);
#endif
	}

//	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setUnifiedTitleAndToolBarOnMac(true);

	// Create actions

	quitAppAct = new QAction(QIcon(":/res/exit.png"), tr("E&xit"), this);
//	quitAppAct->setShortcuts(QKeySequence::Quit);
//	quitAppAct->setShortcut(QKeySequence(tr("Alt+x")));
	//quitAppAct->setShortcut(QKeySequence(tr("Ctrl+q")));
	quitAppAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBQUIT].KBSettingValue)));
	quitAppAct->setShortcutContext(Qt::ApplicationShortcut);
	quitAppAct->setStatusTip(tr("Quit Virtual Jaguar"));
	connect(quitAppAct, SIGNAL(triggered()), this, SLOT(close()));

	powerGreen.addFile(":/res/power-off.png", QSize(), QIcon::Normal, QIcon::Off);
	powerGreen.addFile(":/res/power-on-green.png", QSize(), QIcon::Normal, QIcon::On);
	powerRed.addFile(":/res/power-off.png", QSize(), QIcon::Normal, QIcon::Off);
	powerRed.addFile(":/res/power-on-red.png", QSize(), QIcon::Normal, QIcon::On);

	powerAct = new QAction(powerGreen, tr("&Power"), this);
	powerAct->setStatusTip(tr("Powers Jaguar on/off"));
	powerAct->setCheckable(true);
	powerAct->setChecked(false);
	connect(powerAct, SIGNAL(triggered()), this, SLOT(TogglePowerState()));

	QIcon pauseIcon;
	pauseIcon.addFile(":/res/pause-off.png", QSize(), QIcon::Normal, QIcon::Off);
	pauseIcon.addFile(":/res/pause-on.png", QSize(), QIcon::Normal, QIcon::On);
	pauseAct = new QAction(pauseIcon, tr("Pause"), this);
	pauseAct->setStatusTip(tr("Toggles the running state"));
	pauseAct->setCheckable(true);
	pauseAct->setDisabled(true);
	//pauseAct->setShortcut(QKeySequence(tr("Esc")));
	pauseAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBPAUSE].KBSettingValue)));
	pauseAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(pauseAct, SIGNAL(triggered()), this, SLOT(ToggleRunState()));

	zoomActs = new QActionGroup(this);

	x1Act = new QAction(QIcon(":/res/zoom100.png"), tr("Zoom 100%"), zoomActs);
	x1Act->setStatusTip(tr("Set window zoom to 100%"));
	x1Act->setCheckable(true);
	connect(x1Act, SIGNAL(triggered()), this, SLOT(SetZoom100()));

	x2Act = new QAction(QIcon(":/res/zoom200.png"), tr("Zoom 200%"), zoomActs);
	x2Act->setStatusTip(tr("Set window zoom to 200%"));
	x2Act->setCheckable(true);
	connect(x2Act, SIGNAL(triggered()), this, SLOT(SetZoom200()));

	x3Act = new QAction(QIcon(":/res/zoom300.png"), tr("Zoom 300%"), zoomActs);
	x3Act->setStatusTip(tr("Set window zoom to 300%"));
	x3Act->setCheckable(true);
	connect(x3Act, SIGNAL(triggered()), this, SLOT(SetZoom300()));

	tvTypeActs = new QActionGroup(this);

	ntscAct = new QAction(QIcon(":/res/ntsc.png"), tr("NTSC"), tvTypeActs);
	ntscAct->setStatusTip(tr("Sets Jaguar to NTSC mode"));
	ntscAct->setCheckable(true);
	connect(ntscAct, SIGNAL(triggered()), this, SLOT(SetNTSC()));

	palAct = new QAction(QIcon(":/res/pal.png"), tr("PAL"), tvTypeActs);
	palAct->setStatusTip(tr("Sets Jaguar to PAL mode"));
	palAct->setCheckable(true);
	connect(palAct, SIGNAL(triggered()), this, SLOT(SetPAL()));

	blur.addFile(":/res/blur-off.png", QSize(), QIcon::Normal, QIcon::Off);
	blur.addFile(":/res/blur-on.png", QSize(), QIcon::Normal, QIcon::On);

	blurAct = new QAction(blur, tr("Blur"), this);
	blurAct->setStatusTip(tr("Sets OpenGL rendering to GL_NEAREST"));
	blurAct->setCheckable(true);
	connect(blurAct, SIGNAL(triggered()), this, SLOT(ToggleBlur()));

	aboutAct = new QAction(QIcon(":/res/vj-icon.png"), tr("&About..."), this);
	aboutAct->setStatusTip(tr("Blatant self-promotion"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(ShowAboutWin()));

	helpAct = new QAction(QIcon(":/res/vj-icon.png"), tr("&Contents..."), this);
	helpAct->setStatusTip(tr("Help is available, if you should need it"));
	connect(helpAct, SIGNAL(triggered()), this, SLOT(ShowHelpWin()));

	if (!vjs.softTypeDebugger)
	{
		filePickAct = new QAction(QIcon(":/res/software.png"), tr("&Insert Cartridge..."), this);
		filePickAct->setStatusTip(tr("Insert a cartridge into Virtual Jaguar"));
	}
	else
	{
		filePickAct = new QAction(QIcon(":/res/software.png"), tr("&Insert executable file..."), this);
		filePickAct->setStatusTip(tr("Insert an executable into Virtual Jaguar"));
	}
	//filePickAct->setShortcut(QKeySequence(tr("Ctrl+i")));
	filePickAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBPICKFILE].KBSettingValue)));
	filePickAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(filePickAct, SIGNAL(triggered()), this, SLOT(InsertCart()));

	configAct = new QAction(QIcon(":/res/wrench.png"), tr("&Configure"), this);
	configAct->setStatusTip(tr("Configure options for Virtual Jaguar"));
	//configAct->setShortcut(QKeySequence(tr("Ctrl+c")));
	configAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBCONFIGURE].KBSettingValue)));
	configAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(configAct, SIGNAL(triggered()), this, SLOT(Configure()));

	emustatusAct = new QAction(QIcon(":/res/status.png"), tr("&Status"), this);
	emustatusAct->setStatusTip(tr("Emulator status"));
	//emustatusAct->setShortcut(QKeySequence(tr("Ctrl+s")));
	emustatusAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBEMUSTATUS].KBSettingValue)));
	emustatusAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(emustatusAct, SIGNAL(triggered()), this, SLOT(ShowEmuStatusWin()));

	useCDAct = new QAction(QIcon(":/res/compact-disc.png"), tr("&Use CD Unit"), this);
	useCDAct->setStatusTip(tr("Use Jaguar Virtual CD unit"));
//	useCDAct->setShortcut(QKeySequence(tr("Ctrl+c")));
	useCDAct->setCheckable(true);
	connect(useCDAct, SIGNAL(triggered()), this, SLOT(ToggleCDUsage()));

	frameAdvanceAct = new QAction(QIcon(":/res/frame-advance.png"), tr("&Frame Advance"), this);
	//frameAdvanceAct->setShortcut(QKeySequence(tr("F7")));
	frameAdvanceAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBFRAMEADVANCE].KBSettingValue)));
	frameAdvanceAct->setShortcutContext(Qt::ApplicationShortcut);
	frameAdvanceAct->setDisabled(true);
	connect(frameAdvanceAct, SIGNAL(triggered()), this, SLOT(FrameAdvance()));

	if (vjs.softTypeDebugger)
	{
		restartAct = new QAction(QIcon(":/res/debug-restart.png"), tr("&Restart"), this);
		//restartAct->setShortcut(QKeySequence(tr("Ctrl+Shift+F5")));
		restartAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBRESTART].KBSettingValue)));
		restartAct->setShortcutContext(Qt::ApplicationShortcut);
		restartAct->setCheckable(false);
		restartAct->setDisabled(true);
		connect(restartAct, SIGNAL(triggered()), this, SLOT(DebuggerRestart()));

		traceStepOverAct = new QAction(QIcon(":/res/debug-stepover.png"), tr("&Step Over"), this);
		//traceStepOverAct->setShortcut(QKeySequence(tr("F10")));
		traceStepOverAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBSTEPOVER].KBSettingValue)));
		traceStepOverAct->setShortcutContext(Qt::ApplicationShortcut);
		traceStepOverAct->setCheckable(false);
		traceStepOverAct->setDisabled(true);
		connect(traceStepOverAct, SIGNAL(triggered()), this, SLOT(DebuggerTraceStepOver()));

		traceStepIntoAct = new QAction(QIcon(":/res/debug-stepinto.png"), tr("&Step Into"), this);
		//traceStepIntoAct->setShortcut(QKeySequence(tr("F11")));
		traceStepIntoAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBSTEPINTO].KBSettingValue)));
		traceStepIntoAct->setShortcutContext(Qt::ApplicationShortcut);
		traceStepIntoAct->setCheckable(false);
		traceStepIntoAct->setDisabled(true);
		connect(traceStepIntoAct, SIGNAL(triggered()), this, SLOT(DebuggerTraceStepInto()));

		newBreakpointFunctionAct = new QAction(QIcon(""), tr("&Function Breakpoint"), this);
		newBreakpointFunctionAct->setShortcut(QKeySequence(tr("Ctrl+B")));
		connect(newBreakpointFunctionAct, SIGNAL(triggered()), this, SLOT(NewBreakpointFunction()));
	}

	fullScreenAct = new QAction(QIcon(":/res/fullscreen.png"), tr("F&ull Screen"), this);
	//fullScreenAct->setShortcut(QKeySequence(tr("F9")));
	fullScreenAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBFULLSCREEN].KBSettingValue)));
	fullScreenAct->setShortcutContext(Qt::ApplicationShortcut);
	fullScreenAct->setCheckable(true);
	connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(ToggleFullScreen()));

	// Debugger Actions
	if (vjs.softTypeDebugger)
	{
		exceptionVectorTableBrowseAct = new QAction(QIcon(""), tr("Exception Vector Table"), this);
		exceptionVectorTableBrowseAct->setStatusTip(tr("Shows all Exception Vector Table browser window"));
		connect(exceptionVectorTableBrowseAct, SIGNAL(triggered()), this, SLOT(ShowExceptionVectorTableBrowserWin()));

		allWatchBrowseAct = new QAction(QIcon(":/res/debug-watch.png"), tr("All Watch"), this);
		allWatchBrowseAct->setStatusTip(tr("Shows all Watch browser window"));
		connect(allWatchBrowseAct, SIGNAL(triggered()), this, SLOT(ShowAllWatchBrowserWin()));

		LocalBrowseAct = new QAction(QIcon(":/res/debug-local.png"), tr("Local"), this);
		LocalBrowseAct->setStatusTip(tr("Shows Local browser window"));
		connect(LocalBrowseAct, SIGNAL(triggered()), this, SLOT(ShowLocalBrowserWin()));

		heapallocatorBrowseAct = new QAction(QIcon(""), tr("Heap allocator"), this);
		heapallocatorBrowseAct->setStatusTip(tr("Shows the heap allocator browser window"));
		connect(heapallocatorBrowseAct, SIGNAL(triggered()), this, SLOT(ShowHeapAllocatorBrowserWin()));

		CallStackBrowseAct = new QAction(QIcon(":/res/debug-callstack.png"), tr("Call Stack"), this);
		CallStackBrowseAct->setStatusTip(tr("Shows Call Stack browser window"));
		connect(CallStackBrowseAct, SIGNAL(triggered()), this, SLOT(ShowCallStackBrowserWin()));

		mem1BrowseAct = (QAction **)calloc(vjs.nbrmemory1browserwindow, sizeof(QAction));
		QSignalMapper *signalMapper = new QSignalMapper(this);
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to do the memory desalocation for mem1BrowseAct !!!")
#else
#warning "!!! Need to do the memory desalocation for mem1BrowseAct !!!"
#endif // _MSC_VER
		for (int i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			char MB[100];
			sprintf(MB, "Memory %i", (unsigned int)(i+1));
			mem1BrowseAct[i] = new QAction(QIcon(":/res/debug-memory.png"), tr(MB), this);
			mem1BrowseAct[i]->setStatusTip(tr("Shows a Jaguar memory browser window"));
			//mem1BrowseAct[i]->
			//connect(mem1BrowseAct[0], SIGNAL(triggered()), this, SLOT(ShowMemory1BrowserWin(size_t(0))));
			connect(mem1BrowseAct[i], SIGNAL(triggered()), signalMapper, SLOT(map()));
			signalMapper->setMapping(mem1BrowseAct[i], (int)i);
			connect(signalMapper, SIGNAL(mapped(int)), this, SLOT(ShowMemory1BrowserWin(int)));
		}
	}

	// Debugger Browser Actions
	memBrowseAct = new QAction(QIcon(":/res/tool-memory.png"), tr("Memory Browser"), this);
	memBrowseAct->setStatusTip(tr("Shows the Jaguar memory browser window"));
//	memBrowseAct->setCheckable(true);
	connect(memBrowseAct, SIGNAL(triggered()), this, SLOT(ShowMemoryBrowserWin()));

	stackBrowseAct = new QAction(QIcon(":/res/tool-stack.png"), tr("Stack Browser"), this);
	stackBrowseAct->setStatusTip(tr("Shows the Jaguar stack browser window"));
	//	memBrowseAct->setCheckable(true);
	connect(stackBrowseAct, SIGNAL(triggered()), this, SLOT(ShowStackBrowserWin()));

	cpuBrowseAct = new QAction(QIcon(":/res/tool-cpu.png"), tr("CPU Browser"), this);
	cpuBrowseAct->setStatusTip(tr("Shows the Jaguar CPU browser window"));
//	memBrowseAct->setCheckable(true);
	connect(cpuBrowseAct, SIGNAL(triggered()), this, SLOT(ShowCPUBrowserWin()));

	opBrowseAct = new QAction(QIcon(":/res/tool-op.png"), tr("OP Browser"), this);
	opBrowseAct->setStatusTip(tr("Shows the Jaguar OP browser window"));
//	memBrowseAct->setCheckable(true);
	connect(opBrowseAct, SIGNAL(triggered()), this, SLOT(ShowOPBrowserWin()));

	m68kDasmBrowseAct = new QAction(QIcon(":/res/tool-68k-dis.png"), tr("68K Listing Browser"), this);
	m68kDasmBrowseAct->setStatusTip(tr("Shows the 68K disassembly browser window"));
//	memBrowseAct->setCheckable(true);
	connect(m68kDasmBrowseAct, SIGNAL(triggered()), this, SLOT(ShowM68KDasmBrowserWin()));

	riscDasmBrowseAct = new QAction(QIcon(":/res/tool-risc-dis.png"), tr("RISC Listing Browser"), this);
	riscDasmBrowseAct->setStatusTip(tr("Shows the RISC disassembly browser window"));
//	memBrowseAct->setCheckable(true);
	connect(riscDasmBrowseAct, SIGNAL(triggered()), this, SLOT(ShowRISCDasmBrowserWin()));

	if (vjs.softTypeDebugger)
	{
		VideoOutputAct = new QAction(tr("Output Video"), this);
		VideoOutputAct->setStatusTip(tr("Shows the output video window"));
		connect(VideoOutputAct, SIGNAL(triggered()), this, SLOT(ShowVideoOutputWin()));

		DasmAct = new QAction(tr("Disassembly"), this);
		DasmAct->setStatusTip(tr("Shows the disassembly window"));
		connect(DasmAct, SIGNAL(triggered()), this, SLOT(ShowDasmWin()));
	}

	// Misc. connections...
	connect(filePickWin, SIGNAL(RequestLoad(QString)), this, SLOT(LoadSoftware(QString)));
	connect(filePickWin, SIGNAL(FilePickerHiding()), this, SLOT(Unpause()));

	// Create menus

	fileMenu = menuBar()->addMenu(tr("&Jaguar"));
	fileMenu->addAction(powerAct);
	if (!vjs.softTypeDebugger)
	{
		fileMenu->addAction(pauseAct);
		//	fileMenu->addAction(frameAdvanceAct);
	}
	fileMenu->addAction(filePickAct);
	fileMenu->addAction(useCDAct);
	fileMenu->addAction(configAct);
	fileMenu->addAction(emustatusAct);
	fileMenu->addSeparator();
	fileMenu->addAction(quitAppAct);

	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		debugMenu = menuBar()->addMenu(tr("&Debug"));
		if (vjs.softTypeDebugger)
		{
			debugWindowsMenu = debugMenu->addMenu(tr("&Windows"));
			debugWindowExceptionMenu = debugWindowsMenu->addMenu(tr("&Exception"));
			debugWindowExceptionMenu->addAction(exceptionVectorTableBrowseAct);
			debugWindowsMenu->addSeparator();
#if 0
			debugWindowOutputMenu = debugWindowsMenu->addMenu(tr("&Output"));
			debugWindowOutputMenu->addAction(VideoOutputAct);
			debugWindowsMenu->addSeparator();
#endif
			debugWindowsWatchMenu = debugWindowsMenu->addMenu(tr("&Watch"));
			debugWindowsWatchMenu->addAction(allWatchBrowseAct);
			debugWindowsMenu->addAction(LocalBrowseAct);
			debugWindowsMenu->addSeparator();
			debugWindowsMenu->addAction(CallStackBrowseAct);
			debugWindowsMenu->addSeparator();
			debugWindowsMemoryMenu = debugWindowsMenu->addMenu(tr("&Memory"));
			debugWindowsMemoryMenu->addAction(heapallocatorBrowseAct);
			debugWindowsMemoryMenu->addSeparator();
			for (int i = 0; i < vjs.nbrmemory1browserwindow; i++)
			{
				debugWindowsMemoryMenu->addAction(mem1BrowseAct[i]);
			}
			debugWindowsMenu->addSeparator();
			debugWindowsBrowsesMenu = debugWindowsMenu->addMenu(tr("&Browsers"));
			debugWindowsBrowsesMenu->addAction(memBrowseAct);
			debugWindowsBrowsesMenu->addAction(stackBrowseAct);
			debugWindowsBrowsesMenu->addAction(cpuBrowseAct);
			debugWindowsBrowsesMenu->addAction(opBrowseAct);
			debugWindowsBrowsesMenu->addAction(m68kDasmBrowseAct);
			debugWindowsBrowsesMenu->addAction(riscDasmBrowseAct);
			debugMenu->addSeparator();
			debugMenu->addAction(pauseAct);
			debugMenu->addAction(frameAdvanceAct);
			debugMenu->addAction(restartAct);
			debugMenu->addSeparator();
			debugMenu->addAction(traceStepIntoAct);
			debugMenu->addAction(traceStepOverAct);
#if 0
			debugMenu->addSeparator();
			debugNewBreakpointMenu = debugMenu->addMenu(tr("&New Breakpoint"));
			debugNewBreakpointMenu->addAction(newBreakpointFunctionAct);
#endif
			//debugMenu->addSeparator();
			//debugMenu->addAction(DasmAct);
		}
		else
		{
			debugMenu->addAction(memBrowseAct);
			debugMenu->addAction(stackBrowseAct);
			debugMenu->addAction(cpuBrowseAct);
			debugMenu->addAction(opBrowseAct);
			debugMenu->addAction(m68kDasmBrowseAct);
			debugMenu->addAction(riscDasmBrowseAct);
		}
	}

	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(helpAct);
	helpMenu->addAction(aboutAct);

	// Create toolbars

	toolbar = addToolBar(tr("Stuff"));
	toolbar->addAction(powerAct);
	if (!vjs.softTypeDebugger)
	{
		toolbar->addAction(pauseAct);
		toolbar->addAction(frameAdvanceAct);
	}
	toolbar->addAction(filePickAct);
	toolbar->addAction(useCDAct);
	toolbar->addSeparator();
	if (!vjs.softTypeDebugger)
	{
		toolbar->addAction(x1Act);
		toolbar->addAction(x2Act);
		toolbar->addAction(x3Act);
		toolbar->addSeparator();
		toolbar->addAction(ntscAct);
		toolbar->addAction(palAct);
		toolbar->addSeparator();
		toolbar->addAction(blurAct);
		toolbar->addAction(fullScreenAct);
	}
	else
	{
		debuggerbar = addToolBar(tr("&Debugger"));
		debuggerbar->addAction(pauseAct);
		debuggerbar->addAction(frameAdvanceAct);
		debuggerbar->addAction(restartAct);
		debuggerbar->addSeparator();
		debuggerbar->addAction(traceStepIntoAct);
		debuggerbar->addAction(traceStepOverAct);
	}

	if (vjs.hardwareTypeAlpine)
	{
		debugbar = addToolBar(tr("&Debug"));
		debugbar->addAction(memBrowseAct);
		debugbar->addAction(stackBrowseAct);
		debugbar->addAction(cpuBrowseAct);
		debugbar->addAction(opBrowseAct);
		debugbar->addAction(m68kDasmBrowseAct);
		debugbar->addAction(riscDasmBrowseAct);
	}

	// Add actions to the main window, as hiding widgets with them
	// disables them :-P
	addAction(fullScreenAct);
	addAction(quitAppAct);
	addAction(configAct);
	addAction(emustatusAct);
	addAction(pauseAct);
	addAction(filePickAct);
	addAction(frameAdvanceAct);

	//	Create status bar
	statusBar()->showMessage(tr("Ready"));
	ReadUISettings();
	// Do this in case original size isn't correct (mostly for the first-run case)
	ResizeMainWindow();

	WriteLog("Window creation done\n");

	// Create our test pattern NTSC bitmap
	WriteLog("Test pattern 1 bitmap\n");

	QImage tempImg(":/res/test-pattern.jpg");
	QImage tempImgScaled = tempImg.scaled(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT_PAL, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	for(uint32_t y=0; y<VIRTUAL_SCREEN_HEIGHT_PAL; y++)
	{
		const QRgb * scanline = (QRgb *)tempImgScaled.constScanLine(y);

		for(uint32_t x=0; x<VIRTUAL_SCREEN_WIDTH; x++)
		{
			uint32_t pixel = (qRed(scanline[x]) << 24) | (qGreen(scanline[x]) << 16) | (qBlue(scanline[x]) << 8) | 0xFF;
			testPattern[(y * VIRTUAL_SCREEN_WIDTH) + x] = pixel;
		}
	}

	// Create our test pattern PAL bitmap
	WriteLog("Test pattern 2 bitmap\n");

	QImage tempImg2(":/res/test-pattern-pal.jpg");
	QImage tempImgScaled2 = tempImg2.scaled(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT_PAL, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

	for(uint32_t y=0; y<VIRTUAL_SCREEN_HEIGHT_PAL; y++)
	{
		const QRgb * scanline = (QRgb *)tempImgScaled2.constScanLine(y);

		for(uint32_t x=0; x<VIRTUAL_SCREEN_WIDTH; x++)
		{
			uint32_t pixel = (qRed(scanline[x]) << 24) | (qGreen(scanline[x]) << 16) | (qBlue(scanline[x]) << 8) | 0xFF;
			testPattern2[(y * VIRTUAL_SCREEN_WIDTH) + x] = pixel;
		}
	}

	// Set up timer based loop for animation...
	timer = new QTimer(this);
	connect(timer, SIGNAL(timeout()), this, SLOT(Timer()));

	// This isn't very accurate for NTSC: This is early by 40 msec per frame.
	// This is because it's discarding the 0.6666... on the end of the fraction.
	// Alas, 6 doesn't divide cleanly into 10. :-P
//Should we defer this until SyncUI? Probably.
//No, it doesn't work, because it uses setInterval() instead of start()...
//	timer->start(vjs.hardwareTypeNTSC ? 16 : 20);

	// We set this initially, to make VJ behave somewhat as it would if no
	// cart were inserted and the BIOS was set as active...
	jaguarCartInserted = true;
	WriteLog("Virtual Jaguar %s Rx (Last full build was on %s %s)\n", VJ_RELEASE_VERSION, __DATE__, __TIME__);
	WriteLog("VJ: Initializing jaguar subsystem...\n");
	JaguarInit();
//	memcpy(jagMemSpace + 0xE00000, jaguarBootROM, 0x20000);	// Use the stock BIOS
	memcpy(jagMemSpace + 0xE00000, (vjs.biosType == BT_K_SERIES ? jaguarBootROM : jaguarBootROM2), 0x20000);	// Use the stock BIOS

	// Prevent the file scanner from running if filename passed
	// in on the command line...
	if (autoRun)
		return;

	// Load up the default ROM if in Alpine mode:
	if (vjs.hardwareTypeAlpine)
	{
		bool romLoaded = JaguarLoadFile(vjs.alpineROMPath);

		// If regular load failed, try just a straight file load
		// (Dev only! I don't want people to start getting lazy with their releases again! :-P)
		if (!romLoaded)
			romLoaded = AlpineLoadFile(vjs.alpineROMPath);

		if (romLoaded)
			WriteLog("Alpine Mode: Successfully loaded file \"%s\".\n", vjs.alpineROMPath);
		else
			WriteLog("Alpine Mode: Unable to load file \"%s\"!\n", vjs.alpineROMPath);

		// Attempt to load/run the ABS file...
		LoadSoftware(vjs.absROMPath);
		memcpy(jagMemSpace + 0xE00000, jaguarDevBootROM2, 0x20000);	// Use the stub BIOS
		// Prevent the scanner from running...
		return;
	}

	// Load up the default ROM if in Debugger mode:
	if (vjs.softTypeDebugger)
	{
		bool romLoaded = JaguarLoadFile(vjs.debuggerROMPath);

		// If regular load failed, try just a straight file load
		// (Dev only! I don't want people to start getting lazy with their releases again! :-P)
		if (!romLoaded)
			romLoaded = DebuggerLoadFile(vjs.debuggerROMPath);

		if (romLoaded)
			WriteLog("Debugger Mode: Successfully loaded file \"%s\".\n", vjs.debuggerROMPath);
		else
			WriteLog("Debugger Mode: Unable to load file \"%s\"!\n", vjs.debuggerROMPath);

		// Attempt to load/run the ABS file...
		LoadSoftware(vjs.absROMPath);
		memcpy(jagMemSpace + 0xE00000, jaguarDevBootROM2, 0x20000);	// Use the stub BIOS
																	// Prevent the scanner from running...
		return;
	}

	// Run the scanner if nothing passed in and *not* Alpine mode...
	// NB: Really need to look into caching the info scanned in here...
	filePickWin->ScanSoftwareFolder(allowUnknownSoftware);
	scannedSoftwareFolder = true;
}


void MainWin::LoadFile(QString file)
{
	LoadSoftware(file);
}


void MainWin::SyncUI(void)
{
	// Set toolbar buttons/menus based on settings read in (sync the UI)...
	// (Really, this is to sync command line options passed in)
	blurAct->setChecked(vjs.glFilter);
	x1Act->setChecked(zoomLevel == 1);
	x2Act->setChecked(zoomLevel == 2);
	x3Act->setChecked(zoomLevel == 3);
//	running = powerAct->isChecked();
	ntscAct->setChecked(vjs.hardwareTypeNTSC);
	palAct->setChecked(!vjs.hardwareTypeNTSC);
	powerAct->setIcon(vjs.hardwareTypeNTSC ? powerRed : powerGreen);

	fullScreenAct->setChecked(vjs.fullscreen);
	fullScreen = vjs.fullscreen;
	SetFullScreen(fullScreen);

	// Reset the timer to be what was set in the command line (if any):
//	timer->setInterval(vjs.hardwareTypeNTSC ? 16 : 20);
	timer->start(vjs.hardwareTypeNTSC ? 16 : 20);
}


void MainWin::closeEvent(QCloseEvent * event)
{
	JaguarDone();
// This should only be done by the config dialog
//	WriteSettings();
	WriteUISettings();
	event->accept(); // ignore() if can't close for some reason
}


void MainWin::keyPressEvent(QKeyEvent * e)
{
#ifndef VJ_REMOVE_DEV_CODE
	// From jaguar.cpp
	//extern bool startM68KTracing;		// moved to jaguar.h
	// From joystick.cpp
	extern int blit_start_log;
	// From blitter.cpp
	extern bool startConciseBlitLogging;
#endif

	// We ignore the Alt key for now, since it causes problems with the GUI
	if (e->key() == Qt::Key_Alt)
	{
		e->accept();
		return;
	}
// Bar this shite from release versions kthxbai
#ifndef VJ_REMOVE_DEV_CODE
	else if (e->key() == Qt::Key_F11)
	{
		startM68KTracing = true;
		e->accept();
		return;
	}
	else if (e->key() == Qt::Key_F12)
	{
		blit_start_log = true;
		e->accept();
		return;
	}
	else if (e->key() == Qt::Key_F10)
	{
		startConciseBlitLogging = true;
		e->accept();
		return;
	}
#endif
	else if (e->key() == Qt::Key_F8)
	{
		// ggn: For extra NYAN pleasure...
		// ggn: There you go James :P
		// Shamus: Thanks for the patch! :-D
		WriteLog("    o  +           +        +\n");
		WriteLog("+        o     o       +        o\n");
		WriteLog("-_-_-_-_-_-_-_,------,      o \n");
		WriteLog("_-_-_-_-_-_-_-|   /\\_/\\  \n");
		WriteLog("-_-_-_-_-_-_-~|__( ^ .^)  +     +  \n");
		WriteLog("_-_-_-_-_-_-_-\"\"  \"\"      \n");
		WriteLog("+      o         o   +       o\n");
		WriteLog("    +         +\n");
		e->accept();
		return;
	}

/*
This is done now by a QAction...
	if (e->key() == Qt::Key_F9)
	{
		ToggleFullScreen();
		return;
	}
*/
	HandleKeys(e, true);
}


void MainWin::keyReleaseEvent(QKeyEvent * e)
{
	// We ignore the Alt key for now, since it causes problems with the GUI
	if (e->key() == Qt::Key_Alt)
	{
		e->accept();
		return;
	}

	HandleKeys(e, false);
}


void MainWin::HandleKeys(QKeyEvent * e, bool state)
{
	enum { P1LEFT = 0, P1RIGHT, P1UP, P1DOWN, P2LEFT, P2RIGHT, P2UP, P2DOWN };
	// We kill bad key combos here, before they can get to the emulator...
	// This also kills the illegal instruction problem that cropped up in
	// Rayman!

	// First, settle key states...
	if (e->key() == (int)vjs.p1KeyBindings[BUTTON_L])
		keyHeld[P1LEFT] = state;
	else if (e->key() == (int)vjs.p1KeyBindings[BUTTON_R])
		keyHeld[P1RIGHT] = state;
	else if (e->key() == (int)vjs.p1KeyBindings[BUTTON_U])
		keyHeld[P1UP] = state;
	else if (e->key() == (int)vjs.p1KeyBindings[BUTTON_D])
		keyHeld[P1DOWN] = state;
	else if (e->key() == (int)vjs.p2KeyBindings[BUTTON_L])
		keyHeld[P2LEFT] = state;
	else if (e->key() == (int)vjs.p2KeyBindings[BUTTON_R])
		keyHeld[P2RIGHT] = state;
	else if (e->key() == (int)vjs.p2KeyBindings[BUTTON_U])
		keyHeld[P2UP] = state;
	else if (e->key() == (int)vjs.p2KeyBindings[BUTTON_D])
		keyHeld[P2DOWN] = state;

	// Next, check for conflicts and kill 'em if there are any...
	if (keyHeld[P1LEFT] && keyHeld[P1RIGHT])
		keyHeld[P1LEFT] = keyHeld[P1RIGHT] = false;

	if (keyHeld[P1UP] && keyHeld[P1DOWN])
		keyHeld[P1UP] = keyHeld[P1DOWN] = false;

	if (keyHeld[P2LEFT] && keyHeld[P2RIGHT])
		keyHeld[P2LEFT] = keyHeld[P2RIGHT] = false;

	if (keyHeld[P2UP] && keyHeld[P2DOWN])
		keyHeld[P2UP] = keyHeld[P2DOWN] = false;

	// No bad combos exist now, let's stuff the emulator key buffers...!
	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		if (e->key() == (int)vjs.p1KeyBindings[i])
			joypad0Buttons[i] = (state ? 0x01 : 0x00);

		if (e->key() == (int)vjs.p2KeyBindings[i])
			joypad1Buttons[i] = (state ? 0x01 : 0x00);
	}
}


//
// N.B.: The profile system AutoConnect functionality sets the gamepad IDs here.
//
void MainWin::HandleGamepads(void)
{
	Gamepad::Update();

	for(int i=BUTTON_FIRST; i<=BUTTON_LAST; i++)
	{
		if (vjs.p1KeyBindings[i] & (JOY_BUTTON | JOY_HAT | JOY_AXIS))
			joypad0Buttons[i] = (Gamepad::GetState(gamepadIDSlot1, vjs.p1KeyBindings[i]) ? 0x01 : 0x00);

		if (vjs.p2KeyBindings[i] & (JOY_BUTTON | JOY_HAT | JOY_AXIS))
			joypad1Buttons[i] = (Gamepad::GetState(gamepadIDSlot2, vjs.p2KeyBindings[i]) ? 0x01 : 0x00);
	}
}


void MainWin::Open(void)
{
}


void MainWin::Configure(void)
{
	// Call the configuration dialog and update settings
	ConfigDialog dlg(this);
	//ick.
	dlg.generalTab->useUnknownSoftware->setChecked(allowUnknownSoftware);
	dlg.controllerTab1->profileNum = lastEditedProfile;
	dlg.controllerTab1->SetupLastUsedProfile();
// maybe instead of this, we tell the controller tab to work on a copy that gets
// written if the user hits 'OK'.
	SaveProfiles();		// Just in case user cancels

	if (dlg.exec() == false)
	{
		RestoreProfiles();
		return;
	}

	QString before = vjs.ROMPath;
	QString alpineBefore = vjs.alpineROMPath;
	QString absBefore = vjs.absROMPath;
//	bool audioBefore = vjs.audioEnabled;
	bool audioBefore = vjs.DSPEnabled;
	dlg.UpdateVJSettings();
	QString after = vjs.ROMPath;
	QString alpineAfter = vjs.alpineROMPath;
	QString absAfter = vjs.absROMPath;
//	bool audioAfter = vjs.audioEnabled;
	bool audioAfter = vjs.DSPEnabled;

	bool allowOld = allowUnknownSoftware;
	//ick.
	allowUnknownSoftware = dlg.generalTab->useUnknownSoftware->isChecked();
	lastEditedProfile = dlg.controllerTab1->profileNum;
	AutoConnectProfiles();

	// We rescan the "software" folder if the user either changed the path or
	// checked/unchecked the "Allow unknown files" option in the config dialog.
	if ((before != after) || (allowOld != allowUnknownSoftware))
		filePickWin->ScanSoftwareFolder(allowUnknownSoftware);

	// If the "Alpine" ROM is changed, then let's load it...
	if (alpineBefore != alpineAfter)
	{
		if (!JaguarLoadFile(vjs.alpineROMPath) && !AlpineLoadFile(vjs.alpineROMPath))
		{
			// Oh crap, we couldn't get the file! Alert the media!
			QMessageBox msg;
			msg.setText(QString(tr("Could not load file \"%1\"!")).arg(vjs.alpineROMPath));
			msg.setIcon(QMessageBox::Warning);
			msg.exec();
		}
	}

	// If the "ABS" ROM is changed, then let's load it...
	if (absBefore != absAfter)
	{
		if (!JaguarLoadFile(vjs.absROMPath))
		{
			// Oh crap, we couldn't get the file! Alert the media!
			QMessageBox msg;
			msg.setText(QString(tr("Could not load file \"%1\"!")).arg(vjs.absROMPath));
			msg.setIcon(QMessageBox::Warning);
			msg.exec();
		}
	}

	// If the "Enable DSP" checkbox changed, then we have to re-init the DAC,
	// since it's running in the host audio IRQ...
	if (audioBefore != audioAfter)
	{
		DACDone();
		DACInit();
	}

	// Just in case we crash before a clean exit...
	WriteSettings();

	DebuggerRefreshWindows();
}


//
// Here's the main emulator loop
//
void MainWin::Timer(void)
{
#if 0
static uint32_t ntscTickCount;
	if (vjs.hardwareTypeNTSC)
	{
		ntscTickCount++;
		ntscTickCount %= 3;
		timer->start(16 + (ntscTickCount == 0 ? 1 : 0));
	}
#endif

	if (!running)
		return;

	if (showUntunedTankCircuit)
	{
		// Some machines can't handle this, so we give them the option to disable it. :-)
		if (!plzDontKillMyComputer)
		{
//			if (!vjs.softTypeDebugger)
			{
				// Random hash & trash
				// We try to simulate an untuned tank circuit here... :-)
				for (uint32_t x = 0; x < videoWidget->rasterWidth; x++)
				{
					for (uint32_t y = 0; y < videoWidget->rasterHeight; y++)
					{
						videoWidget->buffer[(y * videoWidget->textureWidth) + x] = (rand() & 0xFF) << 8 | (rand() & 0xFF) << 16 | (rand() & 0xFF) << 24;
					}
				}
			}
		}
	}
	else
	{
		// Otherwise, run the Jaguar simulation
		HandleGamepads();
		JaguarExecuteNew();
		if (!vjs.softTypeDebugger)
			videoWidget->HandleMouseHiding();

static uint32_t refresh = 0;
		// Do autorefresh on debug windows
		// Have to be careful, too much causes the emulator to slow way down!
		if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
		{
			if (refresh == vjs.refresh)
			{
				RefreshAlpineWindows();
				//memBrowseWin->RefreshContents();
				//cpuBrowseWin->RefreshContents();
				refresh = 0;
			}
			else
				refresh++;
		}
	}

	//if (!vjs.softTypeDebugger)
		videoWidget->updateGL();

	// FPS handling
	// Approach: We use a ring buffer to store times (in ms) over a given
	// amount of frames, then sum them to figure out the FPS.
	uint32_t timestamp = SDL_GetTicks();
	// This assumes the ring buffer size is a power of 2
//	ringBufferPointer = (ringBufferPointer + 1) & (RING_BUFFER_SIZE - 1);
	// Doing it this way is better. Ring buffer size can be arbitrary then.
	ringBufferPointer = (ringBufferPointer + 1) % RING_BUFFER_SIZE;
	ringBuffer[ringBufferPointer] = timestamp - oldTimestamp;
	uint32_t elapsedTime = 0;

	for(uint32_t i=0; i<RING_BUFFER_SIZE; i++)
		elapsedTime += ringBuffer[i];

	// elapsedTime must be non-zero
	if (elapsedTime == 0)
		elapsedTime = 1;

	// This is in frames per 10 seconds, so we can have 1 decimal
	uint32_t framesPerSecond = (uint32_t)(((float)RING_BUFFER_SIZE / (float)elapsedTime) * 10000.0);
	uint32_t fpsIntegerPart = framesPerSecond / 10;
	uint32_t fpsDecimalPart = framesPerSecond % 10;
	// If this is updated too frequently to be useful, we can throttle it down
	// so that it only updates every 10th frame or so
	statusBar()->showMessage(QString("%1.%2 FPS").arg(fpsIntegerPart).arg(fpsDecimalPart));
	oldTimestamp = timestamp;

	if (M68KDebugHaltStatus())
		ToggleRunState();
}


void MainWin::TogglePowerState(void)
{
	powerButtonOn = !powerButtonOn;
	running = true;

	// With the power off, we simulate white noise on the screen. :-)
	if (!powerButtonOn)
	{
		// Restore the mouse pointer, if hidden:
		if (!vjs.softTypeDebugger)
			videoWidget->CheckAndRestoreMouseCursor();
		useCDAct->setDisabled(false);
		palAct->setDisabled(false);
		ntscAct->setDisabled(false);
		pauseAct->setChecked(false);
		pauseAct->setDisabled(true);
		showUntunedTankCircuit = true;
		DACPauseAudioThread();
		// This is just in case the ROM we were playing was in a narrow or wide
		// field mode, so the untuned tank sim doesn't look wrong. :-)
		TOMReset();

		if (plzDontKillMyComputer)
		{
			//if (!vjs.softTypeDebugger)
			{
				// We have to do it line by line, because the texture pitch is not
				// the same as the picture buffer's pitch.
				for (uint32_t y = 0; y < videoWidget->rasterHeight; y++)
				{
					if (vjs.hardwareTypeNTSC)
						memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
					else
						memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern2 + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
				}
			}
		}
	}
	else
	{
		useCDAct->setDisabled(true);
		palAct->setDisabled(true);
		ntscAct->setDisabled(true);
		pauseAct->setChecked(false);
		pauseAct->setDisabled(false);
		showUntunedTankCircuit = false;

		// Otherwise, we prepare for running regular software...
		if (CDActive)
		{
// Should check for cartridgeLoaded here as well...!
// We can clear it when toggling CDActive on, so that when we power cycle it
// does the expected thing. Otherwise, if we use the file picker to insert a
// cart, we expect to run the cart! Maybe have a RemoveCart function that only
// works if the CD unit is active?
			setWindowTitle(QString("Virtual Jaguar " VJ_RELEASE_VERSION	" Rx - Now playing: Jaguar CD"));
		}

		WriteLog("GUI: Resetting Jaguar...\n");
		JaguarReset();
		DebuggerResetWindows();
		DACPauseAudioThread(false);
	}
}


void MainWin::ToggleRunState(void)
{
	startM68KTracing = running;
	running = !running;

	if (!running)
	{
		frameAdvanceAct->setDisabled(false);
		pauseAct->setChecked(true);
		pauseAct->setDisabled(false);
		if (vjs.softTypeDebugger)
		{
			traceStepIntoAct->setDisabled(false);
			traceStepOverAct->setDisabled(false);
			restartAct->setDisabled(false);
			m68kDasmWin->Use68KPCAddress();
			GPUDasmWin->UseGPUPCAddress();
			DSPDasmWin->UseDSPPCAddress();
		}

		//if (!vjs.softTypeDebugger)
		{
			// Restore the mouse pointer, if hidden:
			videoWidget->CheckAndRestoreMouseCursor();
			//		frameAdvanceAct->setDisabled(false);

			for (uint32_t i = 0; i < (uint32_t)(videoWidget->textureWidth * 256); i++)
			{
				uint32_t pixel = videoWidget->buffer[i];
				uint8_t r = (pixel >> 24) & 0xFF, g = (pixel >> 16) & 0xFF, b = (pixel >> 8) & 0xFF;
				pixel = ((r + g + b) / 3) & 0x00FF;
				videoWidget->buffer[i] = 0x000000FF | (pixel << 16) | (pixel << 8);
			}

			videoWidget->updateGL();

			cpuBrowseWin->HoldBPM();
			cpuBrowseWin->HandleBPMContinue();
			DebuggerRefreshWindows();
		}
	}
	else
	{
		frameAdvanceAct->setDisabled(true);
		pauseAct->setChecked(false);
		pauseAct->setDisabled(false);
		if (vjs.softTypeDebugger)
		{
			traceStepIntoAct->setDisabled(true);
			traceStepOverAct->setDisabled(true);
			restartAct->setDisabled(true);
		}

		cpuBrowseWin->UnholdBPM();
	}

	// Pause/unpause any running/non-running threads...
	DACPauseAudioThread(!running);
}


void MainWin::SetZoom100(void)
{
	zoomLevel = 1;
	ResizeMainWindow();
}


void MainWin::SetZoom200(void)
{
	zoomLevel = 2;
	ResizeMainWindow();
}


void MainWin::SetZoom300(void)
{
	zoomLevel = 3;
	ResizeMainWindow();
}


void MainWin::SetNTSC(void)
{
	powerAct->setIcon(powerRed);
	timer->setInterval(16);
	vjs.hardwareTypeNTSC = true;
	ResizeMainWindow();
	WriteSettings();
}


void MainWin::SetPAL(void)
{
	powerAct->setIcon(powerGreen);
	timer->setInterval(20);
	vjs.hardwareTypeNTSC = false;
	ResizeMainWindow();
	WriteSettings();
}


void MainWin::ToggleBlur(void)
{
	vjs.glFilter = !vjs.glFilter;
	WriteSettings();
}


void MainWin::ShowAboutWin(void)
{
	aboutWin->show();
}


void MainWin::ShowHelpWin(void)
{
	helpWin->show();
}


void MainWin::InsertCart(void)
{
	// Check to see if we did autorun, 'cause we didn't load anything in that
	// case
	if (!scannedSoftwareFolder)
	{
		filePickWin->ScanSoftwareFolder(allowUnknownSoftware);
		scannedSoftwareFolder = true;
	}

	// If the emulator is running, we pause it here and unpause it later
	// if we dismiss the file selector without choosing anything
	if (running && powerButtonOn)
	{
		ToggleRunState();
		pauseForFileSelector = true;
	}

	filePickWin->show();
}


void MainWin::Unpause(void)
{
	// Here we unpause the emulator if it was paused when we went into the file selector
	if (pauseForFileSelector)
	{
		pauseForFileSelector = false;

		// Some nutter might have unpaused while in the file selector, so check for that
		if (!running)
			ToggleRunState();
	}
}


// Jaguar initialisation and load software file
void MainWin::LoadSoftware(QString file)
{
	running = false;							// Prevent bad things(TM) from happening...
	pauseForFileSelector = false;				// Reset the file selector pause flag

	uint8_t * biosPointer = jaguarBootROM;

	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		biosPointer = jaguarDevBootROM2;
	}

	memcpy(jagMemSpace + 0xE00000, biosPointer, 0x20000);

	powerAct->setDisabled(false);
	powerAct->setChecked(true);
	powerButtonOn = false;
	TogglePowerState();

	// We have to load our software *after* the Jaguar RESET
	cartridgeLoaded = JaguarLoadFile(file.toUtf8().data());
	SET32(jaguarMainRAM, 0, vjs.DRAM_size);		// Set top of stack...

	// This is icky because we've already done it
// it gets worse :-P
	if (!vjs.useJaguarBIOS)
	{
		SET32(jaguarMainRAM, 4, jaguarRunAddress);
	}

	m68k_pulse_reset();

// set the M68K in halt mode in case of a debug mode is used, so control is at user side
	if (vjs.softTypeDebugger)
	{
		m68k_set_reg(M68K_REG_A6, 0);
		m68kDasmWin->SetAddress(jaguarRunAddress);
		//pauseAct->setDisabled(false);
		//pauseAct->setChecked(true);
		ToggleRunState();
		//RefreshWindows();
	}
	else
	{
		if (vjs.hardwareTypeAlpine && !jaguarRunAddress)
		{
			ToggleRunState();
		}
	}

	if ((!vjs.hardwareTypeAlpine || !vjs.softTypeDebugger) && !loadAndGo && jaguarRunAddress)
	{
		QString newTitle = QString("Virtual Jaguar " VJ_RELEASE_VERSION " Rx - Now playing: %1").arg(filePickWin->GetSelectedPrettyName());
		setWindowTitle(newTitle);
	}
}


void MainWin::ToggleCDUsage(void)
{
	CDActive = !CDActive;

	// Set up the Jaguar CD for execution, otherwise, clear memory
	if (CDActive)
		memcpy(jagMemSpace + 0x800000, jaguarCDBootROM, 0x40000);
	else
		memset(jagMemSpace + 0x800000, 0xFF, 0x40000);
}


//
void MainWin::NewBreakpointFunction(void)
{
	brkWin->show();
	brkWin->RefreshContents();
}


// Step Into trace
void MainWin::DebuggerTraceStepInto(void)
{
	JaguarStepInto();
	videoWidget->updateGL();
	DebuggerRefreshWindows();
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to verify the Step Into function !!!")
#else
	#warning "!!! Need to verify the Step Into function !!!"
#endif // _MSC_VER
}


// Restart
void MainWin::DebuggerRestart(void)
{
#if 1
	m68k_pulse_reset();
#else
	m68k_set_reg(M68K_REG_PC, jaguarRunAddress);
	m68k_set_reg(M68K_REG_SP, vjs.DRAM_size);
#endif
	m68k_set_reg(M68K_REG_A6, 0);

	DebuggerResetWindows();
	DebuggerRefreshWindows();
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to verify the Restart function !!!")
#else
	#warning "!!! Need to verify the Restart function !!!"
#endif // _MSC_VER
}


// Step Over trace
void MainWin::DebuggerTraceStepOver(void)
{
	JaguarStepOver(0);
	videoWidget->updateGL();
	DebuggerRefreshWindows();
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to verify the Step Over function !!!")
#else
	#warning "!!! Need to verify the Step Over function !!!"
#endif // _MSC_VER
}


// Advance / Execute for one frame
void MainWin::FrameAdvance(void)
{
//printf("Frame Advance...\n");
	ToggleRunState();
	// Execute 1 frame, then exit (only useful in Pause mode)
	JaguarExecuteNew();
	//if (!vjs.softTypeDebugger)
		videoWidget->updateGL();
	ToggleRunState();
	// Need to execute 1 frames' worth of DSP thread as well :-/

	//m68kDasmWin->Use68KPCAddress();
	//RefreshWindows();
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to execute the DSP thread for 1 frame too !!!")
#else
#warning "!!! Need to execute the DSP thread for 1 frame too !!!"
#endif // _MSC_VER
}


void MainWin::SetFullScreen(bool state/*= true*/)
{
	if (!vjs.softTypeDebugger)
	{
		if (state)
		{
			mainWinPosition = pos();
			menuBar()->hide();
			statusBar()->hide();
			toolbar->hide();

			if (debugbar)
				debugbar->hide();

			// This is needed because the fullscreen may happen on a different
			// screen than screen 0:
			int screenNum = QApplication::desktop()->screenNumber(videoWidget);
			QRect r = QApplication::desktop()->screenGeometry(screenNum);
			double targetWidth = (double)VIRTUAL_SCREEN_WIDTH,
				targetHeight = (double)(vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL);
			double aspectRatio = targetWidth / targetHeight;
			// NOTE: Really should check here to see which dimension constrains the
			//       other. Right now, we assume that height is the constraint.
			int newWidth = (int)(aspectRatio * (double)r.height());
			videoWidget->offset = (r.width() - newWidth) / 2;
			videoWidget->fullscreen = true;
			videoWidget->outputWidth = newWidth;
			videoWidget->setFixedSize(r.width(), r.height());
			showFullScreen();
		}
		else
		{
			// Seems Qt is fussy about this: showNormal() has to go first, or it
			// will keep the window stuck in a psuedo-fullscreen mode with no way
			// to get out of it (except closing the app).
			showNormal();

			// Reset the video widget to windowed mode
			videoWidget->offset = 0;
			videoWidget->fullscreen = false;
			menuBar()->show();
			statusBar()->show();
			toolbar->show();

			if (debugbar)
				debugbar->show();

			ResizeMainWindow();
			move(mainWinPosition);
		}
	}
}


void MainWin::ToggleFullScreen(void)
{
	fullScreen = !fullScreen;
	SetFullScreen(fullScreen);
}


// 
void MainWin::ShowExceptionVectorTableBrowserWin(void)
{
	exceptionvectortableBrowseWin->show();
	exceptionvectortableBrowseWin->RefreshContents();
}


// 
void MainWin::ShowLocalBrowserWin(void)
{
	LocalBrowseWin->show();
	LocalBrowseWin->RefreshContents();
}


// 
void MainWin::ShowCallStackBrowserWin(void)
{
	CallStackBrowseWin->show();
	CallStackBrowseWin->RefreshContents();
}


void MainWin::ShowAllWatchBrowserWin(void)
{
	allWatchBrowseWin->show();
	allWatchBrowseWin->RefreshContents();
}


void MainWin::ShowHeapAllocatorBrowserWin(void)
{
	heapallocatorBrowseWin->show();
	heapallocatorBrowseWin->RefreshContents();
}


void MainWin::ShowMemoryBrowserWin(void)
{
	memBrowseWin->show();
	memBrowseWin->RefreshContents();
}


void MainWin::ShowMemory1BrowserWin(int NumWin)
{
//	for (int i = 0; i < vjs.nbrmemory1browserwindow; i++)
	{
		mem1BrowseWin[NumWin]->show();
		mem1BrowseWin[NumWin]->RefreshContents(NumWin);
	}
}


void MainWin::ShowEmuStatusWin(void)
{
	emuStatusWin->show();
	emuStatusWin->RefreshContents();
}


void MainWin::ShowStackBrowserWin(void)
{
	stackBrowseWin->show();
	stackBrowseWin->RefreshContents();
}


void MainWin::ShowCPUBrowserWin(void)
{
	cpuBrowseWin->show();
	cpuBrowseWin->RefreshContents();
}


void MainWin::ShowOPBrowserWin(void)
{
	opBrowseWin->show();
	opBrowseWin->RefreshContents();
}


void MainWin::ShowM68KDasmBrowserWin(void)
{
	m68kDasmBrowseWin->show();
	m68kDasmBrowseWin->RefreshContents();
}


void MainWin::ShowRISCDasmBrowserWin(void)
{
	riscDasmBrowseWin->show();
	riscDasmBrowseWin->RefreshContents();
}


void	MainWin::ShowDasmWin(void)
{
	DasmWin->show();
//	DasmWin->RefreshContents();
}


void MainWin::ShowVideoOutputWin(void)
{
	//VideoOutputWindowCentrale = mainWindowCentrale->addSubWindow(videoWidget);
	//VideoOutputWindowCentrale->setWindowTitle(QString(tr("Video output")));
	//VideoOutputWindowCentrale->show();
	//memBrowseWin->show();
	//VideoOutputWin->show();
	//VideoOutputWin->RefreshContents(videoWidget);
}


void MainWin::ResizeMainWindow(void)
{
	if (!vjs.softTypeDebugger)
	{
		videoWidget->setFixedSize(zoomLevel * VIRTUAL_SCREEN_WIDTH,
			zoomLevel * (vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL));

		// Show the test pattern if user requested plzDontKillMyComputer mode
		if (!powerButtonOn && plzDontKillMyComputer)
		{
			for (uint32_t y = 0; y < videoWidget->rasterHeight; y++)
			{
				if (vjs.hardwareTypeNTSC)
					memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
				else
					memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern2 + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
			}
		}

		adjustSize();
	}
}


// Read settings
void MainWin::ReadSettings(void)
{
	size_t i;

	QSettings settings("Underground Software", "Virtual Jaguar");

	//zoomLevel = settings.value("zoom", 2).toInt();
	allowUnknownSoftware = settings.value("showUnknownSoftware", false).toBool();
	lastEditedProfile = settings.value("lastEditedProfile", 0).toInt();

	vjs.useJoystick = settings.value("useJoystick", false).toBool();
	vjs.joyport = settings.value("joyport", 0).toInt();
	vjs.hardwareTypeNTSC = settings.value("hardwareTypeNTSC", true).toBool();
	vjs.frameSkip = settings.value("frameSkip", 0).toInt();
	vjs.useJaguarBIOS = settings.value("useJaguarBIOS", false).toBool();
	vjs.GPUEnabled = settings.value("GPUEnabled", true).toBool();
	vjs.DSPEnabled = settings.value("DSPEnabled", true).toBool();
	vjs.audioEnabled = settings.value("audioEnabled", true).toBool();
	vjs.usePipelinedDSP = settings.value("usePipelinedDSP", false).toBool();
	vjs.fullscreen = settings.value("fullscreen", false).toBool();
	vjs.useOpenGL = settings.value("useOpenGL", true).toBool();
	vjs.glFilter = settings.value("glFilterType", 1).toInt();
	vjs.renderType = settings.value("renderType", 0).toInt();
	vjs.biosType = settings.value("biosType", BT_M_SERIES).toInt();
	vjs.useFastBlitter = settings.value("useFastBlitter", false).toBool();
	strcpy(vjs.EEPROMPath, settings.value("EEPROMs", QStandardPaths::writableLocation(QStandardPaths::DataLocation).append("/eeproms/")).toString().toUtf8().data());
	strcpy(vjs.ROMPath, settings.value("ROMs", QStandardPaths::writableLocation(QStandardPaths::DataLocation).append("/software/")).toString().toUtf8().data());

	// Read settings from the Debugger mode
	settings.beginGroup("debugger");
	strcpy(vjs.debuggerROMPath, settings.value("DefaultROM", "").toString().toUtf8().data());
	vjs.nbrdisasmlines = settings.value("NbrDisasmLines", 32).toUInt();
	vjs.disasmopcodes = settings.value("DisasmOpcodes", true).toBool();
	vjs.displayHWlabels = settings.value("DisplayHWLabels", true).toBool();
	vjs.displayFullSourceFilename = settings.value("displayFullSourceFilename", true).toBool();
	vjs.nbrmemory1browserwindow = settings.value("NbrMemory1BrowserWindow", MaxMemory1BrowserWindow).toUInt();
	settings.endGroup();

	// Read settings from the Alpine mode
	settings.beginGroup("alpine");
	strcpy(vjs.alpineROMPath, settings.value("DefaultROM", "").toString().toUtf8().data());
	strcpy(vjs.absROMPath, settings.value("DefaultABS", "").toString().toUtf8().data());
	vjs.refresh = settings.value("refresh", 60).toUInt();
	vjs.allowWritesToROM = settings.value("writeROM", false).toBool();
	settings.endGroup();

	// Read settings from the Keybindings
	settings.beginGroup("keybindings");
	for (i = 0; i < KB_END; i++)
	{
		strcpy(vjs.KBContent[i].KBSettingValue, settings.value(KeyBindingsTable[i].KBNameSetting, KeyBindingsTable[i].KBDefaultValue).toString().toUtf8().data());
	}
	settings.endGroup();

	// Write important settings to the log file
	WriteLog("MainWin: Paths\n");
	WriteLog("     EEPROMPath = \"%s\"\n", vjs.EEPROMPath);
	WriteLog("        ROMPath = \"%s\"\n", vjs.ROMPath);
	WriteLog("  AlpineROMPath = \"%s\"\n", vjs.alpineROMPath);
	WriteLog("DebuggerROMPath = \"%s\"\n", vjs.debuggerROMPath);
	WriteLog("     absROMPath = \"%s\"\n", vjs.absROMPath);
	WriteLog("  Pipelined DSP = %s\n", (vjs.usePipelinedDSP ? "ON" : "off"));

#if 0
	// Keybindings in order of U, D, L, R, C, B, A, Op, Pa, 0-9, #, *
	vjs.p1KeyBindings[BUTTON_U] = settings.value("p1k_up", Qt::Key_S).toInt();
	vjs.p1KeyBindings[BUTTON_D] = settings.value("p1k_down", Qt::Key_X).toInt();
	vjs.p1KeyBindings[BUTTON_L] = settings.value("p1k_left", Qt::Key_A).toInt();
	vjs.p1KeyBindings[BUTTON_R] = settings.value("p1k_right", Qt::Key_D).toInt();
	vjs.p1KeyBindings[BUTTON_C] = settings.value("p1k_c", Qt::Key_J).toInt();
	vjs.p1KeyBindings[BUTTON_B] = settings.value("p1k_b", Qt::Key_K).toInt();
	vjs.p1KeyBindings[BUTTON_A] = settings.value("p1k_a", Qt::Key_L).toInt();
	vjs.p1KeyBindings[BUTTON_OPTION] = settings.value("p1k_option", Qt::Key_O).toInt();
	vjs.p1KeyBindings[BUTTON_PAUSE] = settings.value("p1k_pause", Qt::Key_P).toInt();
	vjs.p1KeyBindings[BUTTON_0] = settings.value("p1k_0", Qt::Key_0).toInt();
	vjs.p1KeyBindings[BUTTON_1] = settings.value("p1k_1", Qt::Key_1).toInt();
	vjs.p1KeyBindings[BUTTON_2] = settings.value("p1k_2", Qt::Key_2).toInt();
	vjs.p1KeyBindings[BUTTON_3] = settings.value("p1k_3", Qt::Key_3).toInt();
	vjs.p1KeyBindings[BUTTON_4] = settings.value("p1k_4", Qt::Key_4).toInt();
	vjs.p1KeyBindings[BUTTON_5] = settings.value("p1k_5", Qt::Key_5).toInt();
	vjs.p1KeyBindings[BUTTON_6] = settings.value("p1k_6", Qt::Key_6).toInt();
	vjs.p1KeyBindings[BUTTON_7] = settings.value("p1k_7", Qt::Key_7).toInt();
	vjs.p1KeyBindings[BUTTON_8] = settings.value("p1k_8", Qt::Key_8).toInt();
	vjs.p1KeyBindings[BUTTON_9] = settings.value("p1k_9", Qt::Key_9).toInt();
	vjs.p1KeyBindings[BUTTON_d] = settings.value("p1k_pound", Qt::Key_Minus).toInt();
	vjs.p1KeyBindings[BUTTON_s] = settings.value("p1k_star", Qt::Key_Equal).toInt();

	vjs.p2KeyBindings[BUTTON_U] = settings.value("p2k_up", Qt::Key_Up).toInt();
	vjs.p2KeyBindings[BUTTON_D] = settings.value("p2k_down", Qt::Key_Down).toInt();
	vjs.p2KeyBindings[BUTTON_L] = settings.value("p2k_left", Qt::Key_Left).toInt();
	vjs.p2KeyBindings[BUTTON_R] = settings.value("p2k_right", Qt::Key_Right).toInt();
	vjs.p2KeyBindings[BUTTON_C] = settings.value("p2k_c", Qt::Key_Z).toInt();
	vjs.p2KeyBindings[BUTTON_B] = settings.value("p2k_b", Qt::Key_X).toInt();
	vjs.p2KeyBindings[BUTTON_A] = settings.value("p2k_a", Qt::Key_C).toInt();
	vjs.p2KeyBindings[BUTTON_OPTION] = settings.value("p2k_option", Qt::Key_Apostrophe).toInt();
	vjs.p2KeyBindings[BUTTON_PAUSE] = settings.value("p2k_pause", Qt::Key_Return).toInt();
	vjs.p2KeyBindings[BUTTON_0] = settings.value("p2k_0", Qt::Key_0).toInt();
	vjs.p2KeyBindings[BUTTON_1] = settings.value("p2k_1", Qt::Key_1).toInt();
	vjs.p2KeyBindings[BUTTON_2] = settings.value("p2k_2", Qt::Key_2).toInt();
	vjs.p2KeyBindings[BUTTON_3] = settings.value("p2k_3", Qt::Key_3).toInt();
	vjs.p2KeyBindings[BUTTON_4] = settings.value("p2k_4", Qt::Key_4).toInt();
	vjs.p2KeyBindings[BUTTON_5] = settings.value("p2k_5", Qt::Key_5).toInt();
	vjs.p2KeyBindings[BUTTON_6] = settings.value("p2k_6", Qt::Key_6).toInt();
	vjs.p2KeyBindings[BUTTON_7] = settings.value("p2k_7", Qt::Key_7).toInt();
	vjs.p2KeyBindings[BUTTON_8] = settings.value("p2k_8", Qt::Key_8).toInt();
	vjs.p2KeyBindings[BUTTON_9] = settings.value("p2k_9", Qt::Key_9).toInt();
	vjs.p2KeyBindings[BUTTON_d] = settings.value("p2k_pound", Qt::Key_Slash).toInt();
	vjs.p2KeyBindings[BUTTON_s] = settings.value("p2k_star", Qt::Key_Asterisk).toInt();
#endif

	WriteLog("Read setting = Done\n");

	ReadProfiles(&settings);
}


// Read UI settings
// Default values will be used in case of no settings can be found
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to check the window geometry to see if the positions are legal !!!")
#else
#warning "!!! Need to check the window geometry to see if the positions are legal !!!"
#endif // _MSC_VER
// i.e., someone could drag it to another screen, close it, then disconnect that screen
void MainWin::ReadUISettings(void)
{
	QPoint pos;
	char mem1Name[100];
	size_t i;
	QSize size;

	// Point on the emulator settings
	QSettings settings("Underground Software", "Virtual Jaguar");
	settings.beginGroup("ui");

	// Emulator main window UI information
	mainWinPosition = settings.value("pos", QPoint(200, 200)).toPoint();
	size = settings.value("size", QSize(400, 400)).toSize();
	resize(size);
	move(mainWinPosition);
	pos = settings.value("cartLoadPos", QPoint(200, 200)).toPoint();
	filePickWin->move(pos);

	// Video output information
	zoomLevel = settings.value("zoom", 2).toInt();

	// Alpine debug UI information (also needed by the Debugger)
	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		// CPU registers UI information
		pos = settings.value("cpuBrowseWinPos", QPoint(200, 200)).toPoint();
		cpuBrowseWin->move(pos);
		settings.value("cpuBrowseWinIsVisible", false).toBool() ? ShowCPUBrowserWin() : void();

		// Memory browser UI information
		pos = settings.value("memBrowseWinPos", QPoint(200, 200)).toPoint();
		memBrowseWin->move(pos);
		settings.value("memBrowseWinIsVisible", false).toBool() ? ShowMemoryBrowserWin() : void();

		// Stack browser UI information
		pos = settings.value("stackBrowseWinPos", QPoint(200, 200)).toPoint();
		stackBrowseWin->move(pos);
		settings.value("stackBrowseWinIsVisible", false).toBool() ? ShowStackBrowserWin() : void();
		size = settings.value("stackBrowseWinSize", QSize(400, 400)).toSize();
		stackBrowseWin->resize(size);

		// Emulator status UI information
		pos = settings.value("emuStatusWinPos", QPoint(200, 200)).toPoint();
		emuStatusWin->move(pos);
		settings.value("emuStatusWinIsVisible", false).toBool() ? ShowEmuStatusWin() : void();

		// OP (Object Processor) UI information
		pos = settings.value("opBrowseWinPos", QPoint(200, 200)).toPoint();
		opBrowseWin->move(pos);
		settings.value("opBrowseWinIsVisible", false).toBool() ? ShowOPBrowserWin() : void();
		size = settings.value("opBrowseWinSize", QSize(400, 400)).toSize();
		opBrowseWin->resize(size);

		// RISC disassembly UI information
		pos = settings.value("riscDasmBrowseWinPos", QPoint(200, 200)).toPoint();
		riscDasmBrowseWin->move(pos);
		settings.value("riscDasmBrowseWinIsVisible", false).toBool() ? ShowRISCDasmBrowserWin() : void();

		// M68k disassembly UI information
		pos = settings.value("m68kDasmBrowseWinPos", QPoint(200, 200)).toPoint();
		m68kDasmBrowseWin->move(pos);
		//settings.value("m68kDasmBrowseWinIsVisible", false).toBool() ? ShowM68KDasmBrowserWin() : void();
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to check the M68k disassembly window position crashing !!!")
#else
		#warning "!!! Need to check the M68k disassembly window position crashing !!!"
#endif // _MSC_VER
	}

	// Debugger UI information
	if (vjs.softTypeDebugger)
	{
#if 0
		pos = settings.value("m68kDasmWinPos", QPoint(200, 200)).toPoint();
		m68kDasmWin->move(pos);
		//settings.value("m68kDasmWinIsVisible", false).toBool() ? m68kDasmWin->show() : m68kDasmWin->hide();
		pos = settings.value("GPUDasmWinPos", QPoint(200, 200)).toPoint();
		GPUDasmWin->move(pos);
		pos = settings.value("DSPDasmWinPos", QPoint(200, 200)).toPoint();
		DSPDasmWin->move(pos);
#endif
		// All watch browser UI information
		pos = settings.value("allWatchBrowseWinPos", QPoint(200, 200)).toPoint();
		allWatchBrowseWin->move(pos);
		settings.value("allWatchBrowseWinIsVisible", false).toBool() ? ShowAllWatchBrowserWin() : void();
		size = settings.value("allWatchBrowseWinSize", QSize(400, 400)).toSize();
		allWatchBrowseWin->resize(size);

		// Local browser UI information
		pos = settings.value("LocalBrowseWinPos", QPoint(200, 200)).toPoint();
		LocalBrowseWin->move(pos);
		settings.value("LocalBrowseWinIsVisible", false).toBool() ? ShowLocalBrowserWin() : void();
		size = settings.value("LocalBrowseWinSize", QSize(400, 400)).toSize();
		LocalBrowseWin->resize(size);

		// Heap memory allocation browser UI information
		pos = settings.value("heapallocatorBrowseWinPos", QPoint(200, 200)).toPoint();
		heapallocatorBrowseWin->move(pos);
		settings.value("heapallocatorBrowseWinIsVisible", false).toBool() ? ShowHeapAllocatorBrowserWin() : void();
		size = settings.value("heapallocatorBrowseWinSize", QSize(400, 400)).toSize();
		heapallocatorBrowseWin->resize(size);

		// Exception Vector Table UI Information
		pos = settings.value("exceptionVectorTableBrowseWinPos", QPoint(200, 200)).toPoint();
		exceptionvectortableBrowseWin->move(pos);
		settings.value("exceptionVectorTableBrowseWinIsVisible", false).toBool() ? ShowExceptionVectorTableBrowserWin() : void();
		size = settings.value("exceptionVectorTableBrowseWinSize", QSize(400, 400)).toSize();
		exceptionvectortableBrowseWin->resize(size);

		// Call Stack browser UI information
		pos = settings.value("CallStackBrowseWinPos", QPoint(200, 200)).toPoint();
		CallStackBrowseWin->move(pos);
		settings.value("CallStackBrowseWinIsVisible", false).toBool() ? ShowCallStackBrowserWin() : void();
		size = settings.value("CallStackBrowseWinSize", QSize(400, 400)).toSize();
		CallStackBrowseWin->resize(size);

		// Memories browser UI information
		for (i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			sprintf(mem1Name, "mem1BrowseWinPos[%i]", (unsigned int)i);
			pos = settings.value(mem1Name, QPoint(200, 200)).toPoint();
			mem1BrowseWin[i]->move(pos);
			sprintf(mem1Name, "mem1BrowseWinIsVisible[%i]", (unsigned int)i);
			settings.value(mem1Name, false).toBool() ? ShowMemory1BrowserWin((int)i) : void();
			sprintf(mem1Name, "mem1BrowseWinSize[%i]", (unsigned int)i);
			size = settings.value(mem1Name, QSize(400, 400)).toSize();
			mem1BrowseWin[i]->resize(size);
		}
	}

	settings.endGroup();

	WriteLog("Read UI setting = Done\n");
}


// Save the settings
void MainWin::WriteSettings(void)
{
	size_t i;

	// Point on the emulator settings
	QSettings settings("Underground Software", "Virtual Jaguar");
	//settings.setValue("pos", pos());
	//settings.setValue("size", size());
	//settings.setValue("cartLoadPos", filePickWin->pos());

	//settings.setValue("zoom", zoomLevel);
	settings.setValue("showUnknownSoftware", allowUnknownSoftware);
	settings.setValue("lastEditedProfile", lastEditedProfile);

	settings.setValue("useJoystick", vjs.useJoystick);
	settings.setValue("joyport", vjs.joyport);
	settings.setValue("hardwareTypeNTSC", vjs.hardwareTypeNTSC);
	settings.setValue("frameSkip", vjs.frameSkip);
	settings.setValue("useJaguarBIOS", vjs.useJaguarBIOS);
	settings.setValue("GPUEnabled", vjs.GPUEnabled);
	settings.setValue("DSPEnabled", vjs.DSPEnabled);
	settings.setValue("audioEnabled", vjs.audioEnabled);
	settings.setValue("usePipelinedDSP", vjs.usePipelinedDSP);
	settings.setValue("fullscreen", vjs.fullscreen);
	settings.setValue("useOpenGL", vjs.useOpenGL);
	settings.setValue("glFilterType", vjs.glFilter);
	settings.setValue("renderType", vjs.renderType);
	settings.setValue("biosType", vjs.biosType);
	settings.setValue("useFastBlitter", vjs.useFastBlitter);
	settings.setValue("JagBootROM", vjs.jagBootPath);
	settings.setValue("CDBootROM", vjs.CDBootPath);
	settings.setValue("EEPROMs", vjs.EEPROMPath);
	settings.setValue("ROMs", vjs.ROMPath);

	// Write settings from the Alpine mode
	settings.beginGroup("alpine");
	settings.setValue("refresh", vjs.refresh);
	settings.setValue("DefaultROM", vjs.alpineROMPath);
	settings.setValue("DefaultABS", vjs.absROMPath);
	settings.setValue("writeROM", vjs.allowWritesToROM);
	settings.endGroup();

	// Write settings from the Debugger mode
	settings.beginGroup("debugger");
	settings.setValue("DisplayHWLabels", vjs.displayHWlabels);
	settings.setValue("NbrDisasmLines", vjs.nbrdisasmlines);
	settings.setValue("DisasmOpcodes", vjs.disasmopcodes);
	settings.setValue("displayFullSourceFilename", vjs.displayFullSourceFilename);
	settings.setValue("NbrMemory1BrowserWindow", (unsigned int)vjs.nbrmemory1browserwindow);
	settings.setValue("DefaultROM", vjs.debuggerROMPath);
	settings.endGroup();

	// Write settings from the Keybindings
	settings.beginGroup("keybindings");
	for (i = 0; i < KB_END; i++)
	{
		settings.setValue(KeyBindingsTable[i].KBNameSetting, vjs.KBContent[i].KBSettingValue);
	}
	settings.endGroup();

#if 0
	settings.setValue("p1k_up", vjs.p1KeyBindings[BUTTON_U]);
	settings.setValue("p1k_down", vjs.p1KeyBindings[BUTTON_D]);
	settings.setValue("p1k_left", vjs.p1KeyBindings[BUTTON_L]);
	settings.setValue("p1k_right", vjs.p1KeyBindings[BUTTON_R]);
	settings.setValue("p1k_c", vjs.p1KeyBindings[BUTTON_C]);
	settings.setValue("p1k_b", vjs.p1KeyBindings[BUTTON_B]);
	settings.setValue("p1k_a", vjs.p1KeyBindings[BUTTON_A]);
	settings.setValue("p1k_option", vjs.p1KeyBindings[BUTTON_OPTION]);
	settings.setValue("p1k_pause", vjs.p1KeyBindings[BUTTON_PAUSE]);
	settings.setValue("p1k_0", vjs.p1KeyBindings[BUTTON_0]);
	settings.setValue("p1k_1", vjs.p1KeyBindings[BUTTON_1]);
	settings.setValue("p1k_2", vjs.p1KeyBindings[BUTTON_2]);
	settings.setValue("p1k_3", vjs.p1KeyBindings[BUTTON_3]);
	settings.setValue("p1k_4", vjs.p1KeyBindings[BUTTON_4]);
	settings.setValue("p1k_5", vjs.p1KeyBindings[BUTTON_5]);
	settings.setValue("p1k_6", vjs.p1KeyBindings[BUTTON_6]);
	settings.setValue("p1k_7", vjs.p1KeyBindings[BUTTON_7]);
	settings.setValue("p1k_8", vjs.p1KeyBindings[BUTTON_8]);
	settings.setValue("p1k_9", vjs.p1KeyBindings[BUTTON_9]);
	settings.setValue("p1k_pound", vjs.p1KeyBindings[BUTTON_d]);
	settings.setValue("p1k_star", vjs.p1KeyBindings[BUTTON_s]);

	settings.setValue("p2k_up", vjs.p2KeyBindings[BUTTON_U]);
	settings.setValue("p2k_down", vjs.p2KeyBindings[BUTTON_D]);
	settings.setValue("p2k_left", vjs.p2KeyBindings[BUTTON_L]);
	settings.setValue("p2k_right", vjs.p2KeyBindings[BUTTON_R]);
	settings.setValue("p2k_c", vjs.p2KeyBindings[BUTTON_C]);
	settings.setValue("p2k_b", vjs.p2KeyBindings[BUTTON_B]);
	settings.setValue("p2k_a", vjs.p2KeyBindings[BUTTON_A]);
	settings.setValue("p2k_option", vjs.p2KeyBindings[BUTTON_OPTION]);
	settings.setValue("p2k_pause", vjs.p2KeyBindings[BUTTON_PAUSE]);
	settings.setValue("p2k_0", vjs.p2KeyBindings[BUTTON_0]);
	settings.setValue("p2k_1", vjs.p2KeyBindings[BUTTON_1]);
	settings.setValue("p2k_2", vjs.p2KeyBindings[BUTTON_2]);
	settings.setValue("p2k_3", vjs.p2KeyBindings[BUTTON_3]);
	settings.setValue("p2k_4", vjs.p2KeyBindings[BUTTON_4]);
	settings.setValue("p2k_5", vjs.p2KeyBindings[BUTTON_5]);
	settings.setValue("p2k_6", vjs.p2KeyBindings[BUTTON_6]);
	settings.setValue("p2k_7", vjs.p2KeyBindings[BUTTON_7]);
	settings.setValue("p2k_8", vjs.p2KeyBindings[BUTTON_8]);
	settings.setValue("p2k_9", vjs.p2KeyBindings[BUTTON_9]);
	settings.setValue("p2k_pound", vjs.p2KeyBindings[BUTTON_d]);
	settings.setValue("p2k_star", vjs.p2KeyBindings[BUTTON_s]);
#endif

	WriteProfiles(&settings);
}


// Save the UI settings
void MainWin::WriteUISettings(void)
{
	char mem1Name[100];
	size_t i;

	// Point on the emulator settings
	QSettings settings("Underground Software", "Virtual Jaguar");
	settings.beginGroup("ui");
	
	// Emulator UI information
	settings.setValue("pos", pos());
	settings.setValue("size", size());
	settings.setValue("cartLoadPos", filePickWin->pos());

	// Video output information
	settings.setValue("zoom", zoomLevel);

	// Alpine debug UI information (also needed by the Debugger)
	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		settings.setValue("cpuBrowseWinPos", cpuBrowseWin->pos());
		settings.setValue("cpuBrowseWinIsVisible", cpuBrowseWin->isVisible());
		settings.setValue("memBrowseWinPos", memBrowseWin->pos());
		settings.setValue("memBrowseWinIsVisible", memBrowseWin->isVisible());
		settings.setValue("stackBrowseWinPos", stackBrowseWin->pos());
		settings.setValue("stackBrowseWinIsVisible", stackBrowseWin->isVisible());
		settings.setValue("stackBrowseWinSize", stackBrowseWin->size());
		settings.setValue("emuStatusWinPos", emuStatusWin->pos());
		settings.setValue("emuStatusWinIsVisible", emuStatusWin->isVisible());
		settings.setValue("opBrowseWinPos", opBrowseWin->pos());
		settings.setValue("opBrowseWinIsVisible", opBrowseWin->isVisible());
		settings.setValue("opBrowseWinSize", opBrowseWin->size());
		settings.setValue("riscDasmBrowseWinPos", riscDasmBrowseWin->pos());
		settings.setValue("riscDasmBrowseWinIsVisible", riscDasmBrowseWin->isVisible());
		settings.setValue("m68kDasmBrowseWinPos", m68kDasmBrowseWin->pos());
		settings.setValue("m68kDasmBrowseWinIsVisible", m68kDasmBrowseWin->isVisible());
	}

	// Debugger UI information
	if (vjs.softTypeDebugger)
	{
#if 0
		settings.setValue("m68kDasmWinPos", m68kDasmWin->pos());
		//settings.setValue("m68kDasmWinIsVisible", m68kDasmWin->isVisible());
		settings.setValue("GPUDasmWinPos", GPUDasmWin->pos());
		settings.setValue("DSPDasmWinPos", DSPDasmWin->pos());
#endif
		settings.setValue("allWatchBrowseWinPos", allWatchBrowseWin->pos());
		settings.setValue("allWatchBrowseWinIsVisible", allWatchBrowseWin->isVisible());
		settings.setValue("allWatchBrowseWinSize", allWatchBrowseWin->size());
		settings.setValue("LocalBrowseWinPos", LocalBrowseWin->pos());
		settings.setValue("LocalBrowseWinIsVisible", LocalBrowseWin->isVisible());
		settings.setValue("LocalBrowseWinSize", LocalBrowseWin->size());
		settings.setValue("heapallocatorBrowseWinPos", heapallocatorBrowseWin->pos());
		settings.setValue("heapallocatorBrowseWinIsVisible", heapallocatorBrowseWin->isVisible());
		settings.setValue("heapallocatorBrowseWinSize", heapallocatorBrowseWin->size());
		settings.setValue("exceptionVectorTableBrowseWinPos", exceptionvectortableBrowseWin->pos());
		settings.setValue("exceptionVectorTableBrowseWinIsVisible", exceptionvectortableBrowseWin->isVisible());
		settings.setValue("exceptionVectorTableBrowseWinSize", exceptionvectortableBrowseWin->size());
		settings.setValue("CallStackBrowseWinPos", CallStackBrowseWin->pos());
		settings.setValue("CallStackBrowseWinIsVisible", CallStackBrowseWin->isVisible());
		settings.setValue("CallStackBrowseWinSize", CallStackBrowseWin->size());
		for (i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			sprintf(mem1Name, "mem1BrowseWinPos[%i]", (unsigned int)i);
			settings.setValue(mem1Name, mem1BrowseWin[i]->pos());
			sprintf(mem1Name, "mem1BrowseWinIsVisible[%i]", (unsigned int)i);
			settings.setValue(mem1Name, mem1BrowseWin[i]->isVisible());
			sprintf(mem1Name, "mem1BrowseWinSize[%i]", (unsigned int)i);
			settings.setValue(mem1Name, mem1BrowseWin[i]->size());
		}
	}

	settings.endGroup();
}


// Refresh alpine debug windows
void	MainWin::RefreshAlpineWindows(void)
{
	cpuBrowseWin->RefreshContents();
	memBrowseWin->RefreshContents();
	stackBrowseWin->RefreshContents();
	emuStatusWin->RefreshContents();
	opBrowseWin->RefreshContents();
	riscDasmBrowseWin->RefreshContents();
	m68kDasmBrowseWin->RefreshContents();
}


// Reset soft debugger windows
void MainWin::DebuggerResetWindows(void)
{
	if (vjs.softTypeDebugger)
	{
		allWatchBrowseWin->Reset();
		heapallocatorBrowseWin->Reset();

		//ResetAlpineWindows();
	}
}


// Refresh soft debugger & alpine debug windows
void MainWin::DebuggerRefreshWindows(void)
{
	size_t i;

	if (vjs.softTypeDebugger)
	{
		m68kDasmWin->RefreshContents();
		GPUDasmWin->RefreshContents();
		DSPDasmWin->RefreshContents();
		allWatchBrowseWin->RefreshContents();
		LocalBrowseWin->RefreshContents();
		CallStackBrowseWin->RefreshContents();
		heapallocatorBrowseWin->RefreshContents();
		for (i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			mem1BrowseWin[i]->RefreshContents(i);
		}

		RefreshAlpineWindows();
	}
}
