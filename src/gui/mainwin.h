//
// mainwin.h: Header file
//
// by James Hammons
// (C) 2010 Underground Software
//
// Modified by Jean-Paul Mari
//

#ifndef __MAINWIN_H__
#define __MAINWIN_H__

//Hrm. uh??? I thought this wasn't the way to do this stuff...???
#include <QtWidgets>
#include "tom.h"

#define RING_BUFFER_SIZE 32

// Main windows
class GLWidget;
//class VideoWindow;
class AboutWindow;
class HelpWindow;
class FilePickerWindow;
//class VideoOutputWindow;
//class DasmWindow;
class m68KDasmWindow;
class GPUDasmWindow;
class DSPDasmWindow;
class EmuStatusWindow;

// Alpine
class MemoryBrowserWindow;
class StackBrowserWindow;
class CPUBrowserWindow;
class OPBrowserWindow;
class M68KDasmBrowserWindow;
class RISCDasmBrowserWindow;

// Debugger
class AllWatchBrowserWindow;
class LocalBrowserWindow;
class CallStackBrowserWindow;
class HeapAllocatorBrowserWindow;
class Memory1BrowserWindow;
//class BrkWindow;
class ExceptionVectorTableBrowserWindow;
class FilesrcListWindow;


// 
class MainWin: public QMainWindow
{
	// All Qt apps require this macro for signal/slot functionality to work
	Q_OBJECT

	public:
//		MainWin(QString);
		MainWin(bool);
		void LoadFile(QString);
		void SyncUI(void);
		void DebuggerRefreshWindows(void);
		void DebuggerResetWindows(void);
		void AlpineRefreshWindows(void);

	protected:
		void closeEvent(QCloseEvent *);
		void keyPressEvent(QKeyEvent *);
		void keyReleaseEvent(QKeyEvent *);

	private slots:
		void Open(void);
		void Configure(void);
		void Timer(void);
		void TogglePowerState(void);
		void ToggleRunState(void);
		void SetZoom100(void);
		void SetZoom200(void);
		void SetZoom300(void);
		void SetNTSC(void);
		void SetPAL(void);
		void ToggleBlur(void);
		void ShowAboutWin(void);
		void ShowHelpWin(void);
		void InsertCart(void);
		void Unpause(void);
		void LoadSoftware(QString);
		void ToggleCDUsage(void);
		void FrameAdvance(void);
		void ToggleFullScreen(void);
		void ShowEmuStatusWin(void);
		void MakeScreenshot(void);
		// Debugger
		void DebuggerTraceStepOver(void);
		void DebuggerTraceStepInto(void);
		void DebuggerRestart(void);
		void ShowAllWatchBrowserWin(void);
		void ShowLocalBrowserWin(void);
		void ShowCallStackBrowserWin(void);
		void ShowHeapAllocatorBrowserWin(void);
		void ShowMemory1BrowserWin(int NumWin);
		void ShowExceptionVectorTableBrowserWin(void);
		void NewBreakpointFunction(void);
		//void ShowVideoOutputWin(void);
		//void ShowDasmWin(void);
		// Alpine
		void ShowMemoryBrowserWin(void);
		void ShowStackBrowserWin(void);
		void ShowCPUBrowserWin(void);
		void ShowOPBrowserWin(void);
		void ShowM68KDasmBrowserWin(void);
		void ShowRISCDasmBrowserWin(void);

	private:
		void HandleKeys(QKeyEvent *, bool);
		void HandleGamepads(void);
		void SetFullScreen(bool state = true);
		void ResizeMainWindow(void);
		void ReadUISettings(void);
		void ReadSettings(void);
		void WriteSettings(void);
		void WriteUISettings(void);

