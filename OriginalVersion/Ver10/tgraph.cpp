////////////////////////////////////////
// start of file tgraph.cpp
// maintains the graphics thread
// 

// os2 includes
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#define INCL_WIN
#include <os2.h>
// crt includes
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <process.h>	// _endthread()
// app includes
// #include "app.h"
#include "defs.h"
#include "tgraph.h"
#include "game.h"
#include "pmgame.h"
#include "info.h"
#include "bermuda.h"
#include "mmsound.h"

//#include "..\_Standard\wcapi.h"


// global sound variables

// must not call the constructors before MMPM-Support is loaded
mem_wav *found0,
		  *foundship,
		  *found,
		  *newhiscore,
		  *hochtief,
		  *lost,
 		  *intro;

 		  
 		  
//----------------------------------------------------------------------
// thread 2 entry point: gets and dispatches object window messages
void threadgraph(void *dummy )      // the 1st parameter for _beginthread must be void(*)(void *)
{
  BOOL       fSuccess;
  HAB        hab;
  HMQ        hmq;
  QMSG       qmsg;


  // thread initialization
  dummy = NULL;	// to get rid of error messages
  hab = WinInitialize( 0 );
  hmq = WinCreateMsgQueue( hab, 0 );

  // prevent system from posting object window a WM_QUIT
  // I'll post WM_QUIT when it's time.
//  fSuccess = WinCancelShutdown( hmq, TRUE );

  fSuccess = WinRegisterClass( hab, APP_CLASS_OBJECT1,
                  (PFNWP)TGraphWinProc, 0, 0 );

  pg->hwndTGraph = WinCreateWindow( HWND_OBJECT, APP_CLASS_OBJECT1, "",
             0, 0, 0, 0, 0, HWND_OBJECT, HWND_BOTTOM, 0, (PVOID)pg, NULL );

  // created OK, ack client
  WinPostMsg( pg->hwndClient, WM_USER_ACK, 0, 0 );

  // get/dispatch messages; user messages, for the most part
  while( WinGetMsg ( hab, &qmsg, 0, 0, 0 ))
  {
    WinDispatchMsg ( hab, &qmsg );
  }

  // tell client window to quit; unnecessary since TSound does this
//  WinPostMsg( pg->hwndClient, WM_QUIT, 0, 0 );

  // clean up
  WinDestroyWindow( pg->hwndTGraph );
  WinDestroyMsgQueue( hmq );
  WinTerminate( hab );
  _endthread();
  return;
}

// --------------------------------------------------------------------------
// object window procedure; mp1 is the window to acknowledge upon completion
//
// hwnd is the graphics hwnd, msg is the task to do, mp1 is the hwnd of the
// ordering window, mp2 is some additional parameter

