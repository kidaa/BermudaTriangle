/*--------------------------------------------------------------*\
 *  Include files, macros, defined constants, and externs
\*--------------------------------------------------------------*/

#define INCL_DEV
#define INCL_WININPUT
#define INCL_WINFRAMEMGR
#define INCL_WINTRACKRECT
#define INCL_WINMENUS
#define INCL_WINSYS
#define INCL_WINTIMER
#define INCL_WINMESSAGEMGR
#define INCL_WINSHELLDATA
#define INCL_WINWINDOWMGR
#define INCL_WINHELP
#define INCL_GPICONTROL
#define INCL_GPIPRIMITIVES
#define INCL_GPILOGCOLORTABLE
#define INCL_GPIBITMAPS
#define INCL_GPITRANSFORMS
#define INCL_DOSSEMAPHORES
#define INCL_DOSDATETIME


#include <os2.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "clock.h"
#include "res.h"
#include "clkdata.h"
#include "help.h"

/*--------------------------------------------------------------*\
 *  Global variables and definitions for this file
\*--------------------------------------------------------------*/

/* Fields marked (RT) are filled in at run-time */

#define RESOURCEFLAG	0xFFFF0000

CHAR szHelpTitle [MAXTITLELENGTH + 1];


/*d-------------------------------------------------------------*\
 *  Entry point declarations
\*-------------------------------------------------------------k*/

VOID ClkPaint (HWND);
VOID ClkCreate (HWND);
VOID ClkDestroy (HWND);
VOID ClkSize (HWND);
VOID ClkTimer (HWND);

/****************************************************************\
 *  Routine Name:ClkWndProc()
\****************************************************************/

MRESULT EXPENTRY ClkWndProc (HWND hwnd, ULONG usMsg, MPARAM mp1, MPARAM mp2)
{
    switch (usMsg) {

    case WM_CREATE:
	ClkCreate (hwnd);
	return (WinDefWindowProc (hwnd, usMsg, mp1, mp2));

    case WM_DESTROY:
	ClkDestroy (hwnd);
	return (WinDefWindowProc (hwnd, usMsg, mp1, mp2));

    case WM_PAINT:
	ClkPaint (hwnd);
	break;

    case WM_SIZE:
	ClkSize (hwnd);
	return (WinDefWindowProc (hwnd, usMsg, mp1, mp2));

    default:
	/* let default window procedure handle it. */
	return (WinDefWindowProc (hwnd, usMsg, mp1, mp2));
    }

    return (MRFROMLONG(0));
}



/****************************************************************\
 *
 *--------------------------------------------------------------
 *  Name:ClkCreate()
 *
 *  Purpose:Intialize a newly created client window
 *
 *  Returns:
 *	    1 - if sucessful execution completed
 *	    0 - if error
\****************************************************************/
VOID ClkCreate ( HWND hwnd )
{
    LONG cxScreen , cyScreen;  /* screen dimensions */
    LONG xLeft , yBottom ;	/* frame window location */
    ULONG cbBuf;
    LONG cyHeight;
    LONG cxWidth;

//    hwndClient = hwnd;

// extern SIZEL sizl;   clkdata.h
// SIZEL sizl = { 200 , 200 }; clkdata.c


    /* determine screen dimensions */
    /* open a device context and create a presentation space */

    hdc = WinOpenWindowDC (hwnd);
    hps = GpiCreatePS (hab, hdc, &sizl, PU_ARBITRARY | GPIT_MICRO |
	    GPIA_ASSOC);

    /*
     * Create our off-screen 'buffer'.
     */
    hdcBuffer = DevOpenDC ( (HAB)0L, OD_MEMORY, "*", 0L, NULL, hdc);
    hpsBuffer = GpiCreatePS (hab, hdcBuffer, &sizl, PU_ARBITRARY |
			       GPIT_MICRO | GPIA_ASSOC);

    GpiCreateLogColorTable (hpsBuffer, 0, LCOLF_RGB, 0, 0, (PLONG)NULL);

// ULONG cxRes , cyRes; clkdata.c
// ULONG cColorPlanes, cColorBitcount; clkdata.c


    /* get the device resolutions so we can make the face appear circular */
    DevQueryCaps (hdc, (LONG)CAPS_VERTICAL_RESOLUTION,(LONG) 1L, &cyRes);
    DevQueryCaps (hdc, CAPS_HORIZONTAL_RESOLUTION, 1L, &cxRes);
    DevQueryCaps (hdc, CAPS_COLOR_PLANES, 1L, &cColorPlanes);
    DevQueryCaps (hdc, CAPS_COLOR_BITCOUNT, 1L, &cColorBitcount);

}


