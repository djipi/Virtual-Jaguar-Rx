//
// mainwin.cpp - Qt-based GUI for Virtual Jaguar: Main Application Window
// by James Hammons
// (C) 2009 Underground Software
//
// Patches
// https://atariage.com/forums/topic/243174-save-states-for-virtual-jaguar-patch/
//
// JLH = James Hammons <jlhamm@acm.org>
// JPM = Jean-Paul Mari <djipi.mari@gmail.com>
//  RG = Richard Goedeken
//  PL = PvtLewis <from Atari Age>
//
// Who  When        What
// ---  ----------  ------------------------------------------------------------
// JLH  12/23/2009  Created this file
// JLH  12/20/2010  Added settings, menus & toolbars
// JLH  07/05/2011  Added CD BIOS functionality to GUI
// JPM   June/2016  Visual Studio support & Soft debugger integration
// JPM        2017  Added stack browser, added GPU/DSP disassembly, added all Watch window
// JPM   Aug./2017  Added heap allocator and memories window, a restart feature, and a [Not Supported] breakpoints window
// JPM  Sept./2017  Save position, size & visibility windows status in the settings; added Exception Vector Table window, the 'Rx' word to the emulator window name, and the keybindings in the settings
// JPM  11/04/2017  Added the local window
// JPM  08/31/2018  Added the call stack window
// JPM  Sept./2018  Added the new Models and BIOS handler, a screenshot feature and source code files browsing
// JPM   Oct./2018  Added search paths in the settings, breakpoints feature, cartridge view menu
// JPM  11/18/2018  Fix crash with non-debugger mode
// JPM  April/2019  Added ELF sections check, added a save memory dump
// JPM   Aug./2019  Update texts descriptions, set cartridge view menu for debugger mode only, added a HW registers browser and source level tracing
// JPM  March/2020  Added the step over for source level tracing
//  RG   Jan./2021  Linux build fixes
// JPM   Apr./2021  Handle number of M68K cycles used in tracing mode, added video output display in a window
// JPM    May/2021  Check missing dll for the tests pattern
// JPM  March/2022  Added cygdrive directory removal setting, a ROM cartridge browser, a GPU/DSP memory browser, added and slightly modified the save state patch from PvtLewis
// JPM   Jan./2024  Use setting for the emulation framerate display
// JPM  07/14/2024  Added a Console standard emulation window
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
// - The source file listing do not need to be refresh more than one time
// - Fix bug in switching between PAL & NTSC in fullscreen mode.
// - Remove SDL dependencies (sound, mainly) from Jaguar core lib
// - Fix inconsistency with trailing slashes in paths (eeproms needs one, software doesn't)
//
// SFDX CODE: S1E9T8H5M23YS

// Uncomment this for debugging...
//#define DEBUG
//#define DEBUGFOO			// Various tool debugging... but not used
//#define DEBUGTP			// Toolpalette debugging... but not used

#include "state.h"
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
#include "stdConsole.h"
#include "debug/cpubrowser.h"
#include "debug/m68kdasmbrowser.h"
#include "debug/memorybrowser.h"
#include "debug/romcartbrowser.h"
#include "debug/stackbrowser.h"
#include "debug/opbrowser.h"
#include "debug/riscdasmbrowser.h"
#include "debug/hwregsbrowser.h"
#include "dac.h"
#include "jaguar.h"
#include "log.h"
#include "file.h"
#ifndef NEWMODELSBIOSHANDLER
#include "jagbios.h"
#include "jagbios2.h"
#include "jagstub2bios.h"
#else
#include "modelsBIOS.h"
#endif
#include "jagcdbios.h"
#include "joystick.h"
#include "m68000/m68kinterface.h"
#include "debugger/DBGManager.h"
#include "debugger/VideoWin.h"
//#include "debugger/DasmWin.h"
#include "debugger/SourcesWin.h"
#include "debugger/m68kDasmWin.h"
#include "debugger/GPUDasmWin.h"
#include "debugger/DSPDasmWin.h"
#include "debugger/memory1browser.h"
#include "debugger/BreakpointsWin.h"
#include "debugger/NewFnctBreakpointWin.h"
#include "debugger/FilesrcListWin.h"
#include "debugger/exceptionvectortablebrowser.h"
#include "debugger/allwatchbrowser.h"
#include "debugger/localbrowser.h"
#include "debugger/heapallocatorbrowser.h"
#include "debugger/callstackbrowser.h"
#include "debugger/CartFilesListWin.h"
#include "debugger/SaveDumpAsWin.h"


// According to SebRmv, this header isn't seen on Arch Linux either... :-/
//#ifdef __GCCWIN32__
// Apparently on win32, usleep() is not pulled in by the usual suspects.
#ifndef _MSC_VER
//#include <unistd.h>
#else
//#include "_MSC_VER/unistd.h"
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

	// windows common features
	aboutWin = new AboutWindow(this);
	helpWin = new HelpWindow(this);
	filePickWin = new FilePickerWindow(this);
	emuStatusWin = new EmuStatusWindow(this);
	stdConsoleWin = new stdConsoleWindow(this);
	
	// windows alpine mode features
	romcartBrowseWin = new ROMCartBrowserWindow(this);
	stackBrowseWin = new StackBrowserWindow(this);
	cpuBrowseWin = new CPUBrowserWindow(this);
	opBrowseWin = new OPBrowserWindow(this);
	m68kDasmBrowseWin = new M68KDasmBrowserWindow(this);
	riscDasmBrowseWin = new RISCDasmBrowserWindow(this);
	hwRegsBrowseWin = new HWRegsBrowserWindow(this);
	for (int i = 0; i < 3; i++)
	{
		memBrowseWin[i] = new MemoryBrowserWindow(this, i);
	}

	// windows debugger mode features
	if (vjs.softTypeDebugger)
	{
		VideoOutputWin = new VideoOutputWindow(this);
		//VideoOutputWin->show();
		//VideoOutputWin->setCentralWidget()
		//DasmWin = new DasmWindow();
		//DasmWin = new DasmWindow(this);
		allWatchBrowseWin = new AllWatchBrowserWindow(this);
		LocalBrowseWin = new LocalBrowserWindow(this);
		heapallocatorBrowseWin = new HeapAllocatorBrowserWindow(this);
		BreakpointsWin = new BreakpointsWindow(this);
		NewFunctionBreakpointWin = new NewFnctBreakpointWindow(this);
		SaveDumpAsWin = new SaveDumpAsWindow(this);
		exceptionvectortableBrowseWin = new ExceptionVectorTableBrowserWindow(this);
		CallStackBrowseWin = new CallStackBrowserWindow(this);
		CartFilesListWin = new CartFilesListWindow(this);

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

		// Setup dock to display source code filenames tree
		QDockWidget *dockFiles = new QDockWidget(tr("Files"), this);
		dockFiles->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		dockFiles->hide();
		addDockWidget(Qt::LeftDockWidgetArea, dockFiles);
		mainWindowCentrale->addAction(dockFiles->toggleViewAction());
		dockFiles->setWidget(FilesrcListWin = new FilesrcListWindow(this));
#if 0
		// Setup dock to display disassembly
		QDockWidget *dockDisasm = new QDockWidget(tr("Disassembly"), this);
		dockDisasm->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
		addDockWidget(Qt::RightDockWidgetArea, dockDisasm);
		mainWindowCentrale->addAction(dockDisasm->toggleViewAction());
		dockDisasm->setWidget(dasmtabWidget = new QTabWidget(this));
#else
		dasmtabWidget = new QTabWidget(this);
#endif
		// Setup disasm tabs
		dasmtabWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
		dasmtabWidget->addTab(SourcesWin = new SourcesWindow(this), tr("Sources"));
		dasmtabWidget->setCurrentIndex(dasmtabWidget->addTab(m68kDasmWin = new m68KDasmWindow(this), tr("M68000")));
		dasmtabWidget->addTab(GPUDasmWin = new GPUDasmWindow(this), tr("GPU"));
		dasmtabWidget->addTab(DSPDasmWin = new DSPDasmWindow(this), tr("DSP"));
		connect(dasmtabWidget, SIGNAL(currentChanged(const int)), this, SLOT(SelectdasmtabWidget(const int)));
#if 1
		setCentralWidget(dasmtabWidget);
#endif
	}