	private:
		GLWidget *videoWidget;
		QMdiArea *mainWindowCentrale;
		QMdiSubWindow *VideoOutputWindowCentrale;
		AboutWindow *aboutWin;
		HelpWindow *helpWin;
		FilePickerWindow *filePickWin;
		EmuStatusWindow *emuStatusWin;
		MemoryBrowserWindow *memBrowseWin;
		StackBrowserWindow *stackBrowseWin;
		CPUBrowserWindow *cpuBrowseWin;
		OPBrowserWindow *opBrowseWin;
		M68KDasmBrowserWindow *m68kDasmBrowseWin;
		RISCDasmBrowserWindow *riscDasmBrowseWin;
		//VideoOutputWindow *VideoOutputWin;
		AllWatchBrowserWindow *allWatchBrowseWin;
		LocalBrowserWindow *LocalBrowseWin;
		CallStackBrowserWindow *CallStackBrowseWin;
		ExceptionVectorTableBrowserWindow *exceptionvectortableBrowseWin;
		HeapAllocatorBrowserWindow *heapallocatorBrowseWin;
		Memory1BrowserWindow **mem1BrowseWin;
		//DasmWindow * DasmWin;
		QTabWidget *dasmtabWidget;
		//QDockWidget *dasmtabWidget;
		m68KDasmWindow *m68kDasmWin;
		GPUDasmWindow *GPUDasmWin;
		DSPDasmWindow *DSPDasmWin;
		FilesrcListWindow *FilesrcListWin;
		//BrkWindow *brkWin;
		QTimer *timer;
		bool running;
		int zoomLevel;
		bool powerButtonOn;
		bool showUntunedTankCircuit;

	public:
		bool cartridgeLoaded;

	private:
		bool allowUnknownSoftware;
		bool CDActive;
//		bool alpineLoadSuccessful;
		bool pauseForFileSelector;
		bool loadAndGo;
		bool keyHeld[8];
		bool fullScreen;
		bool scannedSoftwareFolder;

	public:
		bool plzDontKillMyComputer;
		uint32_t oldTimestamp;
		uint32_t ringBufferPointer;
		uint32_t ringBuffer[RING_BUFFER_SIZE];

	private:
		QPoint mainWinPosition;
//		QSize mainWinSize;
		int lastEditedProfile;
		QMenu *fileMenu;
		QMenu *helpMenu;
		QMenu *debugMenu, *debugWindowsMenu, *debugWindowsBrowsesMenu, *debugWindowsWatchMenu, *debugWindowOutputMenu, *debugWindowExceptionMenu, *debugWindowsMemoryMenu, *debugNewBreakpointMenu;
		QToolBar * toolbar;
		QToolBar * debugbar;
		QToolBar * debuggerbar;

		QActionGroup * zoomActs;
		QActionGroup * tvTypeActs;

		QAction *quitAppAct;
		QAction *powerAct;
		QAction *pauseAct;
		QAction *x1Act;
		QAction *x2Act;
		QAction *x3Act;
		QAction *ntscAct;
		QAction *palAct;
		QAction *blurAct;
		QAction *aboutAct;
		QAction *helpAct;
		QAction *filePickAct;
		QAction *configAct;
		QAction *emustatusAct;
		QAction *useCDAct;
		QAction *frameAdvanceAct;
		QAction *traceStepOverAct;
		QAction *traceStepIntoAct;
		QAction *restartAct;
		QAction *fullScreenAct;
		//QAction *DasmAct;
		QAction *screenshotAct;

		// Alpine
		QAction *memBrowseAct;
		QAction *stackBrowseAct;
		QAction *cpuBrowseAct;
		QAction *opBrowseAct;
		QAction *m68kDasmBrowseAct;
		QAction *riscDasmBrowseAct;

		// Debugger
		//QAction *VideoOutputAct;
		QAction *heapallocatorBrowseAct;
		QAction *allWatchBrowseAct;
		QAction *LocalBrowseAct;
		QAction *CallStackBrowseAct;
		QAction **mem1BrowseAct;
		//QAction *newBreakpointFunctionAct;
		QAction *exceptionVectorTableBrowseAct;

		QIcon powerGreen;
		QIcon powerRed;
		QIcon blur;
		uint32_t testPattern[VIRTUAL_SCREEN_WIDTH * VIRTUAL_SCREEN_HEIGHT_PAL];
		uint32_t testPattern2[VIRTUAL_SCREEN_WIDTH * VIRTUAL_SCREEN_HEIGHT_PAL];
};

#endif	// __MAINWIN_H__