/****************************************************************\
 *  Name:ClkDestroy()
 *  Purpose: Destroy clock face.
 *  Returns:VOID
\****************************************************************/
VOID ClkDestroy (HWND hwnd)
{
    HBITMAP hbm;

    hbm = GpiSetBitmap (hpsBuffer, NULLHANDLE);

    if (hbm != NULLHANDLE)
	GpiDeleteBitmap (hbm);

    GpiDestroyPS (hpsBuffer);
    DevCloseDC (hdcBuffer);
}
/****************************************************************\
 *  Name: ClkSize()
 *
 *  Purpose:When the window has been sized, we calculate  a page
 *	    rectangle which: (a) fills the window rectangle in either
 *	    the x or y dimension, (b) appears square, and (c) is centered
 *	    in the window rectangle
\****************************************************************/
VOID ClkSize (HWND hwnd)
{
    RECTL rclWindow;
    SIZEF sizef;
    LONG cxSquare, cySquare, cxEdge, cyEdge;
    LONG cyHeight;
    LONG cxWidth;
    HBITMAP hbm;
    BITMAPINFOHEADER bmp;

    /*
     * First get rid of any buffer bitmap already there.
     */
    hbm = GpiSetBitmap (hpsBuffer, NULLHANDLE);

    if (hbm != NULLHANDLE)
	GpiDeleteBitmap (hbm);

    /*
     * Get the width and height of the window rectangle.
     */
    WinQueryWindowRect (hwnd, &rclWindow);
    cxWidth = rclWindow.xRight - rclWindow.xLeft - 2;
    cyHeight = rclWindow.yTop - rclWindow.yBottom - 2;

    /*
     * Now create a bitmap the size of the window.
     */
    bmp.cbFix = sizeof(BITMAPINFOHEADER);
    bmp.cx = (SHORT)cxWidth;
    bmp.cy = (SHORT)cyHeight;
    bmp.cPlanes = (SHORT)cColorPlanes;
    bmp.cBitCount = (SHORT)cColorBitcount;
    hbm = GpiCreateBitmap(hpsBuffer, (PBITMAPINFOHEADER2)&bmp,
			  0x0000, (PBYTE)NULL, (PBITMAPINFO2)NULL);
    GpiSetBitmap (hpsBuffer, hbm);

    /*
     * Assume the size of the page rectangle is constrained in the y
     * dimension,compute the x size which would make the rectangle appear
     * square, then check the assumption and do the reverse calculation
     * if necessary.
     */
    cySquare = cyHeight - 2;
    cxSquare = ( cyHeight * cxRes ) / cyRes;

    if (cxWidth < cxSquare)
    {
	cxSquare = cxWidth - 2;
	cySquare = (cxWidth * cyRes) / cxRes;
    }

    /*
     * Fill in the page rectangle and set the page viewport.
     */
    RECTL rclPage;  // clkdata.c war global!! aber warum?

    cxEdge = (cxWidth - cxSquare ) / 2;
    cyEdge = (cyHeight - cySquare ) / 2;
    rclPage.xLeft = cxEdge;
    rclPage.xRight = cxWidth - cxEdge;
    rclPage.yBottom = cyEdge;
    rclPage.yTop = cyHeight - cyEdge;


    f = GpiSetPageViewport (hps, &rclPage);
    f = GpiSetPageViewport (hpsBuffer, &rclPage);

    fBufferDirty = TRUE;
}

/****************************************************************\
 *  Name:ClkTimer()
 *
 *  Purpose: Handles window timer events
 *  Returns:
 *	    1 - if sucessful execution completed
 *	    0 - if error
\****************************************************************/
VOID ClkTimer (HWND hwnd)
{
	/* if we must move the hour and minute hands, redraw it all */
	if (dtNew.minutes != dt.minutes)
	{

	    ClkDrawFace(hpsBuffer);
	    ClkDrawDate(hpsBuffer, DM_REDRAW);
	    ClkDrawHand(hpsBuffer, HT_HOUR_SHADE, dtNew.hours);
	    ClkDrawHand(hpsBuffer, HT_MINUTE_SHADE, dtNew.minutes);
	    ClkDrawHand(hpsBuffer, HT_HOUR, dtNew.hours);
	    ClkDrawHand(hpsBuffer, HT_MINUTE, dtNew.minutes);

	    UpdateScreen (hps, NULL);

	}
}