//	videoWidget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
	setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

	setUnifiedTitleAndToolBarOnMac(true);

	// Quit actions
	quitAppAct = new QAction(QIcon(":/res/exit.png"), tr("E&xit"), this);
	quitAppAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBQUIT].KBSettingValue)));
	quitAppAct->setShortcutContext(Qt::ApplicationShortcut);
	quitAppAct->setStatusTip(tr("Quit Virtual Jaguar"));
	connect(quitAppAct, SIGNAL(triggered()), this, SLOT(close()));

	// Power action
	powerGreen.addFile(":/res/power-off.png", QSize(), QIcon::Normal, QIcon::Off);
	powerGreen.addFile(":/res/power-on-green.png", QSize(), QIcon::Normal, QIcon::On);
	powerRed.addFile(":/res/power-off.png", QSize(), QIcon::Normal, QIcon::Off);
	powerRed.addFile(":/res/power-on-red.png", QSize(), QIcon::Normal, QIcon::On);
	powerAct = new QAction(powerGreen, tr("&Power"), this);
	powerAct->setStatusTip(tr("Powers Jaguar on/off"));
	powerAct->setCheckable(true);
	powerAct->setChecked(false);
	connect(powerAct, SIGNAL(triggered()), this, SLOT(TogglePowerState()));

	// Pause action
	QIcon pauseIcon;
	pauseIcon.addFile(":/res/pause-off.png", QSize(), QIcon::Normal, QIcon::Off);
	pauseIcon.addFile(":/res/pause-on.png", QSize(), QIcon::Normal, QIcon::On);
	pauseAct = new QAction(pauseIcon, tr("Pause"), this);
	pauseAct->setStatusTip(tr("Toggles the running state"));
	pauseAct->setCheckable(true);
	pauseAct->setDisabled(true);
	pauseAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBPAUSE].KBSettingValue)));
	pauseAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(pauseAct, SIGNAL(triggered()), this, SLOT(ToggleRunState()));

	// Screenshot action
	screenshotAct = new QAction(QIcon(":/res/screenshot.png"), tr("&Screenshot"), this);
	screenshotAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBSCREENSHOT].KBSettingValue)));
	screenshotAct->setShortcutContext(Qt::ApplicationShortcut);
	screenshotAct->setCheckable(false);
	screenshotAct->setDisabled(false);
	connect(screenshotAct, SIGNAL(triggered()), this, SLOT(MakeScreenshot()));

	// Zoom actions
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

	// TV type actions
	tvTypeActs = new QActionGroup(this);
	ntscAct = new QAction(QIcon(":/res/ntsc.png"), tr("NTSC"), tvTypeActs);
	ntscAct->setStatusTip(tr("Sets Jaguar to NTSC mode"));
	ntscAct->setCheckable(true);
	connect(ntscAct, SIGNAL(triggered()), this, SLOT(SetNTSC()));
	palAct = new QAction(QIcon(":/res/pal.png"), tr("PAL"), tvTypeActs);
	palAct->setStatusTip(tr("Sets Jaguar to PAL mode"));
	palAct->setCheckable(true);
	connect(palAct, SIGNAL(triggered()), this, SLOT(SetPAL()));

	// Blur action
	blur.addFile(":/res/blur-off.png", QSize(), QIcon::Normal, QIcon::Off);
	blur.addFile(":/res/blur-on.png", QSize(), QIcon::Normal, QIcon::On);
	blurAct = new QAction(blur, tr("Blur"), this);
	blurAct->setStatusTip(tr("Sets OpenGL rendering to GL_NEAREST"));
	blurAct->setCheckable(true);
	connect(blurAct, SIGNAL(triggered()), this, SLOT(ToggleBlur()));

	// About action
	aboutAct = new QAction(QIcon(":/res/vj-icon.png"), tr("&About..."), this);
	aboutAct->setStatusTip(tr("Blatant self-promotion"));
	connect(aboutAct, SIGNAL(triggered()), this, SLOT(ShowAboutWin()));

	// Help action
	helpAct = new QAction(QIcon(":/res/vj-icon.png"), tr("&Contents..."), this);
	helpAct->setStatusTip(tr("Help is available, if you should need it"));
	connect(helpAct, SIGNAL(triggered()), this, SLOT(ShowHelpWin()));

	// File pickup action
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
	filePickAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBPICKFILE].KBSettingValue)));
	filePickAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(filePickAct, SIGNAL(triggered()), this, SLOT(InsertCart()));

	// Configuration action
	configAct = new QAction(QIcon(":/res/wrench.png"), tr("&Configure"), this);
	configAct->setStatusTip(tr("Configure options for Virtual Jaguar"));
	configAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBCONFIGURE].KBSettingValue)));
	configAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(configAct, SIGNAL(triggered()), this, SLOT(Configure()));

	// Emulation status action
	emustatusAct = new QAction(QIcon(":/res/status.png"), tr("&Status"), this);
	emustatusAct->setStatusTip(tr("Emulator status"));
	emustatusAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBEMUSTATUS].KBSettingValue)));
	emustatusAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(emustatusAct, SIGNAL(triggered()), this, SLOT(ShowEmuStatusWin()));

	// Use CD action
	useCDAct = new QAction(QIcon(":/res/compact-disc.png"), tr("&Use CD Unit"), this);
	useCDAct->setStatusTip(tr("Use Jaguar Virtual CD unit"));
	useCDAct->setCheckable(true);
	connect(useCDAct, SIGNAL(triggered()), this, SLOT(ToggleCDUsage()));

	// Frame advance action
	frameAdvanceAct = new QAction(QIcon(":/res/frame-advance.png"), tr("&Frame Advance"), this);
	frameAdvanceAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBFRAMEADVANCE].KBSettingValue)));
	frameAdvanceAct->setShortcutContext(Qt::ApplicationShortcut);
	frameAdvanceAct->setDisabled(true);
	connect(frameAdvanceAct, SIGNAL(triggered()), this, SLOT(FrameAdvance()));

	// Fullscreen action
	fullScreenAct = new QAction(QIcon(":/res/fullscreen.png"), tr("F&ull Screen"), this);
	fullScreenAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBFULLSCREEN].KBSettingValue)));
	fullScreenAct->setShortcutContext(Qt::ApplicationShortcut);
	fullScreenAct->setCheckable(true);
	connect(fullScreenAct, SIGNAL(triggered()), this, SLOT(ToggleFullScreen()));

	// Actions dedicated to debugger mode
	if (vjs.softTypeDebugger)
	{
		// Restart
		restartAct = new QAction(QIcon(":/res/debug-restart.png"), tr("&Restart"), this);
		restartAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBRESTART].KBSettingValue)));
		restartAct->setShortcutContext(Qt::ApplicationShortcut);
		restartAct->setCheckable(false);
		restartAct->setDisabled(true);
		connect(restartAct, SIGNAL(triggered()), this, SLOT(DebuggerRestart()));

		// Step over trace
		traceStepOverAct = new QAction(QIcon(":/res/debug-stepover.png"), tr("&Step Over"), this);
		traceStepOverAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBSTEPOVER].KBSettingValue)));
		traceStepOverAct->setShortcutContext(Qt::ApplicationShortcut);
		traceStepOverAct->setCheckable(false);
		traceStepOverAct->setDisabled(true);
		connect(traceStepOverAct, SIGNAL(triggered()), this, SLOT(DebuggerTraceStepOver()));

		// Trace into tracing
		traceStepIntoAct = new QAction(QIcon(":/res/debug-stepinto.png"), tr("&Step Into"), this);
		traceStepIntoAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBSTEPINTO].KBSettingValue)));
		traceStepIntoAct->setShortcutContext(Qt::ApplicationShortcut);
		traceStepIntoAct->setCheckable(false);
		traceStepIntoAct->setDisabled(true);
		connect(traceStepIntoAct, SIGNAL(triggered()), this, SLOT(DebuggerTraceStepInto()));

		// Function breakpoint
		newFunctionBreakpointAct = new QAction(QIcon(""), tr("&Function Breakpoint"), this);
		newFunctionBreakpointAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBFUNCTIONBREAKPOINT].KBSettingValue)));
		connect(newFunctionBreakpointAct, SIGNAL(triggered()), this, SLOT(ShowNewFunctionBreakpointWin()));
		BreakpointsAct = new QAction(QIcon(":/res/debug-breakpoints.png"), tr("&Breakpoints"), this);
		BreakpointsAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBBREAKPOINTS].KBSettingValue)));
		connect(BreakpointsAct, SIGNAL(triggered()), this, SLOT(ShowBreakpointsWin()));
		deleteAllBreakpointsAct = new QAction(QIcon(":/res/debug-deleteallbreakpoints.png"), tr("&Delete All Breakpoints"), this);
		deleteAllBreakpointsAct->setShortcut(QKeySequence(tr(vjs.KBContent[KBDELETEALLBREAKPOINTS].KBSettingValue)));
		connect(deleteAllBreakpointsAct, SIGNAL(triggered()), this, SLOT(DeleteAllBreakpoints()));
		disableAllBreakpointsAct = new QAction(QIcon(":/res/debug-disableallbreakpoints.png"), tr("&Disable All Breakpoints"), this);
		connect(disableAllBreakpointsAct, SIGNAL(triggered()), this, SLOT(DisableAllBreakpoints()));

		// Save dump
		saveDumpAsAct = new QAction(tr("&Save Dump As..."), this);
		saveDumpAsAct->setCheckable(false);
		saveDumpAsAct->setDisabled(false);
		connect(saveDumpAsAct, SIGNAL(triggered()), this, SLOT(ShowSaveDumpAsWin()));

		VideoOutputAct = new QAction(tr("Output Video"), this);
		VideoOutputAct->setStatusTip(tr("Shows the output video window"));
		connect(VideoOutputAct, SIGNAL(triggered()), this, SLOT(ShowVideoOutputWin()));

		//DasmAct = new QAction(tr("Disassembly"), this);
		//DasmAct->setStatusTip(tr("Shows the disassembly window"));
		//connect(DasmAct, SIGNAL(triggered()), this, SLOT(ShowDasmWin()));

		// Exception vector table window
		exceptionVectorTableBrowseAct = new QAction(QIcon(""), tr("Exception Vector Table"), this);
		exceptionVectorTableBrowseAct->setStatusTip(tr("Shows all Exception Vector Table browser window"));
		connect(exceptionVectorTableBrowseAct, SIGNAL(triggered()), this, SLOT(ShowExceptionVectorTableBrowserWin()));

		// All watch variables window
		allWatchBrowseAct = new QAction(QIcon(":/res/debug-watch.png"), tr("All Watch"), this);
		allWatchBrowseAct->setStatusTip(tr("Shows all Watch browser window"));
		connect(allWatchBrowseAct, SIGNAL(triggered()), this, SLOT(ShowAllWatchBrowserWin()));

		// Local variables window
		LocalBrowseAct = new QAction(QIcon(":/res/debug-local.png"), tr("Locals"), this);
		LocalBrowseAct->setStatusTip(tr("Shows Locals browser window"));
		connect(LocalBrowseAct, SIGNAL(triggered()), this, SLOT(ShowLocalBrowserWin()));

		// Heap (memory) allocation window
		heapallocatorBrowseAct = new QAction(QIcon(""), tr("Heap allocator"), this);
		heapallocatorBrowseAct->setStatusTip(tr("Shows the heap allocator browser window"));
		connect(heapallocatorBrowseAct, SIGNAL(triggered()), this, SLOT(ShowHeapAllocatorBrowserWin()));

		// Call stack window
		CallStackBrowseAct = new QAction(QIcon(":/res/debug-callstack.png"), tr("Call Stack"), this);
		CallStackBrowseAct->setStatusTip(tr("Shows Call Stack browser window"));
		connect(CallStackBrowseAct, SIGNAL(triggered()), this, SLOT(ShowCallStackBrowserWin()));

		// Cart files list
		CartFilesListAct = new QAction(QIcon(""), tr("Directory and files"), this);
		CartFilesListAct->setStatusTip(tr("List of the files in the cartridge's directory structure"));
		connect(CartFilesListAct, SIGNAL(triggered()), this, SLOT(ShowCartFilesListWin()));

		// Memory windows
		mem1BrowseAct = (QAction **)calloc(vjs.nbrmemory1browserwindow, sizeof(QAction));
		QSignalMapper *signalMapperMemory1 = new QSignalMapper(this);
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
			connect(mem1BrowseAct[i], SIGNAL(triggered()), signalMapperMemory1, SLOT(map()));
			signalMapperMemory1->setMapping(mem1BrowseAct[i], (int)i);
			connect(signalMapperMemory1, SIGNAL(mapped(int)), this, SLOT(ShowMemory1BrowserWin(int)));
		}
	}

	// Memory browser window action
	memBrowseAct[0] = new QAction(QIcon(":/res/tool-memory.png"), tr("Memory Browser"), this);
	memBrowseAct[0]->setStatusTip(tr("Shows the Jaguar memory browser window"));
	// DSP memory browwer window action
	memBrowseAct[1] = new QAction(QIcon(":/res/tool-dsp-ram.png"), tr("DSP Memory Browser"), this);
	memBrowseAct[1]->setStatusTip(tr("Shows the Jaguar DSP memory browser window"));
	// GPU memory browser window action
	memBrowseAct[2] = new QAction(QIcon(":/res/tool-gpu-ram.png"), tr("GPU Memory Browser"), this);
	memBrowseAct[2]->setStatusTip(tr("Shows the Jaguar GPU memory browser window"));
	// RISC memory browser window action
	QSignalMapper *signalMapperMemory = new QSignalMapper(this);
	for (int i = 0; i < 3; i++)
	{
		connect(memBrowseAct[i], SIGNAL(triggered()), signalMapperMemory, SLOT(map()));
		signalMapperMemory->setMapping(memBrowseAct[i], i);
		connect(signalMapperMemory, SIGNAL(mapped(int)), this, SLOT(ShowMemoryBrowserWin(int)));
	}

	// ROM browser window action
	romcartBrowseAct = new QAction(QIcon(":/res/tool-romcart.png"), tr("ROM Cartridge Browser"), this);
	romcartBrowseAct->setStatusTip(tr("Shows the Jaguar ROM cartridge browser window"));
	connect(romcartBrowseAct, SIGNAL(triggered()), this, SLOT(ShowROMCartBrowserWin()));

	// Stack browser window action
	stackBrowseAct = new QAction(QIcon(":/res/tool-stack.png"), tr("Stack Browser"), this);
	stackBrowseAct->setStatusTip(tr("Shows the Jaguar stack browser window"));
	connect(stackBrowseAct, SIGNAL(triggered()), this, SLOT(ShowStackBrowserWin()));

	// CPUs (M68000, GPU & DSP browser window action
	cpuBrowseAct = new QAction(QIcon(":/res/tool-cpu.png"), tr("CPU Browser"), this);
	cpuBrowseAct->setStatusTip(tr("Shows the Jaguar CPU browser window"));
	connect(cpuBrowseAct, SIGNAL(triggered()), this, SLOT(ShowCPUBrowserWin()));

	// OP browser window action
	opBrowseAct = new QAction(QIcon(":/res/tool-op.png"), tr("OP Browser"), this);
	opBrowseAct->setStatusTip(tr("Shows the Jaguar OP browser window"));
	connect(opBrowseAct, SIGNAL(triggered()), this, SLOT(ShowOPBrowserWin()));

	// M68000 disassembly browser window
	m68kDasmBrowseAct = new QAction(QIcon(":/res/tool-68k-dis.png"), tr("68K Listing Browser"), this);
	m68kDasmBrowseAct->setStatusTip(tr("Shows the 68K disassembly browser window"));
	connect(m68kDasmBrowseAct, SIGNAL(triggered()), this, SLOT(ShowM68KDasmBrowserWin()));

	// HW registers browser window
	hwRegsBrowseAct = new QAction(QIcon(":/res/tool-hw-regs.png"), tr("HW Registers Browser"), this);
	hwRegsBrowseAct->setStatusTip(tr("Shows the HW registers browser window"));
	connect(hwRegsBrowseAct, SIGNAL(triggered()), this, SLOT(ShowHWRegsBrowserWin()));

	// Risc (DSP / GPU) disassembly browser window
	riscDasmBrowseAct = new QAction(QIcon(":/res/tool-risc-dis.png"), tr("RISC Listing Browser"), this);
	riscDasmBrowseAct->setStatusTip(tr("Shows the RISC disassembly browser window"));
	connect(riscDasmBrowseAct, SIGNAL(triggered()), this, SLOT(ShowRISCDasmBrowserWin()));

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

	// Alpine and debugger menus
	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		// Create debug menu
		debugMenu = menuBar()->addMenu(tr("&Debug"));

		// Create debugger menu
		if (vjs.softTypeDebugger)
		{
			// Create view menu
			viewMenu = menuBar()->addMenu(tr("&View"));

			// Cart menu
			viewCartMenu = viewMenu->addMenu(tr("&Cartridge"));
			viewCartMenu->addAction(CartFilesListAct);
#if 0
			viewCartMenu->addSeparator();
			viewCartMenu->addAction(CartStreamsAct);
#endif

			// Windows menu
			debugWindowsMenu = debugMenu->addMenu(tr("&Windows"));
			debugWindowsMenu->addAction(BreakpointsAct);
			debugWindowExceptionMenu = debugWindowsMenu->addMenu(tr("&Exception"));
			debugWindowExceptionMenu->addAction(exceptionVectorTableBrowseAct);
			debugWindowsMenu->addSeparator();
			debugWindowOutputMenu = debugWindowsMenu->addMenu(tr("&Output"));
			debugWindowOutputMenu->addAction(VideoOutputAct);
			debugWindowsMenu->addSeparator();
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
			debugWindowsBrowsesMenu->addAction(memBrowseAct[0]);
			debugWindowsBrowsesMenu->addAction(stackBrowseAct);
			debugWindowsBrowsesMenu->addAction(cpuBrowseAct);
			debugWindowsBrowsesMenu->addAction(opBrowseAct);
			debugWindowsBrowsesMenu->addAction(m68kDasmBrowseAct);
			debugWindowsBrowsesMenu->addAction(riscDasmBrowseAct);
			debugWindowsBrowsesMenu->addAction(hwRegsBrowseAct);
			debugWindowsBrowsesMenu->addAction(romcartBrowseAct);
			debugWindowsBrowsesMenu->addAction(memBrowseAct[1]);
			debugWindowsBrowsesMenu->addAction(memBrowseAct[2]);
			debugMenu->addSeparator();
			debugMenu->addAction(pauseAct);
			debugMenu->addAction(frameAdvanceAct);
			debugMenu->addAction(restartAct);
			debugMenu->addSeparator();
			debugMenu->addAction(traceStepIntoAct);
			debugMenu->addAction(traceStepOverAct);
			debugMenu->addSeparator();
			debugNewBreakpointMenu = debugMenu->addMenu(tr("&New Breakpoint"));
			debugNewBreakpointMenu->addAction(newFunctionBreakpointAct);
			debugMenu->addAction(deleteAllBreakpointsAct);
			debugMenu->addAction(disableAllBreakpointsAct);
			debugMenu->addSeparator();
			debugMenu->addAction(saveDumpAsAct);
#if 0
			debugMenu->addSeparator();
			debugMenu->addAction(DasmAct);
#endif
		}
		else
		{
			// Create alpine menu
			debugMenu->addAction(memBrowseAct[0]);
			debugMenu->addAction(stackBrowseAct);
			debugMenu->addAction(cpuBrowseAct);
			debugMenu->addAction(opBrowseAct);
			debugMenu->addAction(m68kDasmBrowseAct);
			debugMenu->addAction(riscDasmBrowseAct);
			debugMenu->addAction(hwRegsBrowseAct);
			debugMenu->addAction(romcartBrowseAct);
			debugMenu->addAction(memBrowseAct[1]);
			debugMenu->addAction(memBrowseAct[2]);
		}
	}

	// Help menus
	helpMenu = menuBar()->addMenu(tr("&Help"));
	helpMenu->addAction(helpAct);
	helpMenu->addAction(aboutAct);
	
