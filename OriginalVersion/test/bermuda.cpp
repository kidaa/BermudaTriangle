/////////////////////////////////////////////////////////////////////
//	main program bermuda.cpp
#define INCL_DOSPROCESS
	// DosBeep()
#define INCL_DOSSEMAPHORES
#define INCL_WIN
//#define INCL_GPI
// #define INCL_GPIERRORS       // PMERR_INV_HPS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>    // _beginthread()
#include <string.h>

#include "game.h"
#include "pmgame.h"
#include "defs.h"
#include "info.h"
#include "tgraph.h"
#include "profile.h"
// #include "mmsound.h"
#include "bermuda.h"

#include "..\..\_Standard\wcapi.h"

#define BEEP_WARN_FREQ	440U
#define BEEP_WARN_DUR	100U

// Prototypes

MRESULT EXPENTRY WndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY HighScoreDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY ProdInfoDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY GameSettingsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY ShowHighDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );


// global variables

HAB hab;
HMQ hmq;
HWND hwndFrame;
HWND hwndMain;
HWND hwndHelp;
HELPINIT HelpInit;


// Semaphore handles
HEV hevWaitAfterScan;	// blocks the found... sounds until scanning is finished
HEV hevHiScoreWin;	// blocks the HiScoreWindow until its sound is played
HEV hevWaitAfterSound;	// blocks the next graphical display until the previous
						// scanning sound has finished
HEV hevWaitSoundReady;	// blocks the next graphical display until the sound is
						// ready to play


char Score;
GLOBALS *pg;
GRAPHBOARD GBoard;
BOOL fHideSquare = TRUE;		// indicates whether the grey square is hidden
GRAPHICS_PARAMETERS graphparms;