// aus paint.c

VOID ClkPaint (HWND hwnd)
{
    RECTL rclUpdate;
    HPS hpsWnd;

    if (cp.usDispMode & DM_ANALOG)
    {

	WinBeginPaint (hwnd, hps, &rclUpdate);

	GpiCreateLogColorTable (hps, 0L, LCOLF_RGB, 0L, 0L, (PLONG)NULL);

	WinFillRect (hps, &rclUpdate, cp.clrBackground);

	if (fBufferDirty)
	{
	    DrawClock (hpsBuffer);
	    fBufferDirty = FALSE;
	}

	UpdateScreen (hps, &rclUpdate);

	/*
	 * Draw the second hand last, so xor will work.
	 */
	if (fShowSecondHand && (cp.usDispMode & DM_SECONDHAND))
	    ClkDrawHand(hps, HT_SECOND, dt.seconds);

	WinEndPaint (hps);

    }
    else
    { /*For now, if it is not Analog, it must be digital*/

	hpsWnd = WinBeginPaint (hwnd, NULLHANDLE, &rclUpdate);

	GpiCreateLogColorTable(hpsWnd, 0L, LCOLF_RGB, 0L, 0L, (PLONG) NULL);

	WinFillRect (hpsWnd, &rclUpdate, cp.clrBackground);

	memset (achOldTime, 0, sizeof(achOldTime));
	memset (achOldAmPm, '0', sizeof(achOldAmPm));
	memset (achOldDate, '0', sizeof(achOldDate));

	DrawDigitalTime (hwnd);

	WinEndPaint (hpsWnd);
    }
}


VOID DrawClock (HPS hpsDraw)
{
    RECTL rcl;

    WinQueryWindowRect (hwndClient, &rcl);
    WinFillRect (hpsBuffer, &rcl, cp.clrBackground);

    /* draw the face, the hour hand, and the minute hand */

    ClkDrawRing (hpsDraw);
    ClkDrawFace (hpsDraw);
    ClkDrawDate (hpsDraw, DM_REDRAW);
    ClkDrawHand (hpsDraw, HT_HOUR_SHADE, dt.hours);
    ClkDrawHand (hpsDraw, HT_MINUTE_SHADE, dt.minutes);
    ClkDrawHand (hpsDraw, HT_HOUR, dt.hours);
    ClkDrawHand (hpsDraw, HT_MINUTE, dt.minutes);

    /* draw the tick marks */
    if ((cp.usMajorTickPref == CLKTM_ALWAYS) ||
	    ((cp.usMajorTickPref == CLKTM_NOTICONIC) && ! fIconic))
	ClkDrawTicks (hpsDraw, CLK_MAJORTICKS);

    if ((cp.usMinorTickPref == CLKTM_ALWAYS) ||
	    ((cp.usMinorTickPref == CLKTM_NOTICONIC) && !fIconic))
	ClkDrawTicks (hpsDraw, CLK_MINORTICKS);
}

/****************************************************************\
 *
 *--------------------------------------------------------------
 *
 *  Name:UpdateScreen()
 *
 *  Purpose: Update the screen area.
 *
 *
 *
 *  Usage:
 *
 *  Method:
 *	    -
 *
 *	    -
 *	    -
 *
 *	    -
 *	    -
 *
 *  Returns:
 *
 *
\****************************************************************/
VOID UpdateScreen (HPS hps, RECTL *prclUpdate)
{
    POINTL aptl[3];

    if (prclUpdate != NULL)
    {
	aptl[0].x = prclUpdate->xLeft;
	aptl[0].y = prclUpdate->yBottom;
	aptl[1].x = prclUpdate->xRight;
	aptl[1].y = prclUpdate->yTop;
	aptl[2].x = prclUpdate->xLeft;
	aptl[2].y = prclUpdate->yBottom;
    }
    else
    {
	WinQueryWindowRect (hwndClient, (PRECTL)aptl);
	aptl[2].x =
	aptl[2].y = 0;
    }

    GpiBitBlt (hps, hpsBuffer, 3L, aptl, ROP_SRCCOPY, 0L);
}