#if defined(SAVESTATEPATCH_PvtLewis)
	toolsMenu = menuBar()->addMenu(tr("&Tools"));
	dumpAct = new QAction(QIcon(":/res/software.png"), tr("&Dump Save State..."), this);
	dumpAct->setStatusTip(tr("Dump Save State"));
	dumpAct->setShortcut(QKeySequence(tr("F5")));
	dumpAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(dumpAct, SIGNAL(triggered()), this, SLOT(DumpCommand()));
	loadAct = new QAction(QIcon(":/res/software.png"), tr("&Load Save State..."), this);
	loadAct->setStatusTip(tr("Load Save State"));
	loadAct->setShortcut(QKeySequence(tr("F8")));
	loadAct->setShortcutContext(Qt::ApplicationShortcut);
	connect(loadAct, SIGNAL(triggered()), this, SLOT(LoadCommand()));
	saveSlot0Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 0..."), this);
	saveSlot0Act->setStatusTip(tr("Load Save State"));
	saveSlot0Act->setShortcut(QKeySequence(tr("Shift+0")));
	saveSlot0Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot0Act->setCheckable(true);
	saveSlot0Act->setChecked(true);
	connect(saveSlot0Act, SIGNAL(triggered()), this, SLOT(SaveSlot0Command()));
	saveSlot1Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 1..."), this);
	saveSlot1Act->setStatusTip(tr("Load Save State"));
	saveSlot1Act->setShortcut(QKeySequence(tr("Shift+1")));
	saveSlot1Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot1Act->setCheckable(true);
	saveSlot1Act->setChecked(false);
	connect(saveSlot1Act, SIGNAL(triggered()), this, SLOT(SaveSlot1Command()));
	saveSlot2Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 2..."), this);
	saveSlot2Act->setStatusTip(tr("Load Save State"));
	saveSlot2Act->setShortcut(QKeySequence(tr("Shift+2")));
	saveSlot2Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot2Act->setCheckable(true);
	saveSlot2Act->setChecked(false);
	connect(saveSlot2Act, SIGNAL(triggered()), this, SLOT(SaveSlot2Command()));
	saveSlot3Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 3..."), this);
	saveSlot3Act->setStatusTip(tr("Load Save State"));
	saveSlot3Act->setShortcut(QKeySequence(tr("Shift+3")));
	saveSlot3Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot3Act->setCheckable(true);
	saveSlot3Act->setChecked(false);
	connect(saveSlot3Act, SIGNAL(triggered()), this, SLOT(SaveSlot3Command()));
	saveSlot4Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 4..."), this);
	saveSlot4Act->setStatusTip(tr("Load Save State"));
	saveSlot4Act->setShortcut(QKeySequence(tr("Shift+4")));
	saveSlot4Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot4Act->setCheckable(true);
	saveSlot4Act->setChecked(false);
	connect(saveSlot4Act, SIGNAL(triggered()), this, SLOT(SaveSlot4Command()));
	saveSlot5Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 5..."), this);
	saveSlot5Act->setStatusTip(tr("Load Save State"));
	saveSlot5Act->setShortcut(QKeySequence(tr("Shift+5")));
	saveSlot5Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot5Act->setCheckable(true);
	saveSlot5Act->setChecked(false);
	connect(saveSlot5Act, SIGNAL(triggered()), this, SLOT(SaveSlot5Command()));
	saveSlot6Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 6..."), this);
	saveSlot6Act->setStatusTip(tr("Load Save State"));
	saveSlot6Act->setShortcut(QKeySequence(tr("Shift+6")));
	saveSlot6Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot6Act->setCheckable(true);
	saveSlot6Act->setChecked(false);
	connect(saveSlot6Act, SIGNAL(triggered()), this, SLOT(SaveSlot6Command()));
	saveSlot7Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 7..."), this);
	saveSlot7Act->setStatusTip(tr("Load Save State"));
	saveSlot7Act->setShortcut(QKeySequence(tr("Shift+7")));
	saveSlot7Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot7Act->setCheckable(true);
	saveSlot7Act->setChecked(false);
	connect(saveSlot7Act, SIGNAL(triggered()), this, SLOT(SaveSlot7Command()));
	saveSlot8Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 8..."), this);
	saveSlot8Act->setStatusTip(tr("Load Save State"));
	saveSlot8Act->setShortcut(QKeySequence(tr("Shift+8")));
	saveSlot8Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot8Act->setCheckable(true);
	saveSlot8Act->setChecked(false);
	connect(saveSlot8Act, SIGNAL(triggered()), this, SLOT(SaveSlot8Command()));
	saveSlot9Act = new QAction(QIcon(":/res/software.png"), tr("&Save Slot 9..."), this);
	saveSlot9Act->setStatusTip(tr("Load Save State"));
	saveSlot9Act->setShortcut(QKeySequence(tr("Shift+9")));
	saveSlot9Act->setShortcutContext(Qt::ApplicationShortcut);
	saveSlot9Act->setCheckable(true);
	saveSlot9Act->setChecked(false);
	connect(saveSlot9Act, SIGNAL(triggered()), this, SLOT(SaveSlot9Command()));

	toolsMenu->addAction(dumpAct);
	toolsMenu->addAction(loadAct);
	toolsMenu->addAction(saveSlot0Act);
	toolsMenu->addAction(saveSlot1Act);
	toolsMenu->addAction(saveSlot2Act);
	toolsMenu->addAction(saveSlot3Act);
	toolsMenu->addAction(saveSlot4Act);
	toolsMenu->addAction(saveSlot5Act);
	toolsMenu->addAction(saveSlot6Act);
	toolsMenu->addAction(saveSlot7Act);
	toolsMenu->addAction(saveSlot8Act);
	toolsMenu->addAction(saveSlot9Act);

        // Add to the main window so that they work in full screen mode
	this->addAction(dumpAct);
	this->addAction(loadAct);
	this->addAction(saveSlot0Act);
	this->addAction(saveSlot1Act);
	this->addAction(saveSlot2Act);
	this->addAction(saveSlot3Act);
	this->addAction(saveSlot4Act);
	this->addAction(saveSlot5Act);
	this->addAction(saveSlot6Act);
	this->addAction(saveSlot7Act);
	this->addAction(saveSlot8Act);
	this->addAction(saveSlot9Act);