main( /* INT argc, CHAR *argv[], CHAR *envp[] */ )
{
    QMSG qmsg;					// message queue
    ULONG fWndCtrlData;		// game Window style flags
    BOOL fRegistered;			//
//	RECTL RectWinPos;			// needed when resizing the window
//	LONG cxScreen;
//	LONG cyScreen;
    INT rc;
    ERRORID error;

    static CHAR szClientClass[] = "bermuda.child";
//	pszProgName = argv[0];		// get the full qualified program name

    if( ( hab = WinInitialize(0) ) == NULLHANDLE ){
	DosBeep( BEEP_WARN_FREQ, BEEP_WARN_DUR );
	return 1;
    }
    if( (hmq = WinCreateMsgQueue( hab, 0 ) ) == NULLHANDLE ){
	error = WinGetLastError( hab );
	DosBeep( BEEP_WARN_FREQ, BEEP_WARN_DUR );
	DosBeep( BEEP_WARN_FREQ + 50, BEEP_WARN_DUR );
	WinTerminate( hab );
	return 1;
    }

    fRegistered = WinRegisterClass( hab, szClientClass, (PFNWP)WndProc,
				    CS_SIZEREDRAW | CS_MOVENOTIFY, 0 );

    if( !fRegistered ){
	rc = WinGetLastError( hab );
	DosBeep( BEEP_WARN_FREQ, BEEP_WARN_DUR );
	DosBeep( BEEP_WARN_FREQ + 50, BEEP_WARN_DUR );
	DosBeep( BEEP_WARN_FREQ + 100, BEEP_WARN_DUR );
	WinDestroyMsgQueue( hmq );
	WinTerminate( hab );
	return 1;
    }

    fWndCtrlData = FCF_MINMAX		| FCF_TASKLIST		|
	FCF_SYSMENU    | FCF_TITLEBAR    |
	FCF_SIZEBORDER | FCF_MENU        |
	FCF_ICON			| FCF_AUTOICON;
/*						
						FCF_ACCELTABLE 
						*/ // use these later
    hwndFrame =
	WinCreateStdWindow( HWND_DESKTOP,	// parent window
			    NULL, //WS_ANIMATE,		// don't make window visible yet
			    &fWndCtrlData,	// window parameters, defined above
			    szClientClass,	// window class, def. above
			    pszAppName,	// Title Bar text
			    0,					// client window style
			    0,					// resources are bound within the .exe
			    IDR_MAIN,			// frame-window identifier (see .rc)
			    &hwndMain );	// output client-window handle

    if( hwndFrame == NULLHANDLE ){
	DosBeep( BEEP_WARN_FREQ, BEEP_WARN_DUR);
	DosBeep( BEEP_WARN_FREQ + 50, BEEP_WARN_DUR);
	DosBeep( BEEP_WARN_FREQ + 100, BEEP_WARN_DUR);
	DosBeep( BEEP_WARN_FREQ + 150, BEEP_WARN_DUR);
	WinDestroyMsgQueue( hmq );
	WinTerminate( hab );
	return 1;
    }

    // allocate space for the gameboard
//	GBoard = new GRAPHBOARD( 6, 9 );

    // initialize the help facility
    HelpInit.cb = sizeof(HELPINIT);
    HelpInit.pszTutorialName = NULL;
    HelpInit.phtHelpTable = (PHELPTABLE)MAKELONG(HELP_TABLE, 0xFFFF );
    HelpInit.hmodHelpTableModule = NULLHANDLE;
    HelpInit.hmodAccelActionBarModule = NULLHANDLE;
    HelpInit.idAccelTable = 0;
    HelpInit.idActionBar = 0;
    HelpInit.pszHelpWindowTitle = "Help for Bermuda Triangle";
    HelpInit.fShowPanelId = CMIC_HIDE_PANEL_ID;
    HelpInit.pszHelpLibraryName = "bermuda.hlp";

    hwndHelp = WinCreateHelpInstance( hab, &HelpInit );
    if( !hwndHelp || HelpInit.ulReturnCode ){	// Help instance creation failed
	WinMessageBox( HWND_DESKTOP, hwndMain,
		       "Couldn't create help instance.\nHelp will be disabled.\n" \
		       "To use the online help the file bermuda.hlp must either be " \
		       "in the current working directory or in one of the " \
		       "directories listed in your HELP environment variable.",
		       "Error when using WinCreateHelpInstance",
		       0, MB_OK | MB_INFORMATION );
    } else {
	if ( !WinAssociateHelpInstance( hwndHelp, hwndFrame ) )
	    WinMessageBox( hwndMain, hwndFrame,
			   "Couldn't associate help instance.\nHelp will be disabled.",
			   "Error when using WinAssociateHelpInstance",
			   0, MB_OK | MB_INFORMATION );
    }


    LONG cxScreen = WinQuerySysValue( HWND_DESKTOP, SV_CXSCREEN );
    LONG cyScreen = WinQuerySysValue( HWND_DESKTOP, SV_CYSCREEN );
wcprintf("jetzt kommt WinSetWindowPos in main: %d %d", cxScreen *2 / 3,
	 cyScreen * 2 / 3 );
    WinSetWindowPos( hwndFrame, NULLHANDLE, cxScreen / 6,
		     cyScreen  / 6,
		     cxScreen  * 2 / 3, cyScreen * 2 / 3,
		     SWP_SIZE | SWP_MOVE );
wcprintf("fertig mit WinSetWindowPos");
    InfoData.SetLineStyle( IDC_DIAGONALS_AND_VERTICALS );
    InfoData.SetSound( TRUE );

//    ReadProfile( hab );

    // Now we can make the active window visible:
    if( !WinShowWindow( hwndFrame, TRUE ) ){
	DosBeep( BEEP_WARN_FREQ, BEEP_WARN_DUR );
	DosBeep( BEEP_WARN_FREQ + 50, BEEP_WARN_DUR );
	DosBeep( BEEP_WARN_FREQ + 100, BEEP_WARN_DUR );
	DosBeep( BEEP_WARN_FREQ + 150, BEEP_WARN_DUR );
	DosBeep( BEEP_WARN_FREQ + 200, BEEP_WARN_DUR );
    }

    // Now we can bring the window to the foreground
    // (it doesn't matter if this isn't successful)
    WinSetActiveWindow( HWND_DESKTOP, hwndFrame );

    // get / dispatch message loop
    while( WinGetMsg( hab, (PQMSG)&qmsg, NULLHANDLE, 0L, 0L ) )
	WinDispatchMsg( hab, (PQMSG)&qmsg );

    // clean up
    WinDestroyWindow( hwndFrame );
    WinDestroyMsgQueue( hmq );
    WinTerminate( hab );
    return 0;

}	// end of main


		
		
