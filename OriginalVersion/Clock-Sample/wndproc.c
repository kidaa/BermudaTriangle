ClkWndProc - Window Procedure (was passiert wann)
ClkCreate - bei der WM_CREATE msg
ClkDestroy - bei der WM_DESTROY msg
ClkSize - bei der WM_SIZE msg
ClkTimer - wie das Zeichnen behandelt wird

ShadeLight etc - um Licht und Schatten auszurechnen (fr Rahmen?)

/****************************************************************\
 *  Routine Name:ClkWndProc()
\****************************************************************/

MRESULT EXPENTRY ClkWndProc (HWND hwnd, ULONG usMsg, MPARAM mp1, MPARAM mp2)
{
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

}



/****************************************************************\
 *  Name:ClkCreate()
 *
 *  Purpose:Intialize a newly created client window
    called at WM_CREATE msg
 *  Returns:
 *          1 - if sucessful execution completed
 *          0 - if error
\****************************************************************/
VOID ClkCreate ( HWND hwnd )
{
    LONG cxScreen , cyScreen;  /* screen dimensions */
    LONG xLeft , yBottom ;      /* frame window location */
    ULONG cbBuf;
    LONG cyHeight;
    LONG cxWidth;

    hwndClient = hwnd;

    /* we are called before the global hwndFrame is valid */
    hwndFrame = WinQueryWindow ( hwnd , QW_PARENT) ;
    hwndTitleBar = WinWindowFromID ( hwndFrame , FID_TITLEBAR ) ;
    hwndSysMenu = WinWindowFromID ( hwndFrame , FID_SYSMENU ) ;
    hwndMinMax = WinWindowFromID ( hwndFrame , FID_MINMAX ) ;

    /* load our menus */
    hwndMenu = WinLoadMenu (hwndFrame, NULLHANDLE, IDR_MAIN);
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

    /* get the device resolutions so we can make the face appear circular */
    DevQueryCaps (hdc, (LONG)CAPS_VERTICAL_RESOLUTION,(LONG) 1L, &cyRes);
    DevQueryCaps (hdc, CAPS_HORIZONTAL_RESOLUTION, 1L, &cxRes);
    DevQueryCaps (hdc, CAPS_COLOR_PLANES, 1L, &cColorPlanes);
    DevQueryCaps (hdc, CAPS_COLOR_BITCOUNT, 1L, &cColorBitcount);

    cxScreen = WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN);
    cyScreen = WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN);

    /*
     * Calculate an initial window position and size.
     */
    xLeft = cxScreen / 8;
    yBottom = cyScreen / 2;
    cxWidth = cxScreen / 3;
    cyHeight = cyScreen / 2;
    WinSetWindowPos (hwndFrame, NULLHANDLE, xLeft, yBottom,
                       cxWidth, cyHeight,
                       SWP_SIZE | SWP_MOVE | SWP_ACTIVATE);

}


/****************************************************************\
 *  Name:ClkDestroy()
 *
 *  Purpose: Destroy clock face.
 *  Returns:VOID
 *
 *
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
 *          rectangle which: (a) fills the window rectangle in either
 *          the x or y dimension, (b) appears square, and (c) is centered
 *          in the window rectangle
 *  Returns:
 *
 *
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
     * Fill in the page rectangle and set the page viewport.
     */
    rclPage.xLeft = cxEdge;
    rclPage.xRight = cxWidth - cxEdge;
    rclPage.yBottom = cyEdge;
    rclPage.yTop = cyHeight - cyEdge;

    f = GpiSetPageViewport (hps, &rclPage);
    f = GpiSetPageViewport (hpsBuffer, &rclPage);

    GpiQueryCharBox(hpsBuffer, &sizef);
    GpiSetCharBox(hpsBuffer, &sizef);

    fBufferDirty = TRUE;

}

/****************************************************************\
 *  Name:ClkTimer()
 *
 *  Purpose: Handles window timer events
 *  Returns:
 *          1 - if sucessful execution completed
 *          0 - if error
\****************************************************************/
VOID ClkTimer (HWND hwnd)
{
    ClkDrawFace(hpsBuffer);
    ClkDrawDate(hpsBuffer, DM_REDRAW);
    ClkDrawHand(hpsBuffer, HT_HOUR_SHADE, dtNew.hours);
    ClkDrawHand(hpsBuffer, HT_MINUTE_SHADE, dtNew.minutes);
    ClkDrawHand(hpsBuffer, HT_HOUR, dtNew.hours);
    ClkDrawHand(hpsBuffer, HT_MINUTE, dtNew.minutes);
    
    UpdateScreen (hps, NULL);

}

//######################################################################

/****************************************************************\
 *  Name:ShadeLight()
 *
 *  Purpose: Find the shade and light color index values and
 *           install them in the colors   array of the element
 *  Returns:
 *
 *
\****************************************************************/
VOID ShadeLight(PLONG nplColors)
{
   typedef  union  _RGBLONG
   {
        RGB rgb;
        LONG lng;
   } RGBLONG ;
   RGBLONG  rgbSurface,rgbShade,rgbLight;

   rgbSurface.lng = rgbShade.lng = rgbLight.lng = 0L;

   #ifdef DISABLE
   rgbSurface.lng = GpiQueryRGBColor(hps,0L,);
   #endif
   rgbSurface.lng = nplColors[SURFACE];
   rgbShade.rgb.bBlue = ShadeRGBByte(rgbSurface.rgb.bBlue);
   rgbShade.rgb.bRed = ShadeRGBByte(rgbSurface.rgb.bRed);
   rgbShade.rgb.bGreen = ShadeRGBByte(rgbSurface.rgb.bGreen);
   rgbLight.rgb.bBlue = LightRGBByte(rgbSurface.rgb.bBlue);
   rgbLight.rgb.bRed = LightRGBByte(rgbSurface.rgb.bRed);
   rgbLight.rgb.bGreen = LightRGBByte(rgbSurface.rgb.bGreen);
   nplColors[SHADE] = rgbShade.lng;
   nplColors[LIGHT] = rgbLight.lng;
}


/****************************************************************\
 *  Name:ShadeRGBByte
\****************************************************************/

BYTE ShadeRGBByte(BYTE brgb)
{
  #define SHADER   ( (BYTE) 0x50)

  if (brgb > SHADER)
  {
     return (brgb - SHADER);
  }
  else
  {
     return (0);
  }

}
/****************************************************************\
 *  Name:LightRGBByte
\****************************************************************/
BYTE LightRGBByte(BYTE brgb)
{

  #define LIGHTER  ( (BYTE) 0x40)

  if (brgb < (0xff - LIGHTER) )
  {
     return (brgb + LIGHTER);
  }

  else return (0xff);

}



/*--------------------------------------------------------------*\
 *  End of file :wndproc.c
\*--------------------------------------------------------------*/