#endif

	// Create toolbars
	toolbar = addToolBar(tr("System"));
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
		toolbar->addAction(screenshotAct);
		toolbar->addSeparator();
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
		debuggerbar->addSeparator();
		debuggerbar->addAction(BreakpointsAct);
	}

	if (vjs.hardwareTypeAlpine)
	{
		debugbar = addToolBar(tr("&Debug"));
		debugbar->addAction(memBrowseAct[0]);
		debugbar->addAction(stackBrowseAct);
		debugbar->addAction(cpuBrowseAct);
		debugbar->addAction(opBrowseAct);
		debugbar->addAction(m68kDasmBrowseAct);
		debugbar->addAction(riscDasmBrowseAct);
		debugbar->addAction(hwRegsBrowseAct);
		debugbar->addAction(romcartBrowseAct);
		debugbar->addAction(memBrowseAct[1]);
		debugbar->addAction(memBrowseAct[2]);
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
	QImage tempImg(":/res/test-pattern.jpg");
	if (!tempImg.isNull())
	{
		QImage tempImgScaled = tempImg.scaled(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT_PAL, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		for (uint32_t y = 0; y < VIRTUAL_SCREEN_HEIGHT_PAL; y++)
		{
			const QRgb * scanline = (QRgb *)tempImgScaled.constScanLine(y);

			for (uint32_t x = 0; x < VIRTUAL_SCREEN_WIDTH; x++)
			{
				uint32_t pixel = (qRed(scanline[x]) << 24) | (qGreen(scanline[x]) << 16) | (qBlue(scanline[x]) << 8) | 0xFF;
				testPattern[(y * VIRTUAL_SCREEN_WIDTH) + x] = pixel;
			}
		}

		WriteLog("Test pattern 1 bitmap sucessful\n");
	}
	else
	{
		WriteLog("Test pattern 1 bitmap failed, qjpeg.dll or qjpegd.dll is missing in the imageformats directory\n");
	}

	// Create our test pattern PAL bitmap
	QImage tempImg2(":/res/test-pattern-pal.jpg");
	if (!tempImg2.isNull())
	{
		QImage tempImgScaled2 = tempImg2.scaled(VIRTUAL_SCREEN_WIDTH, VIRTUAL_SCREEN_HEIGHT_PAL, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);

		for (uint32_t y = 0; y < VIRTUAL_SCREEN_HEIGHT_PAL; y++)
		{
			const QRgb * scanline = (QRgb *)tempImgScaled2.constScanLine(y);

			for (uint32_t x = 0; x < VIRTUAL_SCREEN_WIDTH; x++)
			{
				uint32_t pixel = (qRed(scanline[x]) << 24) | (qGreen(scanline[x]) << 16) | (qBlue(scanline[x]) << 8) | 0xFF;
				testPattern2[(y * VIRTUAL_SCREEN_WIDTH) + x] = pixel;
			}
		}

		WriteLog("Test pattern 2 bitmap sucessful\n");
	}
	else
	{
		WriteLog("Test pattern 2 bitmap failed, qjpeg.dll or qjpegd.dll is missing in the imageformats directory\n");
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

#ifndef NEWMODELSBIOSHANDLER
	//	memcpy(jagMemSpace + 0xE00000, jaguarBootROM, 0x20000);	// Use the stock BIOS
	memcpy(jagMemSpace + 0xE00000, (vjs.biosType == BT_K_SERIES ? jaguarBootROM : jaguarBootROM2), 0x20000);	// Use the stock BIOS
#else
	SelectBIOS(vjs.biosType);
#endif

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
#ifndef NEWMODELSBIOSHANDLER
		memcpy(jagMemSpace + 0xE00000, jaguarDevBootROM2, 0x20000);	// Use the stub BIOS
#else
		SelectBIOS(vjs.biosType);
#endif
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
#ifndef NEWMODELSBIOSHANDLER
		memcpy(jagMemSpace + 0xE00000, jaguarDevBootROM2, 0x20000);	// Use the stub BIOS
																	// Prevent the scanner from running...
#else
		SelectBIOS(vjs.biosType);
#endif
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


// 
void MainWin::SelectdasmtabWidget(const int Index)
{
	// check sources tab
	if (Index == 0)
	{
		SourcesWin->RefreshContents();
	}
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

	RefreshWindows();
}


// Main emulator loop no matter if the executable binary is in pause or running
// The loop won't execute anything in case of the emulator is not running
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
	// emulator must be running
	if (running)
	{
		// check executable binary status
		if (showUntunedTankCircuit)
		{
			// the executable binary is in pause mode, will make the video screen snowing by user acceptance
			if (!plzDontKillMyComputer)
			{
				// fill video screen with random hash & trash, to try to simulate an untuned tank circuit here
				for (uint32_t x = 0; x < videoWidget->rasterWidth; x++)
				{
					for (uint32_t y = 0; y < videoWidget->rasterHeight; y++)
					{
						videoWidget->buffer[(y * videoWidget->textureWidth) + x] = (rand() & 0xFF) << 8 | (rand() & 0xFF) << 16 | (rand() & 0xFF) << 24;
					}
				}
			}
		}
		else
		{
			// otherwise, run the Jaguar simulation
			HandleGamepads();
			JaguarExecuteNew();
			videoWidget->HandleMouseHiding();
			// auto-refresh specfic debug windows, lower refresh value can slow down the emulator
			static uint32_t refresh = 0;
			if (refresh++ == vjs.refresh)
			{
				if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
				{
					AlpineRefreshWindows();
				}

				refresh = 0;
			}
		}

		videoWidget->updateGL();

		// FPS handling, uses a ring buffer to store times (in ms) between frames
		uint32_t timestamp = SDL_GetTicks();
		ringBufferPointer = (ringBufferPointer + 1) % RING_BUFFER_SIZE;
		ringBuffer[ringBufferPointer] = timestamp - oldTimestamp;
		oldTimestamp = timestamp;
		// calculus the elapsed time
		uint32_t elapsedTime = 0;
		for (uint32_t i = 0; i < RING_BUFFER_SIZE; i++)
		{
			elapsedTime += ringBuffer[i];
		}
		// elpased time cannot be nul, to avoid division by 0
		if (elapsedTime == 0)
		{
			elapsedTime = 1;
		}
		// get number of FPS based on elapsed time per block of 10 seconds
		uint32_t framesPerSecond = (uint32_t)(((float)RING_BUFFER_SIZE / (float)elapsedTime) * 10000.0);
		uint32_t fpsIntegerPart = framesPerSecond / 10;
		uint32_t fpsDecimalPart = framesPerSecond % 10;
		// display number of FPS
		vjs.useDisplayEmuFPS ? statusBar()->showMessage(QString("%1.%2 FPS").arg(fpsIntegerPart).arg(fpsDecimalPart)) : statusBar()->showMessage(QString("FPS: Off"));

		// toggle the state of the emulator in case of M68K is set to halt (for tracing mode)
		if (M68KDebugHaltStatus())
		{
			ToggleRunState();
		}

		// refresh window with minimal impact on the emulation speed 
		CommonRefreshWindows();
	}
}


// Toggle the power state, it can be either on or off
void MainWin::TogglePowerState(void)
{
	running = true;

	// switch power on/off
	powerButtonOn = !powerButtonOn;
	if (!powerButtonOn)
	{
		// restore the mouse pointer, if hidden:
		videoWidget->CheckAndRestoreMouseCursor();
		// enable specfic feature available when binary is not running
		useCDAct->setDisabled(false);
		palAct->setDisabled(false);
		ntscAct->setDisabled(false);
		pauseAct->setChecked(false);
		pauseAct->setDisabled(true);

		showUntunedTankCircuit = true;

		DACPauseAudioThread();
		// This is just in case the ROM we were playing was in a narrow or wide field mode, so the untuned tank sim doesn't look wrong. :-)
		TOMReset();

		if (plzDontKillMyComputer)
		{
			// We have to do it line by line, because the texture pitch is not the same as the picture buffer's pitch.
			for (uint32_t y = 0; y < videoWidget->rasterHeight; y++)
			{
				if (vjs.hardwareTypeNTSC)
				{
					memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
				}
				else
				{
					memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern2 + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
				}
			}
		}
	}
	else
	{
		// disable specfic feature available when binary is running
		useCDAct->setDisabled(true);
		palAct->setDisabled(true);
		ntscAct->setDisabled(true);
		pauseAct->setChecked(false);
		pauseAct->setDisabled(false);

		showUntunedTankCircuit = false;

		// display the use of the CD
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
		DebuggerReset();
		CommonReset();
		DebuggerResetWindows();
		CommonResetWindows();
		DACPauseAudioThread(false);
	}
}


// Toggle the emulator state, it can be either on or off
void MainWin::ToggleRunState(void)
{
	startM68KTracing = running;

	// switch the running mode
	running = !running;
	if (!running)
	{
		// set action buttons for the pause mode
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

		// restore the mouse pointer, if hidden:
		videoWidget->CheckAndRestoreMouseCursor();

		// video screen turned blue as to show a pause mode
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

		RefreshWindows();
	}
	else
	{
		// set action buttons for the run mode
		frameAdvanceAct->setDisabled(true);
		pauseAct->setChecked(false);
		pauseAct->setDisabled(false);
		if (vjs.softTypeDebugger)
		{
			traceStepIntoAct->setDisabled(true);
			traceStepOverAct->setDisabled(true);
			restartAct->setDisabled(true);
			BreakpointsWin->RefreshContents();
		}

		cpuBrowseWin->UnholdBPM();
	}

	emuStatusWin->ResetM68KCycles();
	ShowstdConsoleWin();
	// Pause/unpause any running/non-running threads...
	DACPauseAudioThread(!running);
}


// Resize the video screen with a zoom level 1
void MainWin::SetZoom100(void)
{
	zoomLevel = 1;
	ResizeMainWindow();
}


// Resize the video screen with a zoom level 2
void MainWin::SetZoom200(void)
{
	zoomLevel = 2;
	ResizeMainWindow();
}


// Resize the video screen with a zoom level 3
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

#if defined(SAVESTATEPATCH_PvtLewis)
extern volatile bool dac_load_state;
extern volatile bool dac_dump_state;

#ifdef Q_OS_WIN
#include <windows.h> // for Sleep
#endif
void MainWin::msSleep(int ms)
{
#ifdef Q_OS_WIN
    Sleep(uint(ms));
#else
    struct timespec ts = { ms / 1000, (ms % 1000) * 1000 * 1000 };
    nanosleep(&ts, NULL);
#endif
}

void MainWin::DumpCommand(void)
{
  if (!vjs.DSPEnabled) {
    // Calling ToggleRunState is not necessary, just using it for the screen flash
    ToggleRunState();
    DumpSaveState();
    ToggleRunState();
  } else {
    dac_dump_state = true;
    int ctr = 0;
    while (dac_dump_state) {
      msSleep(1);
      ctr++;
      if (ctr > 1000) {
        break;
      }
    }
    // Calling ToggleRunState is not necessary, just using it for the screen flash
    ToggleRunState();
    DumpSaveState();
    ToggleRunState();
  }
}

void MainWin::LoadCommandTimer(void)
{
  LoadSaveState();
}

void MainWin::LoadCommand(void)
{
  if (CanTryToLoadSaveState() == -1) {
    return;
  }

  if (!vjs.DSPEnabled) {
    LoadSaveState();
  } else {
    dac_load_state = true;
    int ctr = 0;
    while (dac_load_state) {
      msSleep(1);
      ctr++;
      if (ctr > 1000) {
        break;
      }
    }
    LoadSaveState();
  }
}

extern int save_slot;
void MainWin::SaveSlot0Command(void)
{
  saveSlot0Act->setChecked(true);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 0;
}
void MainWin::SaveSlot1Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(true);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 1;
}
void MainWin::SaveSlot2Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(true);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 2;
}
void MainWin::SaveSlot3Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(true);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 3;
}
void MainWin::SaveSlot4Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(true);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 4;
}
void MainWin::SaveSlot5Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(true);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 5;
}
void MainWin::SaveSlot6Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(true);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 6;
}
void MainWin::SaveSlot7Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(true);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(false);
  save_slot = 7;
}
void MainWin::SaveSlot8Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(true);
  saveSlot9Act->setChecked(false);
  save_slot = 8;
}
void MainWin::SaveSlot9Command(void)
{
  saveSlot0Act->setChecked(false);
  saveSlot1Act->setChecked(false);
  saveSlot2Act->setChecked(false);
  saveSlot3Act->setChecked(false);
  saveSlot4Act->setChecked(false);
  saveSlot5Act->setChecked(false);
  saveSlot6Act->setChecked(false);
  saveSlot7Act->setChecked(false);
  saveSlot8Act->setChecked(false);
  saveSlot9Act->setChecked(true);
  save_slot = 9;
}
#endif

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

	// Setup BIOS in his own dedicated Jaguar memory