MRESULT EXPENTRY WndProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
//	static POINTS DragStartPtrPos;
    POINTL ptl;
    static char Row, Col;
    static BOOL HasMoved = FALSE;
    static BOOL IntroSoundPlayed = FALSE;
    static BOOL RealPaint = TRUE;           // indicates whether the board
				// has to be repainted or just copied
    INT aktscan;
    CHAR msgtext[256];
    ULONG ulResponse;
//	ERRORID errId;

	
    switch( msg ){
    case WM_CREATE:
	if( !InfoData.LoadHigh() ){	// get previously saved highscores
	    InfoData.ResetHigh();
	    WinMessageBox( HWND_DESKTOP, hwndMain,
			   "The file scores.dat " \
			   "(which is in the current directory) was somehow corrupted." \
			   " All Highscores will be reset to Zero.",
			   "Error when loading Highscores",
			   0, MB_OK | MB_INFORMATION );
	}

	// allocate memory for global variables; see GLOBALS struct in tgraph.h
	pg = new GLOBALS;
	// initialize globals to zero
	memset( pg, 0, sizeof( GLOBALS ));
	// store globals pointer into client window words; see WinRegisterClass
//    WinSetWindowULong( hwnd, QWL_USER, (ULONG) pg );

    wcprintf("1: %x", WinGetLastError( hab ) );
	DosCreateEventSem( NULL, &hevWaitAfterScan, 0, FALSE );
				// Sem is created in reset state
	DosCreateEventSem( NULL, &hevHiScoreWin, 0, FALSE );
	DosCreateEventSem( NULL, &hevWaitAfterSound, 0, FALSE );
	DosCreateEventSem( NULL, &hevWaitSoundReady, 0, TRUE );
				// Sem is created in posted state
	// hevWaitAfterScan and hewWaitAfterSound are used to indicate
	// when the respective WM_CREATE routines are done.
	// after that they are in posted state, as desired
            
	// initialize globals with important data
	pg->hab          = hab;
	pg->hwndClient   = hwnd;
	pg->hwndFrame    = WinQueryWindow( hwnd, QW_PARENT );
	pg->hwndTitlebar = WinWindowFromID( pg->hwndFrame, FID_TITLEBAR );
	pg->hwndMenubar  = WinWindowFromID( pg->hwndFrame, FID_MENU );
	// create graphics and sound threads
	pg->tidTSound = _beginthread( &threadsound, NULL, LEN_STACK, NULL );
	pg->tidTGraph = _beginthread( &threadgraph, NULL, LEN_STACK, NULL );
	DosWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
	WinPostMsg( pg->hwndTSound, WM_SOUND_INTRO, MPFROMHWND(hwnd), 0 );
	// wait for the sound's WM_CREATE
	DosWaitEventSem( hevWaitAfterScan, SEM_INDEFINITE_WAIT );
	// wait for the graphics' WM_CREATE
	InfoData.ShipsNotFound = GBoard.GetShipNumber();
	wcprintf("create: %x", WinGetLastError( hab ) );
	return (MRESULT)0;
			
    case WM_CONTROL:
	break;
    case WM_QUIT:
	break;
			
    case WM_CLOSE:	// this message is sent before WM_QUIT
	InfoData.SaveHigh( WinGetCurrentTime(hab) );
				// save the highscores and provide a random seed
	// get pointer to globals from window words
//    pg = (PGLOBALS) WinQueryWindowULong( hwnd, QWL_USER );
	// tell object windows to quit, then exit their threads
//			WinSendMsg( pg->hwndTGraph, WM_DESTROY, mp1, mp2 );
	WinPostMsg( pg->hwndTGraph, WM_QUIT, mp1, mp2 );
//			WinSendMsg( pg->hwndTSound, WM_DESTROY, mp1, mp2 );
	WinPostMsg( pg->hwndTSound, WM_QUIT, mp1, mp2 );
	DosCloseEventSem( hevWaitAfterScan );
	DosCloseEventSem( hevHiScoreWin );
	DosCloseEventSem( hevWaitAfterSound );
	DosCloseEventSem( hevWaitSoundReady );
	WriteProfile( hab );
	delete pg;
	return (MRESULT) 0;
	
    case WM_ERASEBACKGROUND:
	wcprintf("erasebackground");        
//	return (MRESULT) FALSE;
	return (MRESULT) TRUE;

    case WM_PAINT:
///////////////////////////////////
	    {
		RECTL rectl;
		WinQueryWindowRect( pg->hwndClient, &rectl );
wcprintf("Linienrechteck: Breite: %d H”he: %d", rectl.xRight, rectl.yTop );
		// test size:
		GpiSetColor( hpsGlob, CLR_RED );
		ptl.x = rectl.xLeft; ptl.y = rectl.yBottom;
		GpiMove( hpsGlob, &ptl );
		ptl.x = rectl.xRight; ptl.y = rectl.yTop;
		GpiLine( hpsGlob, &ptl );
	    }
///////////////////////////
	break;

    case WM_SIZE:
	wcprintf("main wnd function wm-size");
	RealPaint = TRUE;
	GBoard.SetPMBoardValues( SHORT1FROMMP( mp2 ), SHORT2FROMMP( mp2 ) );
	WndResize( hwnd );
	wcprintf("size: %x", WinGetLastError( hab ) );
	break;
			
    case WM_BEGINDRAG:
	WinSetCapture( HWND_DESKTOP, hwnd );	// capture the mouse pointer
	GBoard.SetfDrag( TRUE );	// indicate that mouse is being dragged
	GBoard.ResetFirstDraw();	// for initialization of drag op.
	fHideSquare = TRUE;
	WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
		    MPFROM2SHORT( 0, 0 ) );