MRESULT EXPENTRY TGraphWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
	HWND hwndToAck;
	static HPS hps;
	RECTL rectl;
	POINTL ptl;
	ULONG ulPC;
	INT TIME_ON = 100, TIME_OFF = 15;


  // store the handle of the window to ack upon task completion;
	hwndToAck = HWNDFROMMP(mp1);

  switch( msg ) {
  	
  case WM_CREATE:
    // for the create case, mp1 is pointer to globals;
    // save it in object window words; dependency on WinRegisterClass
//    pg = (GLOBALS) mp1;
//    WinSetWindowULong( hwnd, QWL_USER, (ULONG) pg  );
		hps = WinGetPS( pg->hwndClient );
    return (MRESULT) 0;

	case WM_USER_PAINT:
		rectl.xLeft = rectl.yBottom = 0;
		rectl.xRight = GBoard.GetWinWidth();
		rectl.yTop = GBoard.GetWinHeight();
//		GBoard.ShowPointerPos( hwndToAck, 0, 0 ); // removes the square
		WinFillRect( hps, &rectl, CLR_WHITE );	// clear background
		ShowStatusLine( hps, GBoard.MovesNeeded(), InfoData.ShipsNotFound,
							 GBoard.GetWinWidth(), GBoard.GetWinHeight() );
		GBoard.DrawPMBoard( hps );
		GBoard.ClearDrawPoint();	// because no square is drawn right now
		WinQueryPointerPos( HWND_DESKTOP, &ptl );
		WinMapWindowPoints( HWND_DESKTOP, hwndToAck, &ptl, 1);
		GBoard.ShowPointerPos( hps, ptl.x, ptl.y );
			// draws square at the current ptr pos
	   // tell originating window that the task is complete
    	WinPostMsg( hwndToAck, WM_USER_ACK, (MPARAM) msg, 0 );
//    	return (MRESULT) 0;
		break;
		
	case WM_GRAPH_SCAN:
		POINTL left, right, top, bot, lefttop, leftbot, righttop, rightbot,
				 center;
		GBoard.CalcScanLines( (CHAR)SHORT1FROMMP( mp2 ),
									 (CHAR)SHORT2FROMMP( mp2 ),
									 left, right, top, bot, lefttop, leftbot,
								 	 righttop, rightbot, center );

		if( center.x != righttop.x || center.y != righttop.y ){
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, righttop, TIME_ON, TIME_OFF );
			// turns the line on, waits TIME_ON, turns line off, waits TIME_OFF
		}	// only do something if a line will be drawn
	
		if( center.x != top.x || center.y != top.y ){
			WinWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, top, TIME_ON, TIME_OFF );
		}
		
		if( center.x != lefttop.x || center.y != lefttop.y ){
			DosWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, lefttop, TIME_ON, TIME_OFF );
		}
		
		if( center.x != left.x || center.y != left.y ){
			DosWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, left, TIME_ON, TIME_OFF );
		}
		
		if( center.x != leftbot.x || center.y != leftbot.y ){
			DosWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, leftbot, TIME_ON, TIME_OFF );
		}
		
		if( center.x != bot.x || center.y != bot.y ){
			DosWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, bot, TIME_ON, TIME_OFF );
		}
		
		if( center.x != rightbot.x || center.y != rightbot.y ){
			DosWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, rightbot, TIME_ON, TIME_OFF );
		}
		
		if( center.x != right.x || center.y != right.y ){
			DosWaitEventSem( hevWaitAfterSound, SEM_INDEFINITE_WAIT );
			DosResetEventSem( hevWaitAfterSound, &ulPC );
			DosResetEventSem( hevWaitSoundReady, &ulPC );
			WinPostMsg( pg->hwndTSound, WM_SOUND_HOCHTIEF, MPFROMHWND(hwnd), 0 );
			WinWaitEventSem( hevWaitSoundReady, SEM_INDEFINITE_WAIT );
			GBoard.OneLine( hps, center, right, TIME_ON );
			// don't wait after the line has been turned off
		}
	
		DosPostEventSem( hevWaitAfterScan );
    	WinPostMsg( hwndToAck, WM_USER_ACK, (MPARAM) msg, 0 );
		break;
		
	case WM_DRAWPMPLACE:
		GBoard.DrawPMPlace( hps, CHAR1FROMMP( mp2 ), CHAR2FROMMP( mp2 ),
										 CHAR3FROMMP( mp2 ), CHAR4FROMMP( mp2 ) );
		break;
		
	case WM_DRAWPMMARK:
		GBoard.DrawPMMark( hps, (CHAR)SHORT1FROMMP( mp2 ),
										(CHAR)SHORT2FROMMP( mp2 ) );
		break;
		
	case WM_SHOWPOINTERPOS:
		GBoard.ShowPointerPos( hps, SHORT1FROMMP( mp2 ), SHORT2FROMMP( mp2 ) );
		break;

	case WM_DISPLAYLINES:	
		GBoard.DisplayLines( hps );
		break;
		
	case WM_DRAWDRAGLINE:	// draw the dragging line
		ptl.x = SHORT1FROMMP(mp1);
		ptl.y = SHORT2FROMMP(mp1);
		GBoard.DrawDragLine( hps, (CHAR)SHORT1FROMMP(mp2),
									(CHAR)SHORT2FROMMP(mp2), ptl );
		break;

	case WM_MARKDRAGLINE:	// mark all places along the dragging line
		GBoard.MarkDragLine( hps, (CHAR)SHORT1FROMMP(mp1),
										  (CHAR)SHORT2FROMMP(mp1) );
		break;

	case WM_SHOWSTATUSLINE:
		ShowStatusLine( hps, GBoard.MovesNeeded(), InfoData.ShipsNotFound,
									 GBoard.GetWinWidth(), GBoard.GetWinHeight() );

		break;	
		
	case WM_DESTROY:
		WinReleasePS( hps );
		return 0;
  }

  // default:
  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

//
//-----------thread 3 entry point: sound thread
//  gets and dispatches object window messages
//
void threadsound(void *dummy )      // the 1st parameter for _beginthread must be void(*)(void *)
{
  BOOL       fSuccess;
  HAB        hab;
  HMQ        hmq;
  QMSG       qmsg;


  // thread initialization
  dummy = NULL;	// to get rid of error messages
  hab = WinInitialize( 0 );
  hmq = WinCreateMsgQueue( hab, 0 );

  // prevent system from posting object window a WM_QUIT
  // I'll post WM_QUIT when it's time.
//  fSuccess = WinCancelShutdown( hmq, TRUE );

  fSuccess = WinRegisterClass( hab, APP_CLASS_OBJECT2,
                  (PFNWP)TSoundWinProc, 0, 0 );

  pg->hwndTSound = WinCreateWindow( HWND_OBJECT, APP_CLASS_OBJECT2, "",
             0, 0, 0, 0, 0, HWND_OBJECT, HWND_BOTTOM, 0, (PVOID)pg, NULL );

  // created OK, ack client
  WinPostMsg( pg->hwndClient, WM_SOUND_ACK, 0, 0 );

  // get/dispatch messages; user messages, for the most part
	while( WinGetMsg ( hab, &qmsg, 0, 0, 0 ))
		WinDispatchMsg ( hab, &qmsg );

  // tell client window to quit
  WinPostMsg( pg->hwndClient, WM_QUIT, 0, 0 );

  // clean up
  WinDestroyWindow( pg->hwndTSound );
  WinDestroyMsgQueue( hmq );
  WinTerminate( hab );
  _endthread();	// suggestion taken from edmi 1-2
}