#ifndef NEWMODELSBIOSHANDLER
	uint8_t * biosPointer = jaguarBootROM;

	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		biosPointer = jaguarDevBootROM2;
	}

	memcpy(jagMemSpace + 0xE00000, biosPointer, 0x20000);
#else
	SelectBIOS(vjs.biosType);
#endif

	// Turn 'on' the power to initialize the Jaguar
	powerAct->setDisabled(false);
	powerAct->setChecked(true);
	powerButtonOn = false;
	TogglePowerState();

	// We have to load our software *after* the Jaguar RESET
	cartridgeLoaded = JaguarLoadFile(file.toUtf8().data());
	SET32(jaguarMainRAM, 0, vjs.DRAM_size);						// Set stack in the M68000's Reset SP

	// Get the Console standard emulation variable address
	stdConsoleExist = stdConsole_set(STDCONSOLE_STDIN, DBGManager_GetAdrFromSymbolName((char *)"cngetc_value"));
	stdConsoleExist |= stdConsole_set(STDCONSOLE_STDOUT, DBGManager_GetAdrFromSymbolName((char *)"cnputc_value")) ? true : false;

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
		SourcesWin->Init();
		//pauseAct->setDisabled(false);
		//pauseAct->setChecked(true);
		ToggleRunState();
		//RefreshWindows();
	}
	else
	{
		// Prevent the launch in case of software without a start address and without BIOS presence
		if (!vjs.useJaguarBIOS && !jaguarRunAddress)
		{
			ToggleRunState();
		}
	}

	// Display the Atari Jaguar software which is running
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


// Open, or display, the breakpoints list window
void MainWin::ShowBreakpointsWin(void)
{
	BreakpointsWin->show();
	BreakpointsWin->RefreshContents();
}


// Delete all breakpoints
void MainWin::DeleteAllBreakpoints(void)
{
	cpuBrowseWin->ResetBPM();
	m68k_brk_reset();
	ShowBreakpointsWin();
}


// Disable all breakpoints
void MainWin::DisableAllBreakpoints(void)
{
	cpuBrowseWin->DisableBPM();
	m68k_brk_disable();
	ShowBreakpointsWin();
}


// Open, or display, the new breakpoint function window
void MainWin::ShowNewFunctionBreakpointWin(void)
{
	NewFunctionBreakpointWin->SetFnctBreakpointWin(BreakpointsWin);
	NewFunctionBreakpointWin->show();
	ShowBreakpointsWin();
}


// Display list of files found in cartridge
void MainWin::ShowCartFilesListWin(void)
{
	CartFilesListWin->show();
	CartFilesListWin->RefreshContents();
}


// Display the save dump pickup file
void MainWin::ShowSaveDumpAsWin(void)
{
	SaveDumpAsWin->show();
}


// Step Into trace
void MainWin::DebuggerTraceStepInto(void)
{
	if (SourcesWin->isVisible() && SourcesWin->GetTraceStatus())
	{
		while (!SourcesWin->CheckChangeLine())
		{
			emuStatusWin->UpdateM68KCycles(JaguarStepInto());
		}
	}
	else
	{
		emuStatusWin->UpdateM68KCycles(JaguarStepInto());
	}

	videoWidget->updateGL();
	RefreshWindows();
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to verify the Step Into function !!!")
#else
	#warning "!!! Need to verify the Step Into function !!!"
#endif // _MSC_VER
}