//			GBoard.ShowPointerPos( hwnd, 0, 0 ); // removes the square
	ptl.x = SHORT1FROMMP(mp1);
	ptl.y = SHORT2FROMMP(mp1);
	Row = GBoard.GetBoardRow( ptl.y );	// starting point of drag
	Col = GBoard.GetBoardCol( ptl.x );	// operation; static!
	return (MRESULT)TRUE;
			
    case WM_MOUSEMOVE:
	if( GBoard.GetfDrag() ){	// if mouse is being dragged
	    WinSendMsg( pg->hwndTGraph, WM_DRAWDRAGLINE, mp1,
			MPFROM2SHORT( Row, Col ) );
	    HasMoved = TRUE;
	} else {		// mouse is moved normally
	    if( !fHideSquare )
		WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
			    mp1 );
//					GBoard.ShowPointerPos( hwnd, SHORT1FROMMP(mp1),
//														  SHORT2FROMMP(mp1));
	}
	break;	
    case WM_ENDDRAG:
	WinSetCapture( HWND_DESKTOP, NULLHANDLE ); // release the captured
				// mouse pointer
	if( HasMoved ){	// mousemove has actually been moved
	    WinSendMsg( pg->hwndTGraph, WM_MARKDRAGLINE,
			MPFROM2SHORT( Row, Col ), 0 );
	    HasMoved = FALSE;
	}
	GBoard.SetfDrag( FALSE );
	GBoard.ClearDrawPoint();	// because no square is drawn right now
	fHideSquare = FALSE;
	WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
		    mp1 );
//			GBoard.ShowPointerPos( hwnd, SHORT1FROMMP(mp1), SHORT2FROMMP(mp1));
				// draws square at the current ptr pos
	break;
			
    case WM_CHAR:	// key was pressed
	if( SHORT2FROMMP( mp2 ) != VK_SPACE ) break;	// only space is interesting
	if( GBoard.GetfDrag() ) break;	// do nothing while dragging
	if( !GBoard.GetfShowLines() ){	// lines not visible yet
	    GBoard.SetfShowLines( TRUE );
	    WinSendMsg( pg->hwndTGraph, WM_DISPLAYLINES, 0, 0 );
	}
	break;
			
    case WM_BUTTON1CLICK:
	if( !InfoData.ShipsNotFound ) break;	// game is finished
	ptl.x = (LONG)SHORT1FROMMP( mp1 );
	ptl.y = (LONG)SHORT2FROMMP( mp1 );
	Row = GBoard.GetBoardRow( ptl.y );
	Col = GBoard.GetBoardCol( ptl.x );
	if( !Row || !Col ) break;
	fHideSquare = TRUE;
	WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
		    MPFROM2SHORT( 0, 0 ) );
