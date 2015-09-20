/****************************************************************\
 *  Name:UpdateScreen()
 *
 *  Purpose: Update the screen area.
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
/****************************************************************\
 *  Name:ClkPaint
 *
 *  Purpose: Paint the clock client window.
 *  Returns:  VOID
 *
 *
\****************************************************************/
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
	    // zeichne Uhr in Buffer:
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
}