// --------------------------------------------------------------------------
// object window procedure; mp1 is the window to acknowledge upon completion
//
// hwnd is the graphics hwnd, msg is the task to do, mp1 is the hwnd of the
// ordering window, mp2 is some additional parameter

MRESULT EXPENTRY TSoundWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  HWND          hwndToAck;


  // store the handle of the window to ack upon task completion;
  hwndToAck = HWNDFROMMP(mp1);

  switch( msg ) {
  case WM_CREATE:
  {
  		CHAR tmp[20];
    // for the create case, mp1 is pointer to globals;
    // save it in object window words; dependency on WinRegisterClass
//    pg = (GLOBALS) mp1;
//    WinSetWindowULong( hwnd, QWL_USER, (ULONG) pg  );
		InfoData.SetSoundAvailability( !LoadMMPMSupport() );
			// LoadMMPMSupport returns FALSE on success
		if( InfoData.IsSoundAvailable() ){	// now the constructors can be called
			strcpy( tmp, "found0.wav" );
			found0 = new mem_wav( tmp );
			if( !tmp[0] ){		// error in constructor
				InfoData.SetSoundAvailability( FALSE );
				return 0;
			}	
//			strcpy( tmp, "intro.wav");
//			intro = new mem_wav( tmp );
//			if( !tmp[0] ){		// error in constructor
//				InfoData.SetSoundAvailability( FALSE );
//				return 0;
//			}	
//			PlaySound( intro, InfoData.GetSound() );
//			// play the intro sound as early as possible
			strcpy( tmp, "foundsh.wav");
			foundship = new mem_wav( tmp );
			if( !tmp[0] ){		// error in constructor
				InfoData.SetSoundAvailability( FALSE );
				return 0;
			}	
			strcpy( tmp, "found.wav");
			found = new mem_wav( tmp, 4 );
			if( !tmp[0] ){		// error in constructor
				InfoData.SetSoundAvailability( FALSE );
				return 0;
			}	
			strcpy( tmp, "newhisco.wav");
			newhiscore = new mem_wav( tmp );
			if( !tmp[0] ){		// error in constructor
				InfoData.SetSoundAvailability( FALSE );
				return 0;
			}	
			strcpy( tmp, "hochtief.wav");
			hochtief = new mem_wav( tmp );
			if( !tmp[0] ){		// error in constructor
				InfoData.SetSoundAvailability( FALSE );
				return 0;
			}	
			strcpy( tmp, "lost.wav");
 			lost = new mem_wav( tmp );
			if( !tmp[0] ){		// error in constructor
				InfoData.SetSoundAvailability( FALSE );
				return 0;
			}	
 		}
	}
	return (MRESULT) 0;

	case WM_DESTROY:
		FreeMMPMSupport();	// doesn't do any harm if sound is not available
		return 0;
		
	case WM_SOUND_INTRO:
		PlaySound( intro, InfoData.GetSound() );
    	return (MRESULT) 0;

	case WM_SOUND_FOUND0:
		WinWaitEventSem( hevWaitAfterScan, SEM_INDEFINITE_WAIT );
			// wait till scanning is done
      PlaySound( found0, InfoData.GetSound() );
    	return (MRESULT) 0;

	case WM_SOUND_FOUNDSHIP:
		WinWaitEventSem( hevWaitAfterScan, SEM_INDEFINITE_WAIT );
      PlaySound( foundship, InfoData.GetSound() );
    	return (MRESULT) 0;

	case WM_SOUND_FOUND:
		WinWaitEventSem( hevWaitAfterScan, SEM_INDEFINITE_WAIT );
		if( InfoData.GetSound() )	// (*found)( x ) ist nicht gesichert!
	      PlaySound( &((*found)(LONGFROMMP(mp2)) ), InfoData.GetSound() );
      // dereference the pointer "found", assign count (contained in mp2)
		// to it with operator() and reference it again
    	return (MRESULT) 0;

	case WM_SOUND_HOCHTIEF:
    	DosPostEventSem( hevWaitSoundReady );	// signal that sound will start now
      PlaySound( hochtief, InfoData.GetSound() );
    	DosPostEventSem( hevWaitAfterSound );	// signal that sound is finished
    	return (MRESULT) 0;

	case WM_SOUND_NEWHISCORE:
		DosPostEventSem( hevHiScoreWin ); // posts the sem so that the
			// HiScore dialog window can be displayed
      PlaySound( newhiscore, InfoData.GetSound() );
    	return (MRESULT) 0;

	case WM_SOUND_LOST:
		DosPostEventSem( hevHiScoreWin ); // posts the sem so that the
			// "Lost" dialog window can be displayed
      PlaySound( lost, InfoData.GetSound() );
    	return (MRESULT) 0;
  }

  // default:
  return WinDefWindowProc( hwnd, msg, mp1, mp2 );
}