//			GBoard.ShowPointerPos( hwnd, 0, 0 );	// hides pointer square
	if(( aktscan = GBoard.GetDiscovered( Row, Col )) != -1 ){
	    WinSendMsg( pg->hwndTGraph, WM_DRAWPMPLACE, MPFROMHWND(hwnd),
			MPFROMSH2CH( MAKESHORT(Row, Col),
				     (CHAR)aktscan,(CHAR)TRUE));
// umstricken auf WinPostMsg												 
				// toggle Place display
	} else {	// scan Place
	    DosResetEventSem( hevWaitAfterScan, &ulResponse );
// DosBeep(500, 150 );				
	    WinPostMsg( pg->hwndTGraph, WM_GRAPH_SCAN, MPFROMHWND(hwnd),
			MPFROM2SHORT( Row, Col ) );
// DosBeep( 800, 150 );								
	    WinWaitEventSem( hevWaitAfterScan, SEM_INDEFINITE_WAIT );
// DosBeep( 1000, 150 );				
				// first the scanning sounds must be played (and finished)
	    aktscan = GBoard.Scan( Row, Col );
	    if( aktscan == GBoard.GetShipNumber() + 10 ){
		InfoData.ShipsNotFound--;
		WinPostMsg( pg->hwndTSound, WM_SOUND_FOUNDSHIP, MPFROMHWND(hwnd), 0 );
	    } else {
		if( aktscan )
		    WinPostMsg( pg->hwndTSound, WM_SOUND_FOUND,
				MPFROMHWND(hwnd), MPFROMLONG( aktscan ) );
		else	
		    WinPostMsg( pg->hwndTSound, WM_SOUND_FOUND0, MPFROMHWND(hwnd), 0 );
	    }
	    WinWaitEventSem( hevWaitAfterScan, SEM_INDEFINITE_WAIT );
				// waits until scanning is done, and only then displays the
				// field icon
//				hps = WinGetPS( hwnd );
	    WinSendMsg( pg->hwndTGraph, WM_DRAWPMPLACE, MPFROMHWND(hwnd),
			MPFROMSH2CH( MAKESHORT(Row, Col),
				     (CHAR)aktscan,(CHAR)TRUE));
// umstricken auf WinPostMsg
	    WinPostMsg( pg->hwndTGraph, WM_SHOWSTATUSLINE, 0, 0 );
				
//				ShowStatusLine( hps, GBoard.MovesNeeded(), InfoData.ShipsNotFound,
//									 GBoard.GetWinWidth(), GBoard.GetWinHeight() );
//				WinReleasePS( hps );
	    if( !InfoData.ShipsNotFound ){	// game is finished, all ships found
		Score = GBoard.MovesNeeded();
		if	( !InfoData.ReturnLastHigh()	// still space in the hiscore table
		|| Score < InfoData.ReturnLastHigh() ){	// player kicks last one out
				// player enters highscore table
		    WinPostMsg( pg->hwndTSound, WM_SOUND_NEWHISCORE, MPFROMHWND(hwnd), 0 );
		    WinWaitEventSem( hevHiScoreWin, SEM_INDEFINITE_WAIT );
				// waits until the NEWHISCORE sound is actually played
		    WinDlgBox( HWND_DESKTOP, hwnd, HighScoreDlgProc, (HMODULE)0,
			       IDR_HIGHSCOREDLG, NULL );
		    WinPostMsg( hwnd, WM_COMMAND,
				MPFROMSHORT(IDM_GAMEHIGH), (MPARAM)0 );
				// show highscore-table
		    DosResetEventSem( hevHiScoreWin, &ulResponse ); // resets the sem again
		} else {
		    WinPostMsg( pg->hwndTSound, WM_SOUND_LOST, MPFROMHWND(hwnd), 0 );
		    WinWaitEventSem( hevHiScoreWin, SEM_INDEFINITE_WAIT );
				// waits until the NEWHISCORE sound is actually played
		    sprintf( msgtext,
			     "You needed %d moves to find the lost ships. " \
			     "To enter the highscore list you need %d moves." \
			     " So try again!", Score, InfoData.ReturnLastHigh() - 1 );
		    WinMessageBox( HWND_DESKTOP, hwnd, msgtext, "Oh, Shit!", 0,
				   MB_OK | MB_INFORMATION | MB_HELP );
		}
	    }
	}
	fHideSquare = FALSE;
	WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
		    MPFROM2SHORT( ptl.x, ptl.y ) );