// Restart the Jaguar executable
void MainWin::DebuggerRestart(void)
{
#if 1
	m68k_pulse_reset();
#else
	m68k_set_reg(M68K_REG_PC, jaguarRunAddress);
	m68k_set_reg(M68K_REG_SP, vjs.DRAM_size);
#endif
	dasmtabWidget->setCurrentIndex(1);		// set focus on the disasm M68K tab
	m68k_set_reg(M68K_REG_A6, 0);
	m68k_brk_hitcounts_reset();
	emuStatusWin->ResetM68KCycles();
	bpmHitCounts = 0;
	DebuggerResetWindows();
	CommonResetWindows();
	SourcesWin->Init();
	RefreshWindows();
#ifdef _MSC_VER
#pragma message("Warning: !!! Need to verify the Restart function !!!")
#else
	#warning "!!! Need to verify the Restart function !!!"
#endif // _MSC_VER
}


// Step Over trace
void MainWin::DebuggerTraceStepOver(void)
{
	if (SourcesWin->isVisible() && SourcesWin->GetTraceStatus())
	{
		while (!SourcesWin->CheckChangeLine())
		{
			emuStatusWin->UpdateM68KCycles(JaguarStepOver(0));
		}
	}
	else
	{
		emuStatusWin->UpdateM68KCycles(JaguarStepOver(0));
	}

	videoWidget->updateGL();
	RefreshWindows();
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
		//vjs.softTypeDebugger ? VideoOutputWin->RefreshContents(videoWidget) : NULL;
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


// Show the M68K exception vector table browser window
// This debug only window comes from user request
void MainWin::ShowExceptionVectorTableBrowserWin(void)
{
	exceptionvectortableBrowseWin->show();
	exceptionvectortableBrowseWin->RefreshContents();
}


// Show the (M68k DRAM) local variable browser window
// This debug only window comes from user request
void MainWin::ShowLocalBrowserWin(void)
{
	LocalBrowseWin->show();
	LocalBrowseWin->RefreshContents();
}


// Show the (M68k DRAM) call stack browser window
// This debug only window comes from user request
void MainWin::ShowCallStackBrowserWin(void)
{
	CallStackBrowseWin->show();
	CallStackBrowseWin->RefreshContents();
}


// Show the all watches browser window
// This debug only window comes from user request
void MainWin::ShowAllWatchBrowserWin(void)
{
	allWatchBrowseWin->show();
	allWatchBrowseWin->RefreshContents();
}


// Show the (M68k DRAM) heap allocation browser window
// This debug only window comes from user request
void MainWin::ShowHeapAllocatorBrowserWin(void)
{
	heapallocatorBrowseWin->show();
	heapallocatorBrowseWin->RefreshContents();
}


// Show the ROM cart browser window
// This debug only window comes from user request
void MainWin::ShowROMCartBrowserWin(void)
{
	romcartBrowseWin->show();
	romcartBrowseWin->RefreshContents();
}


// Show the memory (M68K DRAM, GPU & DSP) browser window
// This debug only window comes from user request
void MainWin::ShowMemoryBrowserWin(int NumWin)
{
	memBrowseWin[NumWin]->show();
	memBrowseWin[NumWin]->RefreshContents();
}


// Show one of the memory (M68K DRAM) browser window
// This debug only window comes from user request
void MainWin::ShowMemory1BrowserWin(int NumWin)
{
//	for (int i = 0; i < vjs.nbrmemory1browserwindow; i++)
	{
		mem1BrowseWin[NumWin]->show();
		mem1BrowseWin[NumWin]->RefreshContents(NumWin);
	}
}


// Show the Console standard emulation window
// This window is automatically displayed in case of the Console standard emulation has been detected in the build
void MainWin::ShowstdConsoleWin(void)
{
	if (stdConsoleExist)
	{
		stdConsoleWin->show();
		stdConsoleWin->RefreshContents();
	}
	else
	{
		stdConsoleWin->hide();
	}
}


// Show the emulation status window
// This window is displayed by user request
void MainWin::ShowEmuStatusWin(void)
{
	emuStatusWin->show();
	emuStatusWin->RefreshContents();
}


// Show the M68K stack browser window
// This debug window is displayed by user request
void MainWin::ShowStackBrowserWin(void)
{
	stackBrowseWin->show();
	stackBrowseWin->RefreshContents();
}


// Show the CPUs (M68K, GPU and DSP) browser window
// This debug window is displayed by user request
void MainWin::ShowCPUBrowserWin(void)
{
	cpuBrowseWin->show();
	cpuBrowseWin->RefreshContents();
}


// Show the OP (Object Processor) browser window
// This debug window is displayed by user request
void MainWin::ShowOPBrowserWin(void)
{
	opBrowseWin->show();
	opBrowseWin->RefreshContents();
}


// Show the HW registers browser window
// This debug window is displayed by user request
void MainWin::ShowHWRegsBrowserWin(void)
{
	hwRegsBrowseWin->show();
	hwRegsBrowseWin->RefreshContents();
}


// Show the M68K code disassembly browser window
// This debug window is displayed by user request
void MainWin::ShowM68KDasmBrowserWin(void)
{
	m68kDasmBrowseWin->show();
	m68kDasmBrowseWin->RefreshContents();
}


// Show the RISC (GPU/DSP) code disassembly browser window
// This debug window is displayed by user request
void MainWin::ShowRISCDasmBrowserWin(void)
{
	riscDasmBrowseWin->show();
	riscDasmBrowseWin->RefreshContents();
}


//
#if 0
void	MainWin::ShowDasmWin(void)
{
//	DasmWin->show();
//	DasmWin->RefreshContents();
}
#endif


// 
void MainWin::ShowVideoOutputWin(void)
{
	//VideoOutputWindowCentrale = mainWindowCentrale->addSubWindow(videoWidget);
	//VideoOutputWindowCentrale->setWindowTitle(QString(tr("Video output")));
	//VideoOutputWindowCentrale->show();
	//memBrowseWin->show();
	VideoOutputWin->show();
	VideoOutputWin->SetupVideo(videoWidget);
	//VideoOutputWin->adjustSize();
	//VideoOutputWin->RefreshContents(videoWidget);
}


// Resize video window based on zoom factor
// It doesn't apply in debugger mode as we use this window to display disassembly
void MainWin::ResizeMainWindow(void)
{
	if (!vjs.softTypeDebugger)
	{
		videoWidget->setFixedSize(zoomLevel * VIRTUAL_SCREEN_WIDTH,	zoomLevel * (vjs.hardwareTypeNTSC ? VIRTUAL_SCREEN_HEIGHT_NTSC : VIRTUAL_SCREEN_HEIGHT_PAL));

		// Show the test pattern if user requested plzDontKillMyComputer mode
		if (!powerButtonOn && plzDontKillMyComputer)
		{
			for (uint32_t y = 0; y < videoWidget->rasterHeight; y++)
			{
				if (vjs.hardwareTypeNTSC)
				{
					memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
				}
				else
				{
					memcpy(videoWidget->buffer + (y * videoWidget->textureWidth), testPattern2 + (y * VIRTUAL_SCREEN_WIDTH), VIRTUAL_SCREEN_WIDTH * sizeof(uint32_t));
				}
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
	lastEditedProfile = settings.value("lastEditedProfile", 0).toInt();

	vjs.useJoystick = settings.value("useJoystick", false).toBool();
	vjs.joyport = settings.value("joyport", 0).toInt();
	vjs.hardwareTypeNTSC = settings.value("hardwareTypeNTSC", true).toBool();
	vjs.frameSkip = settings.value("frameSkip", 0).toInt();
	vjs.audioEnabled = settings.value("audioEnabled", true).toBool();
	vjs.usePipelinedDSP = settings.value("usePipelinedDSP", false).toBool();
	vjs.useOpenGL = settings.value("useOpenGL", true).toBool();
	vjs.glFilter = settings.value("glFilterType", 1).toInt();
	vjs.renderType = settings.value("renderType", 0).toInt();

	// read the BIOS & console model settings
	vjs.biosType = settings.value("biosType", BT_M_SERIES).toInt();
	vjs.jaguarModel = settings.value("jaguarModel", JAG_M_SERIES).toInt();
	vjs.useJaguarBIOS = settings.value("useJaguarBIOS", false).toBool();
	vjs.useRetailBIOS = settings.value("useRetailBIOS", false).toBool();
	vjs.useDevBIOS = settings.value("useDevBIOS", false).toBool();

	// read the general settings
	vjs.compressSaveStates = settings.value("compressSaveStates", true).toBool();
	strcpy(vjs.SaveStatePath, settings.value("SaveStates", QStandardPaths::writableLocation(QStandardPaths::DataLocation).append("/savestates/")).toString().toUtf8().data());
	strcpy(vjs.EEPROMPath, settings.value("EEPROMs", QStandardPaths::writableLocation(QStandardPaths::DataLocation).append("/eeproms/")).toString().toUtf8().data());
	strcpy(vjs.ROMPath, settings.value("ROMs", QStandardPaths::writableLocation(QStandardPaths::DataLocation).append("/software/")).toString().toUtf8().data());
	strcpy(vjs.screenshotPath, settings.value("Screenshots", QStandardPaths::writableLocation(QStandardPaths::DataLocation).append("/screenshots/")).toString().toUtf8().data());
	vjs.fullscreen = settings.value("fullscreen", false).toBool();
	vjs.GPUEnabled = settings.value("GPUEnabled", true).toBool();
	vjs.DSPEnabled = settings.value("DSPEnabled", true).toBool();
	allowUnknownSoftware = settings.value("showUnknownSoftware", false).toBool();
	vjs.useDisplayEmuFPS = settings.value("useDisplayEmuFPS", true).toBool();

	// read the exceptions settings
	vjs.allowWritesToROM = settings.value("writeROM", true).toBool();
	vjs.allowM68KExceptionCatch = settings.value("M68KExceptionCatch", false).toBool();
	vjs.allowWritesToUnknownLocation = settings.value("WriteUnknownLocation", true).toBool();
	vjs.useFastBlitter = settings.value("useFastBlitter", false).toBool();

	// read settings from the Debugger mode
	settings.beginGroup("debugger");
	strcpy(vjs.debuggerROMPath, settings.value("DefaultROM", "").toString().toUtf8().data());
	strcpy(vjs.sourcefilesearchPaths, settings.value("SourceFileSearchPaths", "").toString().toUtf8().data());
	vjs.nbrdisasmlines = settings.value("NbrDisasmLines", 32).toUInt();
	vjs.disasmopcodes = settings.value("DisasmOpcodes", true).toBool();
	vjs.displayHWlabels = settings.value("DisplayHWLabels", true).toBool();
	vjs.displayFullSourceFilename = settings.value("displayFullSourceFilename", true).toBool();
	vjs.ELFSectionsCheck = settings.value("ELFSectionsCheck", false).toBool();
	vjs.nbrmemory1browserwindow = settings.value("NbrMemory1BrowserWindow", MaxMemory1BrowserWindow).toUInt();
	vjs.cygdriveDirRemoval = settings.value("cygdriveDirRemoval", false).toBool();
	settings.endGroup();

	// read settings from the Alpine mode
	settings.beginGroup("alpine");
	strcpy(vjs.alpineROMPath, settings.value("DefaultROM", "").toString().toUtf8().data());
	strcpy(vjs.absROMPath, settings.value("DefaultABS", "").toString().toUtf8().data());
	vjs.refresh = settings.value("refresh", 60).toUInt();
	settings.endGroup();

	// read settings from the Keybindings
	settings.beginGroup("keybindings");
	for (i = 0; i < KB_END; i++)
	{
		strcpy(vjs.KBContent[i].KBSettingValue, settings.value(KeyBindingsTable[i].KBNameSetting, KeyBindingsTable[i].KBDefaultValue).toString().toUtf8().data());
	}
	settings.endGroup();

#if 0
	// Write important settings to the log file
	WriteLog("MainWin: Paths\n");
	WriteLog("SaveStatePath = \"%s\"\n", vjs.SaveStatePath);
	WriteLog("           EEPROMPath = \"%s\"\n", vjs.EEPROMPath);
	WriteLog("              ROMPath = \"%s\"\n", vjs.ROMPath);
	WriteLog("        AlpineROMPath = \"%s\"\n", vjs.alpineROMPath);
	WriteLog("      DebuggerROMPath = \"%s\"\n", vjs.debuggerROMPath);
	WriteLog("           absROMPath = \"%s\"\n", vjs.absROMPath);
	WriteLog("      ScreenshotsPath = \"%s\"\n", vjs.screenshotPath);
	WriteLog("SourceFileSearchPaths = \"%s\"\n", vjs.sourcefilesearchPaths);
	WriteLog("MainWin: Misc.\n");
	WriteLog("   Pipelined DSP = %s\n", (vjs.usePipelinedDSP ? "ON" : "off"));
#endif

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
	DBGManager_SourceFileSearchPathsSet(vjs.sourcefilesearchPaths);
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
	char Name[100];
	int i;
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

	// Emulator status UI information
	pos = settings.value("emuStatusWinPos", QPoint(200, 200)).toPoint();
	emuStatusWin->move(pos);
	settings.value("emuStatusWinIsVisible", false).toBool() ? ShowEmuStatusWin() : void();
	
	// Console standard emulation information
	pos = settings.value("stdConsoleWinPos", QPoint(200, 200)).toPoint();
	stdConsoleWin->move(pos);
	size = settings.value("stdConsoleWinSize", QSize(400, 400)).toSize();
	stdConsoleWin->resize(size);
	stdConsoleWin->StyleSheetColor->setCheckState(Qt::CheckState(settings.value("stdConsoleWinStyleSheetColorCheck", 0).toInt()));

	// Alpine debug UI information (also needed by the Debugger)
	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		// ROM UI information
		pos = settings.value("romcartBrowseWinPos", QPoint(200, 200)).toPoint();
		romcartBrowseWin->move(pos);
		settings.value("romcartBrowseWinIsVisible", false).toBool() ? ShowROMCartBrowserWin() : void();

		// CPU registers UI information
		pos = settings.value("cpuBrowseWinPos", QPoint(200, 200)).toPoint();
		cpuBrowseWin->move(pos);
		settings.value("cpuBrowseWinIsVisible", false).toBool() ? ShowCPUBrowserWin() : void();

		// Memory browser UI information
		for (i = 0; i < 3; i++)
		{
			sprintf(Name, "memBrowseWinPos[%i]", i);
			pos = settings.value(Name, QPoint(200, 200)).toPoint();
			memBrowseWin[i]->move(pos);
			sprintf(Name, "memBrowseWinIsVisible[%i]", i);
			settings.value(Name, false).toBool() ? ShowMemoryBrowserWin(i) : void();
		}

		// Stack browser UI information
		pos = settings.value("stackBrowseWinPos", QPoint(200, 200)).toPoint();
		stackBrowseWin->move(pos);
		settings.value("stackBrowseWinIsVisible", false).toBool() ? ShowStackBrowserWin() : void();
		size = settings.value("stackBrowseWinSize", QSize(400, 400)).toSize();
		stackBrowseWin->resize(size);

		// OP (Object Processor) UI information
		pos = settings.value("opBrowseWinPos", QPoint(200, 200)).toPoint();
		opBrowseWin->move(pos);
		settings.value("opBrowseWinIsVisible", false).toBool() ? ShowOPBrowserWin() : void();
		size = settings.value("opBrowseWinSize", QSize(400, 400)).toSize();
		opBrowseWin->resize(size);

		// HW registers UI information
		pos = settings.value("hwRegsBrowseWinPos", QPoint(200, 200)).toPoint();
		hwRegsBrowseWin->move(pos);
		settings.value("hwRegsBrowseWinIsVisible", false).toBool() ? ShowHWRegsBrowserWin() : void();
		size = settings.value("hwRegsBrowseWinSize", QSize(400, 400)).toSize();
		hwRegsBrowseWin->resize(size);

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

		// cartridge directory and files UI information
		pos = settings.value("CartFilesListWinPos", QPoint(200, 200)).toPoint();
		CartFilesListWin->move(pos);
		settings.value("CartFilesListWinIsVisible", false).toBool() ? ShowCartFilesListWin() : void();
		size = settings.value("CartFilesListWinSize", QSize(400, 400)).toSize();
		CartFilesListWin->resize(size);

		// Save dump UI information
		pos = settings.value("SaveDumpAsWinPos", QPoint(200, 200)).toPoint();
		SaveDumpAsWin->move(pos);
		settings.value("SaveDumpAsWinIsVisible", false).toBool() ? ShowSaveDumpAsWin() : void();
		size = settings.value("SaveDumpAsWinSize", QSize(400, 400)).toSize();
		SaveDumpAsWin->resize(size);

		// save output video UI information
		pos = settings.value("VideoOutputWinPos", QPoint(200, 200)).toPoint();
		VideoOutputWin->move(pos);
		settings.value("VideoOutputWinIsVisible", false).toBool() ? ShowVideoOutputWin() : void();
		size = settings.value("VideoOutputWinSize", QSize(400, 400)).toSize();
		VideoOutputWin->resize(size);

		// Breakpoints UI information
		pos = settings.value("BreakpointsWinPos", QPoint(200, 200)).toPoint();
		BreakpointsWin->move(pos);
		settings.value("BreakpointsWinIsVisible", false).toBool() ? ShowBreakpointsWin() : void();
		size = settings.value("BreakpointsWinSize", QSize(400, 400)).toSize();
		BreakpointsWin->resize(size);
		// New function breakpoint UI information
		pos = settings.value("NewFunctionBreakpointWinPos", QPoint(200, 200)).toPoint();
		NewFunctionBreakpointWin->move(pos);
		settings.value("NewFunctionBreakpointWinIsVisible", false).toBool() ? ShowNewFunctionBreakpointWin() : void();
		size = settings.value("NewFunctionBreakpointWinSize", QSize(400, 400)).toSize();
		NewFunctionBreakpointWin->resize(size);

		// Memories browser UI information
		for (i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			sprintf(Name, "mem1BrowseWinPos[%i]", (unsigned int)i);
			pos = settings.value(Name, QPoint(200, 200)).toPoint();
			mem1BrowseWin[i]->move(pos);
			sprintf(Name, "mem1BrowseWinIsVisible[%i]", (unsigned int)i);
			settings.value(Name, false).toBool() ? ShowMemory1BrowserWin((int)i) : void();
			sprintf(Name, "mem1BrowseWinSize[%i]", (unsigned int)i);
			size = settings.value(Name, QSize(400, 400)).toSize();
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
	settings.setValue("lastEditedProfile", lastEditedProfile);

	settings.setValue("useJoystick", vjs.useJoystick);
	settings.setValue("joyport", vjs.joyport);
	settings.setValue("hardwareTypeNTSC", vjs.hardwareTypeNTSC);
	settings.setValue("frameSkip", vjs.frameSkip);
	settings.setValue("audioEnabled", vjs.audioEnabled);
	settings.setValue("usePipelinedDSP", vjs.usePipelinedDSP);
	settings.setValue("useOpenGL", vjs.useOpenGL);
	settings.setValue("glFilterType", vjs.glFilter);
	settings.setValue("renderType", vjs.renderType);
	//settings.setValue("JagBootROM", vjs.jagBootPath);
	//settings.setValue("CDBootROM", vjs.CDBootPath);

	// write the BIOS & console model settings
	settings.setValue("jaguarModel", vjs.jaguarModel);
	settings.setValue("biosType", vjs.biosType);
	settings.setValue("useJaguarBIOS", vjs.useJaguarBIOS);
	settings.setValue("useRetailBIOS", vjs.useRetailBIOS);
	settings.setValue("useDevBIOS", vjs.useDevBIOS);

	// write the general settings
	settings.setValue("SaveStates", vjs.SaveStatePath);
	settings.setValue("compressSaveStates", vjs.compressSaveStates);
	settings.setValue("EEPROMs", vjs.EEPROMPath);
	settings.setValue("ROMs", vjs.ROMPath);
	settings.setValue("Screenshots", vjs.screenshotPath);
	settings.setValue("GPUEnabled", vjs.GPUEnabled);
	settings.setValue("DSPEnabled", vjs.DSPEnabled);
	settings.setValue("fullscreen", vjs.fullscreen);
	settings.setValue("showUnknownSoftware", allowUnknownSoftware);
	settings.setValue("useFastBlitter", vjs.useFastBlitter);
	settings.setValue("useDisplayEmuFPS", vjs.useDisplayEmuFPS);

	// write the exceptions settings 
	settings.setValue("writeROM", vjs.allowWritesToROM);
	settings.setValue("M68KExceptionCatch", vjs.allowM68KExceptionCatch);
	settings.setValue("WriteUnknownLocation", vjs.allowWritesToUnknownLocation);

	// write settings from the Alpine mode
	settings.beginGroup("alpine");
	settings.setValue("refresh", vjs.refresh);
	settings.setValue("DefaultROM", vjs.alpineROMPath);
	settings.setValue("DefaultABS", vjs.absROMPath);
	settings.endGroup();

	// write settings from the Debugger mode
	settings.beginGroup("debugger");
	settings.setValue("DisplayHWLabels", vjs.displayHWlabels);
	settings.setValue("NbrDisasmLines", (qulonglong) vjs.nbrdisasmlines);
	settings.setValue("DisasmOpcodes", vjs.disasmopcodes);
	settings.setValue("displayFullSourceFilename", vjs.displayFullSourceFilename);
	settings.setValue("ELFSectionsCheck", vjs.ELFSectionsCheck);
	settings.setValue("NbrMemory1BrowserWindow", (unsigned int)vjs.nbrmemory1browserwindow);
	settings.setValue("DefaultROM", vjs.debuggerROMPath);
	settings.setValue("SourceFileSearchPaths", vjs.sourcefilesearchPaths);
	settings.setValue("cygdriveDirRemoval", vjs.cygdriveDirRemoval);
	settings.endGroup();

	// write settings from the Keybindings
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
	DBGManager_SourceFileSearchPathsSet(vjs.sourcefilesearchPaths);
}


// Save the UI settings
void MainWin::WriteUISettings(void)
{
	char Name[100];
	int i;

	// Point on the emulator settings
	QSettings settings("Underground Software", "Virtual Jaguar");
	settings.beginGroup("ui");
	
	// Emulator UI information
	settings.setValue("pos", pos());
	settings.setValue("size", size());
	settings.setValue("cartLoadPos", filePickWin->pos());

	// Video output information
	settings.setValue("zoom", zoomLevel);

	// Common UI information
	settings.setValue("emuStatusWinPos", emuStatusWin->pos());
	settings.setValue("emuStatusWinIsVisible", emuStatusWin->isVisible());
	settings.setValue("stdConsoleWinPos", stdConsoleWin->pos());
	settings.setValue("stdConsoleWinSize", stdConsoleWin->size());
	settings.setValue("stdConsoleWinStyleSheetColorCheck", stdConsoleWin->StyleSheetColor->checkState());
	
	// Alpine debug UI information (also needed by the Debugger)
	if (vjs.hardwareTypeAlpine || vjs.softTypeDebugger)
	{
		// ROM cartridge browser window
		settings.setValue("romcartBrowseWinPos", romcartBrowseWin->pos());
		settings.setValue("romcartBrowseWinIsVisible", romcartBrowseWin->isVisible());
		// CPU & RISC registers display window
		settings.setValue("cpuBrowseWinPos", cpuBrowseWin->pos());
		settings.setValue("cpuBrowseWinIsVisible", cpuBrowseWin->isVisible());
		// Memory browser window
		for (i = 0; i < 3; i++)
		{
			sprintf(Name, "memBrowseWinPos[%i]", i);
			settings.setValue(Name, memBrowseWin[i]->pos());
			sprintf(Name, "memBrowseWinIsVisible[%i]", i);
			settings.setValue(Name, memBrowseWin[i]->isVisible());
		}
		// Stack browser window
		settings.setValue("stackBrowseWinPos", stackBrowseWin->pos());
		settings.setValue("stackBrowseWinIsVisible", stackBrowseWin->isVisible());
		settings.setValue("stackBrowseWinSize", stackBrowseWin->size());
		// OP (Object Processor) browser window
		settings.setValue("opBrowseWinPos", opBrowseWin->pos());
		settings.setValue("opBrowseWinIsVisible", opBrowseWin->isVisible());
		settings.setValue("opBrowseWinSize", opBrowseWin->size());
		// HW registers browser window
		settings.setValue("hwRegsBrowseWinPos", hwRegsBrowseWin->pos());
		settings.setValue("hwRegsBrowseWinIsVisible", hwRegsBrowseWin->isVisible());
		settings.setValue("hwRegsBrowseWinSize", hwRegsBrowseWin->size());
		// RISC disassembly browser window
		settings.setValue("riscDasmBrowseWinPos", riscDasmBrowseWin->pos());
		settings.setValue("riscDasmBrowseWinIsVisible", riscDasmBrowseWin->isVisible());
		// M68K disassembly browser window
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
		settings.setValue("BreakpointsWinPos", BreakpointsWin->pos());
		settings.setValue("BreakpointsWinIsVisible", BreakpointsWin->isVisible());
		settings.setValue("BreakpointsWinSize", BreakpointsWin->size());
		settings.setValue("NewFunctionBreakpointWinPos", NewFunctionBreakpointWin->pos());
		settings.setValue("NewFunctionBreakpointWinIsVisible", NewFunctionBreakpointWin->isVisible());
		settings.setValue("NewFunctionBreakpointWinSize", NewFunctionBreakpointWin->size());
		settings.setValue("CartFilesListWinPos", CartFilesListWin->pos());
		settings.setValue("CartFilesListWinIsVisible", CartFilesListWin->isVisible());
		settings.setValue("CartFilesListWinSize", CartFilesListWin->size());
		settings.setValue("SaveDumpAsWinPos", SaveDumpAsWin->pos());
		settings.setValue("SaveDumpAsWinIsVisible", SaveDumpAsWin->isVisible());
		settings.setValue("SaveDumpAsWinSize", SaveDumpAsWin->size());
		settings.setValue("VideoOutputWinPos", VideoOutputWin->pos());
		settings.setValue("VideoOutputWinIsVisible", VideoOutputWin->isVisible());
		settings.setValue("VideoOutputWinSize", VideoOutputWin->size());

		for (i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			sprintf(Name, "mem1BrowseWinPos[%i]", i);
			settings.setValue(Name, mem1BrowseWin[i]->pos());
			sprintf(Name, "mem1BrowseWinIsVisible[%i]", i);
			settings.setValue(Name, mem1BrowseWin[i]->isVisible());
			sprintf(Name, "mem1BrowseWinSize[%i]", i);
			settings.setValue(Name, mem1BrowseWin[i]->size());
		}
	}

	settings.endGroup();
}


// Refresh Alpine debug windows
void MainWin::AlpineRefreshWindows(void)
{
	cpuBrowseWin->RefreshContents();
	for (size_t i = 0; i < 3; i++)
	{
		memBrowseWin[i]->RefreshContents();
	}
	stackBrowseWin->RefreshContents();
	opBrowseWin->RefreshContents();
	riscDasmBrowseWin->RefreshContents();
	m68kDasmBrowseWin->RefreshContents();
	hwRegsBrowseWin->RefreshContents();
}


// 
void MainWin::CommonResetWindows(void)
{
	//stdConsoleExist = false;
	//stdConsoleWin->hide();
	stdConsoleWin->Reset();
}


// Reset common
void MainWin::CommonReset(void)
{
	//stdConsoleWin->Reset();
}


// Reset soft debugger
void MainWin::DebuggerReset(void)
{
	if (vjs.softTypeDebugger)
	{
		DeleteAllBreakpoints();
	}
}


// Reset soft debugger windows
void MainWin::DebuggerResetWindows(void)
{
	if (vjs.softTypeDebugger)
	{
		FilesrcListWin->Reset();
		allWatchBrowseWin->Reset();
		heapallocatorBrowseWin->Reset();
		BreakpointsWin->Reset();
		CartFilesListWin->Reset();
		SourcesWin->Reset();
		//ResetAlpineWindows();
	}
}


// Refresh windows common to all emulation modes (debug or not)
// Emulation status, and Console standard emulation
void MainWin::CommonRefreshWindows(void)
{
	emuStatusWin->RefreshContents();
	stdConsoleWin->RefreshContents();
	//IOConsoleWin->RefreshContents();
}


// Refresh view windows
void MainWin::ViewRefreshWindows(void)
{
	CartFilesListWin->RefreshContents();
}


// 
void MainWin::RefreshWindows(void)
{
	DebuggerRefreshWindows();
	CommonRefreshWindows();
}


// Refresh soft debugger & alpine debug windows
void MainWin::DebuggerRefreshWindows(void)
{
	if (vjs.softTypeDebugger)
	{
		//VideoOutputWin->RefreshContents(videoWidget);
		FilesrcListWin->RefreshContents();
		SourcesWin->RefreshContents();
		m68kDasmWin->RefreshContents();
		GPUDasmWin->RefreshContents();
		DSPDasmWin->RefreshContents();
		allWatchBrowseWin->RefreshContents();
		LocalBrowseWin->RefreshContents();
		CallStackBrowseWin->RefreshContents();
		heapallocatorBrowseWin->RefreshContents();
		BreakpointsWin->RefreshContents();
		for (size_t i = 0; i < vjs.nbrmemory1browserwindow; i++)
		{
			mem1BrowseWin[i]->RefreshContents(i);
		}

		AlpineRefreshWindows();
		ViewRefreshWindows();
	}
}


// Create and save screenshot
void MainWin::MakeScreenshot(void)
{
	char Text[256];
	QImage screenshot;
	time_t now = time(0);
	struct tm tstruct;

	// Create filename
	tstruct = *localtime(&now);
	sprintf(Text, "%svj_%i%i%i_%i%i%i.jpg", vjs.screenshotPath, tstruct.tm_year, tstruct.tm_mon, tstruct.tm_mday, tstruct.tm_hour, tstruct.tm_min, tstruct.tm_sec);

	// Create screenshot
	screenshot = videoWidget->grabFrameBuffer();
	screenshot.save((char *)Text, "JPG", 100);
}