//			GBoard.ShowPointerPos( hwnd, ptl.x, ptl.y ); // redisplay ptr square
	break;
			
    case WM_BUTTON2CLICK:
	fHideSquare = TRUE;
	WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
		    MPFROM2SHORT( 0, 0 ) );
	ptl.x = (LONG)SHORT1FROMMP( mp1 );
	ptl.y = (LONG)SHORT2FROMMP( mp1 );
	Row = GBoard.GetBoardRow( ptl.y );
	Col = GBoard.GetBoardCol( ptl.x );
	WinSendMsg( pg->hwndTGraph, WM_DRAWPMMARK, MPFROMHWND(hwnd),
		    MPFROM2SHORT( Row, Col ) );
	fHideSquare = FALSE;
	WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
		    MPFROM2SHORT( ptl.x, ptl.y ) );
	break;
			
    case WM_COMMAND:
	switch( SHORT1FROMMP( mp1 ) ){
	case IDM_GAMENEW:
	    GBoard.NewGame();
	    InfoData.ShipsNotFound = GBoard.GetShipNumber();
	    RealPaint = TRUE;
	    WinInvalidateRect( hwnd, NULL, TRUE );
	    break;
	case IDM_GAMESETTINGS:
	    if( WinDlgBox( HWND_DESKTOP, hwndFrame,
			   GameSettingsDlgProc, (HMODULE)0,
			   IDR_GAMESETTINGSDLG, NULL ) ){
				// screen must be repainted
		RealPaint = TRUE;
		WinInvalidateRect( hwnd, NULL, TRUE );
	    }
	    break;
	case IDM_GAMEHIGH:
	    if( !WinDlgBox( HWND_DESKTOP, hwndFrame,
			    ShowHighDlgProc, (HMODULE)0,
			    IDR_SHOWHIGHDLG, NULL ) ){	// user requested "Clear"
		if( WinMessageBox( HWND_DESKTOP, hwndMain,
				   "Do you really want to eradicate all those " \
				   "arduously achieved highscores?",
				   "Clear Highscores",
				   0, MB_OKCANCEL | MB_WARNING ) == MBID_OK )
		    InfoData.ResetHigh();
	    }
	    break;
					
	case IDM_HELPINDEX:	// help index
	    WinSendMsg( hwndHelp, HM_HELP_INDEX, 0, 0 );
	    break;
					
	case IDM_HELPGENERAL:	// general help
	    WinSendMsg( hwndHelp, HM_EXT_HELP, 0, 0 );
	    break;
					
	case IDM_HELPEXTENDED:	// help on help (system page)
	    WinSendMsg( hwndHelp, HM_DISPLAY_HELP, 0, 0 );
	    break;
					
	case IDM_HELPKEYS:	// keys help
	    WinSendMsg( hwndHelp, HM_KEYS_HELP, 0, 0 );
	    break;
					
	
	case IDM_HELPPRODUCTINFO:
	    ulResponse = WinDlgBox( HWND_DESKTOP, hwndFrame,
				    ProdInfoDlgProc, (HMODULE)0,
				    IDR_PRODINFODLG, NULL );
	    break;		
	}
	break;

    case HM_QUERY_KEYS_HELP:                // system asks which page to display
	return MRFROMSHORT( PANEL_HELPKEYS );
	      
    case HM_HELPSUBITEM_NOT_FOUND:
	return (MRESULT)FALSE;
			
    case WM_USER_ACK:	// graphics task finished its work
//         DosBeep( 1000, 150 );
	switch( (ULONG)mp1 ){
	case WM_USER_PAINT:
	    WinQueryPointerPos( HWND_DESKTOP, &ptl );
	    WinMapWindowPoints( HWND_DESKTOP, hwnd, &ptl, 1);
	    fHideSquare = FALSE;
	    WinSendMsg( pg->hwndTGraph, WM_SHOWPOINTERPOS, MPFROMHWND(hwnd),
			MPFROM2SHORT( ptl.x, ptl.y ) );
//					GBoard.ShowPointerPos( hwnd, ptl.x, ptl.y );
				// painting has finished, square can be displayed now
	    break;
	}
	break;	
    case WM_SOUND_ACK:
	switch( (ULONG)mp1 ){
	case WM_SOUND_INTRO:
	    break;
	}
	break;

			
    default:
	return (MRESULT)WinDefWindowProc( hwnd, msg, mp1, mp2 );
    }		// end switch( msg )
    return (MRESULT)WinDefWindowProc( hwnd, msg, mp1, mp2 );
}		// end MRESULT EXPENTRY WndProc()

	
MRESULT EXPENTRY HighScoreDlgProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
	char PlayerName[21];

	switch( msg ){
		case WM_INITDLG:
			WinSetDlgItemShort( hwnd, IDR_SCORENUMBER, (USHORT)Score, FALSE);
			return (MRESULT)0;
		case WM_COMMAND:
			switch( SHORT1FROMMP( mp1 ) ){
				case DID_OK:
					WinQueryDlgItemText( hwnd, ID_NAME, 20, PlayerName );
					if( PlayerName[0] == '\0' )
						strcpy( PlayerName, "<didn't tell me>" );
					InfoData.InsertHigh( PlayerName, Score );
					// Score is a global variable
					WinDismissDlg( hwnd, TRUE );
					return (MRESULT)0;
				default:
					return WinDefDlgProc( hwnd, msg, mp1, mp2 );
			}
	}
	return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

MRESULT EXPENTRY ProdInfoDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
	switch( msg ){
		case WM_INITDLG:
			return (MRESULT)0;
		case WM_COMMAND:
			WinDismissDlg( hwnd, TRUE );
			return (MRESULT)0;
	}
	return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

MRESULT EXPENTRY GameSettingsDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
	ULONG OldLineStyle = InfoData.GetLineStyle();
	switch( msg ){
		case WM_INITDLG:
			(VOID)WinCheckButton( hwnd, (USHORT)IDC_DIAGONALS, InfoData.IsLineStyle(IDC_DIAGONALS) );
			(VOID)WinCheckButton( hwnd, (USHORT)IDC_VERTICALS, InfoData.IsLineStyle(IDC_VERTICALS));
			(VOID)WinCheckButton( hwnd, (USHORT)IDC_DIAGONALS_AND_VERTICALS, InfoData.IsLineStyle(IDC_DIAGONALS_AND_VERTICALS));
			(VOID)WinCheckButton( hwnd, (USHORT)IDC_SOUND, InfoData.GetSound() );
			(VOID)WinEnableControl( hwnd, (USHORT)IDC_SOUND,
											InfoData.IsSoundAvailable() );
			break;
		case WM_COMMAND:
			switch( SHORT1FROMMP( mp1 ) ){
				case DID_OK:
					if( WinQueryButtonCheckstate( hwnd, IDC_DIAGONALS ) )
						InfoData.SetLineStyle(IDC_DIAGONALS);
					else
						if( WinQueryButtonCheckstate( hwnd, IDC_VERTICALS ) )
							InfoData.SetLineStyle(IDC_VERTICALS);
						else InfoData.SetLineStyle(IDC_DIAGONALS_AND_VERTICALS);
					InfoData.SetSound( WinQueryButtonCheckstate(hwnd, IDC_SOUND) );
					WinDismissDlg( hwnd, !InfoData.IsLineStyle(OldLineStyle) );
						// TRUE => repaint because of new linestyle
					return (MRESULT)0;
				case DID_CANCEL:
					WinDismissDlg( hwnd, FALSE );
					return (MRESULT)0;
				case IDC_GAMESETTINGSHELP:
					return (MRESULT)0;				
			}
	}	
	return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

MRESULT EXPENTRY ShowHighDlgProc(HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 )
{
	CHAR i;
	switch( msg ){
		case WM_INITDLG:
			for( i = 0; i < 10; i++){
				WinSetDlgItemText( hwnd, ULONG(321 + i) , InfoData.GetHighName(i) );
				// item IDR_HIGHNAME1 ... IDR_HIGHNAME10
				WinSetDlgItemShort( hwnd, ULONG(331 + i), (USHORT)InfoData.GetHighScore(i),
										  FALSE);
				// item IDR_HIGHSCORE1 ... IDR_HIGHSCORE10
			}
			break;
		case WM_COMMAND:
			switch( SHORT1FROMMP( mp1 ) ){
				case DID_OK:
					WinDismissDlg( hwnd, TRUE );
					return (MRESULT)0; 
				case IDR_HIGHCLEAR:
					WinDismissDlg( hwnd, FALSE );
					return (MRESULT)0;
			}
	}
	return WinDefDlgProc( hwnd, msg, mp1, mp2 );
}

