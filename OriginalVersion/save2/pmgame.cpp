/////////////////////////////////////////////////////////////////
//		graphics functions: pmgame.cpp


#define INCL_WIN
#define INCL_GPI
#define INCL_DOSPROCESS
#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>     // for strcpy() from info.h

#include "game.h"
#include "defs.h"
#include "info.h"
#include "pmgame.h"
#include "bermuda.h"

#include "..\_Standard\wcapi.h"

// global variables:

HDC hdcGlob, hdcBufferGlob;
HPS hpsGlob, hpsBufferGlob;

HBITMAP hbmGlob;

LONG cColorPlanes, cColorBitCount;

// private functions:

// returns TRUE if a line has been painted 
BOOL GRAPHBOARD::OneLine( const HPS hps, POINTL from, POINTL to, INT time_on,
									INT time_off )
{
    if( from.x == to.x && from.y == to.y ) return FALSE;	// nothing to do

    POINTL fromLeftTop, fromRightBot, fromRightTop, toLeftTop, toRightBot,
	toRightTop;
    const CHAR CenterOffset = 1, PointDist = 2;		// 1 and 2

    // from and to really are fromLeftBot and toLeftBot
    from.x -= CenterOffset;
    fromLeftTop.x = from.x;
    from.y -= CenterOffset;
    fromRightBot.y = from.y;
    to.x -= CenterOffset;
    toLeftTop.x = to.x;
    to.y -= CenterOffset;
    toRightBot.y = to.y;	
    fromRightTop.y = fromLeftTop.y = from.y + PointDist;
    fromRightTop.x = fromRightBot.x = from.x + PointDist;
    toRightTop.y = toLeftTop.y = to.y + PointDist;
    toRightTop.x = toRightBot.x = to.x + PointDist;

    if( (from.x < to.x && from.y < to.y )
	|| ( from.x > to.x && from.y > to.y ) ){
				// if the diagonal is painted from lower left to upper right,
				// do an exchange from <-> fromLeftTop and to <-> toLeftTop.
				// this is necessary for nicer painting
				// so in this case from is fromLeftTop and fromLeftTop is fromLeftBot
	from.x += PointDist;
	fromRightBot.x -= PointDist;
	to.x += PointDist;
	toRightBot.x -= PointDist;
    }

    GpiBeginPath( hps, 1L );
    GpiMove( hps, &from );
    GpiLine( hps, &to );
    GpiLine( hps, &toRightBot );
    GpiLine( hps, &fromRightBot );
    GpiLine( hps, &from );
    GpiLine( hps, &to );
    GpiLine( hps, &toRightTop );
    GpiLine( hps, &fromRightTop );
    GpiLine( hps, &from );
    GpiLine( hps, &to );
    GpiLine( hps, &toLeftTop );
    GpiLine( hps, &fromLeftTop );
    GpiLine( hps, &from );
    GpiEndPath( hps );

    GpiSetColor( hps, CLR_BLUE );
    GpiSetMix( hps, FM_XOR );
    GpiFillPath ( hps, 1L, FPATH_WINDING );

    if( time_on ){	// if the line will be visible for a short while and then rem.
	DosSleep( time_on );
	GpiBeginPath( hps, 1L );
	GpiMove( hps, &from );
	GpiLine( hps, &to );
	GpiLine( hps, &toRightBot );
	GpiLine( hps, &fromRightBot );
	GpiLine( hps, &from );
	GpiLine( hps, &to );
	GpiLine( hps, &toRightTop );
	GpiLine( hps, &fromRightTop );
	GpiLine( hps, &from );
	GpiLine( hps, &to );
	GpiLine( hps, &toLeftTop );
	GpiLine( hps, &fromLeftTop );
	GpiLine( hps, &from );
	GpiEndPath( hps );
	GpiFillPath ( hps, 1L, FPATH_WINDING );	// remove it
	if( time_off ) DosSleep( time_off );	// wait after removing the line
    }
    return TRUE;		// line(s) has / have been painted
}

// public functions:
GRAPHBOARD::GRAPHBOARD( char Row, char Col )    // default values defined in pmgame.h
: BOARD( Row, Col )
{
    fDrag = FALSE;
    fShowLines = FALSE;
    FirstDraw = TRUE;
} 



VOID GRAPHBOARD::SetPMBoardValues( const short Width, const short Height )
{
    WinSizeX = Width;		
    WinSizeY = Height - 30;	// 30 pel needed to show the status line
    int dx = WinSizeX / (Columns - 1);		// distance in x- and y-direction
    int dy = WinSizeY / (Rows - 1);
    dist = dx < dy ? dx : dy;
    dx = WinSizeX - (Columns - 3) * dist;	// border at both sides
    dy = WinSizeY - (Rows - 3) * dist;
    LowerLeftPlace.x = dx / 2;		// offset from the window's edge
    LowerLeftPlace.y = dy / 2;
}

VOID GRAPHBOARD::DrawPMBoard( const HPS hps, const HPS hpsBuf )
{
    // paint the background with the "pool" bitmap
    const int BMPSize = 150;  // size of the "pool" bitmap
    HBITMAP hbm = GpiLoadBitmap( hps, (HMODULE)NULL, BMP_POOL, 
				 BMPSize, BMPSize );  // load the bitmap
    HBITMAP hbmBuf = GpiLoadBitmap( hpsBuf, (HMODULE)NULL, BMP_POOL, 
				 BMPSize, BMPSize );  // load the bitmap

    POINTL BMPCorner;
    RECTL BMPRect, *PBMPRect;
    BOOL YNull, XNull;
    for( BMPCorner.y = 0; BMPCorner.y < GBoard.GetWinHeight(); 
			  BMPCorner.y += BMPSize ){
	if( BMPCorner.y + BMPSize < GBoard.GetWinHeight() )
	    YNull = TRUE;  // no limitation for y direction
	else
	    YNull = FALSE;
	for( BMPCorner.x = 0; BMPCorner.x < GBoard.GetWinWidth();
			      BMPCorner.x += BMPSize ){
	    if( BMPCorner.x + BMPSize < GBoard.GetWinWidth() )
		XNull = TRUE;
	    else
		XNull = FALSE;
	    if( XNull && YNull ) PBMPRect = NULL; // bitmap fits
	    else {
		BMPRect.xLeft = BMPRect.yBottom = 0;
		BMPRect.xRight = BMPRect.yTop = BMPSize;
		if( !XNull ) 
		    BMPRect.xRight = GBoard.GetWinWidth() - BMPCorner.x;
		if( !YNull ) 
		    BMPRect.yTop = GBoard.GetWinHeight() - BMPCorner.y;
		PBMPRect = &BMPRect;
	    }

	    WinDrawBitmap( hps, hbm, PBMPRect, &BMPCorner, 0, 0, 
			   DBM_IMAGEATTRS | DBM_NORMAL );
	    WinDrawBitmap( hpsBuf, hbmBuf, PBMPRect, &BMPCorner, 0, 0, 
			   DBM_IMAGEATTRS | DBM_NORMAL );
	} // for BMPCorner.x
    } // for BMPCorner.y

   GpiDeleteBitmap( hbm );      /* releases the bitmap-handle   */
   GpiDeleteBitmap( hbmBuf );      /* releases the bitmap-handle   */

    POINTL StartHor = LowerLeftPlace;
    POINTL EndHor;
    POINTL StartVert = LowerLeftPlace;
    POINTL EndVert;
    int Value;
	
    EndHor.x = StartHor.x + (Columns - 3) * dist;	// shown cols = Columns - 2
    EndHor.y = StartHor.y;
    EndVert.x = StartVert.x;
    EndVert.y = StartVert.y + (Rows - 3) * dist;

    GpiSetColor( hps, CLR_BLACK );
    GpiSetColor( hpsBuf, CLR_BLACK );
    GpiSetLineType( hps, LINETYPE_SOLID );
    GpiSetLineType( hpsBuf, LINETYPE_SOLID );
    GpiSetLineWidth( hps, LINEWIDTH_NORMAL );
    GpiSetLineWidth( hpsBuf, LINEWIDTH_NORMAL );


    if( !InfoData.IsLineStyle(IDC_DIAGONALS) ){
	// first we paint the horizontal lines:
	for( ; StartHor.y <= EndVert.y; StartHor.y += dist, EndHor.y += dist ){
	    GpiMove( hps, &StartHor );
	    GpiMove( hpsBuf, &StartHor );
	    GpiLine( hps, &EndHor );
	    GpiLine( hpsBuf, &EndHor );
	}
	// then the vertical lines:
	for(; StartVert.x <= EndHor.x; StartVert.x += dist, EndVert.x += dist ){
	    GpiMove( hps, &StartVert );
	    GpiMove( hpsBuf, &StartVert );
	    GpiLine( hps, &EndVert );
	    GpiLine( hpsBuf, &EndVert );
	}
    }	// end if !InfoData.IsLineStyle( IDC_DIAGONALS )
    DosSleep( 1 );	// free time slice 

    if( !InfoData.IsLineStyle(IDC_VERTICALS) ){
	// there are still some diagonals left:
	// Start... draw from left top to right bottom
	GpiSetLineType( hps, LINETYPE_DOT );
	GpiSetLineType( hpsBuf, LINETYPE_DOT );
	StartVert.x = LowerLeftPlace.x;
	StartVert.y = LowerLeftPlace.y + dist;
	StartHor.x = LowerLeftPlace.x + dist;
	StartHor.y = LowerLeftPlace.y;

	// End... draw from right top to left bottom
	EndVert.x = LowerLeftPlace.x + ( Columns - 3) * dist;
	EndVert.y = LowerLeftPlace.y + dist;
	EndHor.x = LowerLeftPlace.x + ( Columns - 4) * dist;
	EndHor.y = LowerLeftPlace.y;
		
	int VertBreak = 0;
	int HorBreak = 0;
	while( StartVert.x != StartHor.x || StartVert.y != StartHor.y )
	    {
		GpiMove( hps, &StartHor );
		GpiMove( hpsBuf, &StartHor );
		GpiLine( hps, &StartVert );
		GpiLine( hpsBuf, &StartVert );
		GpiMove( hps, &EndHor );
		GpiMove( hpsBuf, &EndHor );
		GpiLine( hps, &EndVert );
		GpiLine( hpsBuf, &EndVert );
		if( VertBreak ){
		    StartVert.x += dist;
		    EndVert.x -= dist;
		} else {
		    StartVert.y += dist;
		    EndVert.y += dist;
		    if( StartVert.y == LowerLeftPlace.y + (Rows - 3)*dist)
			VertBreak = 1;
		}
		if( HorBreak ){
		    StartHor.y += dist;
		    EndHor.y += dist;
		} else {
		    StartHor.x += dist;
		    EndHor.x -= dist;
		    if( StartHor.x == LowerLeftPlace.x + (Columns - 3 ) * dist )
			HorBreak = 1;
		}
	    }	
    }	// end if !InfoData.IsLineStyle( IDC_VERTICALS )
    DosSleep( 1 );	// free time slice 
	
    // now we insert the values that have already been discovered
    for( char i = 1; i < Rows - 1; i++ ){
	for( char j = 1; j < Columns - 1; j++ ){
	    if( (Value = GetDiscovered( i, j) ) != -1 ){
		DrawPMPlace( hps, i, j, Value, FALSE );
		DrawPMPlace( hpsBuf, i, j, Value, FALSE );
	    } else {	
		ToggleMarked( i, j );
		DrawPMMark( hps, hpsBuf, i, j );  // cheat a little
	    }
	}			// for j
	DosSleep( 1 );		// free timeslice
    }				// for i
}

VOID GRAPHBOARD::DrawPMPlace( const HPS hps, const char Row,
			      const char Col, INT Value,
			      const BOOL fSwap )
{
    POINTL Point;
    ULONG Bitmap;
    HBITMAP hbm;

    Point.x = LowerLeftPlace.x + (Col - 1)*dist - dist/3;
    Point.y = LowerLeftPlace.y + (Row - 1)*dist - dist/3;
    if( !fSwap ){
	if( Value >= 50 ) Value -= 50;
	else if(Value <= 4 ) Value += 50;
    }
				// fSwap indicates if a number
				// will be swapped into a circle or vice versa
    if ( Value >= 50 ) {	// a number has already been found and will be
				// replaced by a circle now
	Bitmap = BMP_FCIRCLE;
	SetDiscovered( Row, Col, (SHORT)(Value - 50) );
    } else {
	switch( Value ){
	case 0: Bitmap = BMP_F0; break;
	case 1: Bitmap = BMP_F1; break;
	case 2: Bitmap = BMP_F2; break;
	case 3: Bitmap = BMP_F3; break;
	case 4: Bitmap = BMP_F4; break;
	default: Bitmap = BMP_SHIP;
	}
	if( Value != GetShipNumber() + 10 )
	    SetDiscovered( Row, Col, (SHORT)(Value + 50) );
    }
    hbm = GpiLoadBitmap( hps, (HMODULE)NULL, Bitmap, dist*2/3, dist*2/3);
//	if( hbm == NULLHANDLE ) DosBeep( 500, 150 );
//	if( !(WinDrawBitmap( hps, hbm, NULL, &Point, 0, 0, DBM_NORMAL ) ))
//		DosBeep( 1000, 150 );
    WinDrawBitmap(hps, hbm, NULL, &Point, 0, 0, DBM_NORMAL);
    GpiDeleteBitmap( hbm );		// releases the bitmap-handle
}

VOID GRAPHBOARD::DrawPMMark( const HPS hps, const HPS hpsBuf, 
			     const char Row, const char Col )
{
    POINTL Point;
    ULONG Bitmap;
    HBITMAP hbm;

    if( GetDiscovered( Row, Col ) != -1 ) return;
    if( GetMarked( Row, Col ) ) Bitmap = BMP_UNMARK;
    else Bitmap = BMP_MARK;
    Point.x = LowerLeftPlace.x + (Col - 1)*dist - dist/6;
    Point.y = LowerLeftPlace.y + (Row - 1)*dist - dist/6;
    hbm = GpiLoadBitmap( hps, (HMODULE)NULL, Bitmap, dist/3, dist/3);
    WinDrawBitmap(hps, hbm, NULL, &Point, 0, 0, DBM_NORMAL);
    GpiDeleteBitmap( hbm );		// releases the bitmap-handle
    // and now for the buffer:
    hbm = GpiLoadBitmap( hpsBuf, (HMODULE)NULL, Bitmap, dist/3, dist/3);
    WinDrawBitmap(hpsBuf, hbm, NULL, &Point, 0, 0, DBM_NORMAL);
    GpiDeleteBitmap( hbm );		// releases the bitmap-handle
    ToggleMarked( Row, Col );
}
	
	


char GRAPHBOARD::GetBoardCol( LONG ptrx )
{
    ptrx += dist / 2;
    if( (ptrx -= LowerLeftPlace.x) < 0 ) return 0; // ptrx = 0;
    if( (ptrx /= dist) > (Columns - 3) ) return 0; // ptrx = Columns - 3;
    return (CHAR)(ptrx + 1);
}
	
char GRAPHBOARD::GetBoardRow(	LONG ptry )
{
    ptry += dist / 2;
    if( (ptry -= LowerLeftPlace.y) < 0 ) return 0; //ptry = 0;
    if( (ptry /= dist) > (Rows - 3) ) return 0; //ptry = Rows - 3;
    return (CHAR)(ptry + 1);
}


VOID GRAPHBOARD::DrawDragLine( const HPS hps, const char BeginRow,
								 const char BeginCol, POINTL ptl )
{
    int NewLengthX = 0, NewLengthY = 0;
    POINTL NewDrawPoint;		// where we want to paint to
    POINTL BeginPoint;	// where we begin to paint

    BeginPoint.x = NewDrawPoint.x = LowerLeftPlace.x + (BeginCol -1)*dist;
    BeginPoint.y = NewDrawPoint.y = LowerLeftPlace.y + (BeginRow - 1)*dist;
    if( FirstDraw ){	// DrawPoint is the point where we drew to the last time
	DrawPoint.x = BeginPoint.x ;
	DrawPoint.y = BeginPoint.y;
	FirstDraw = FALSE;
    }

    int XOffset = ptl.x - BeginPoint.x;
    int YOffset = ptl.y - BeginPoint.y;
    double help = double(XOffset);
    if( help == 0 ) help = 0.001;
    double Quot = YOffset / help;
    if( Quot < 0 ) Quot = -Quot;

    if( Quot < 0.5 )
	NewLengthX = XOffset;
    else 
	if( Quot < 1 )	{
	    NewLengthX = NewLengthY = YOffset;
	    if( (YOffset > 0 && XOffset < 0) || (YOffset < 0 && XOffset > 0) )
		NewLengthX = - NewLengthX;
	}
	else
	    if( Quot < 2 ){
		NewLengthX = NewLengthY = XOffset;
		if( (YOffset > 0 && XOffset < 0) || (YOffset < 0 && XOffset > 0) )
		    NewLengthY = - NewLengthY;
	    }
	    else
		NewLengthY = YOffset;

    if( NewLengthX >= 0 )
	NewLengthX += dist/2;
    else	
	NewLengthX -= dist/2;

    if( NewLengthY >= 0 )
	NewLengthY += dist/2;
    else	
	NewLengthY -= dist/2;

    NewLengthX /= dist;
    NewLengthY /= dist;	

    NewDrawPoint.x += NewLengthX * dist;
    NewDrawPoint.y += NewLengthY * dist;
    // determine where to draw to

    if( NewDrawPoint.x < LowerLeftPlace.x
	|| NewDrawPoint.y < LowerLeftPlace.y 
	|| NewDrawPoint.x > LowerLeftPlace.x + (Columns - 3) * dist 
	|| NewDrawPoint.y > LowerLeftPlace.y + (Rows - 3) * dist )
	// NewDrawPoint is out of range
	{
	    NewDrawPoint.x = NewDrawPoint.y = 0;
	    //	 	NewDrawPoint = BeginPoint;
	}

    if( DrawPoint.y == NewDrawPoint.y && NewDrawPoint.x == DrawPoint.x  ) return;
    // nothing has changed -> no drawing necessary

    if( DrawPoint.x && DrawPoint.y )
	OneLine( hps, BeginPoint, DrawPoint );	// remove old line
    if( NewDrawPoint.x && NewDrawPoint.y )	
	OneLine( hps, BeginPoint, NewDrawPoint );	// draw new line
    DrawPoint = NewDrawPoint;	// adjust the DrawPoint for the next drawing
}


				
VOID GRAPHBOARD::MarkDragLine( const HPS hps, const HPS hpsBuf,
			       const char BeginRow, const char BeginCol )
{
    char EndRow, EndCol;
    INT dx, dy;
    POINTL BeginPoint;

    if( !DrawPoint.x && !DrawPoint.y ) return;
    // pointer out of range -> do nothing
		
    EndRow = GetBoardRow( DrawPoint.y );
    EndCol = GetBoardCol( DrawPoint.x );

    BeginPoint.x = LowerLeftPlace.x + (BeginCol -1)*dist;
    BeginPoint.y = LowerLeftPlace.y + (BeginRow - 1)*dist;

    dx = EndCol - BeginCol;
    if ( dx )
	if ( dx > 0 ) dx = 1;
	else dx = -1;

    dy = EndRow - BeginRow;
    if( dy )
	if( dy > 0 ) dy = 1;
	else dy = -1;

    OneLine( hps, BeginPoint, DrawPoint );
	
    while( EndRow != BeginRow || EndCol != BeginCol ){
	if( !GetMarked( EndRow, EndCol) )
	    DrawPMMark( hps, hpsBuf, EndRow, EndCol );
	EndRow -= dy;
	EndCol -= dx;
    }
    if( !GetMarked( EndRow, EndCol) )
	DrawPMMark( hps, hpsBuf, EndRow, EndCol );
}

/////////////////////////////////////////////////////////////////
// this function is called at any mousemove. If the pointer has
// moved from one place to another, the function removes the square
// at the old position, adjusts DrawPoint and draws a new square.
// If the help lines are visible, this function also turns them off.
	
VOID GRAPHBOARD::ShowPointerPos( HPS hps, LONG ptx, LONG pty )
{
    POINTL Point;

    Point.x = LowerLeftPlace.x + (GetBoardCol( ptx ) - 1) * dist;
    Point.y = LowerLeftPlace.y + (GetBoardRow( pty ) - 1) * dist;
    if( Point.x == DrawPoint.x && Point.y == DrawPoint.y ) return;
    // nothing to do

    if( GetfShowLines() ){	// if the help lines are visible, turn them off
	SetfShowLines( FALSE );
	DisplayLines( hps );
    }

    GpiSetMix( hps, FM_XOR );
    GpiSetColor( hps, CLR_DARKGRAY );
    if( DrawPoint.x >= LowerLeftPlace.x && DrawPoint.y >= LowerLeftPlace.y ) {
	// there exists a visible square that must be removed
	DrawPoint.x -= dist / 3;
	DrawPoint.y -= dist / 3;
	GpiMove( hps, &DrawPoint );
	DrawPoint.x += 2 * dist / 3;
	DrawPoint.y += 2 * dist / 3;
	GpiBox( hps, DRO_OUTLINEFILL, &DrawPoint, 0, 0 );

    }
    DrawPoint.x = Point.x;	// DrawPoint is set to the new point
    DrawPoint.y = Point.y;
    if( Point.x >= LowerLeftPlace.x && Point.y >= LowerLeftPlace.y ){
	// neither GetBoardCol nor GetBoardRow returned 0 => point is visible
	Point.x -= dist / 3;
	Point.y -= dist / 3;
	GpiMove( hps, &Point );
	Point.x += 2 * dist / 3;
	Point.y += 2 * dist / 3;
	GpiBox( hps, DRO_OUTLINEFILL, &Point, 0, 0 );
    }
    GpiSetMix( hps, FM_OVERPAINT );
}
	

VOID GRAPHBOARD::DisplayLines( const HPS hps )
{
    POINTL P1, P2;
    INT BorderDist;
    INT Row, Col;


    Row = GetBoardRow( DrawPoint.y );
    Col = GetBoardCol( DrawPoint.x );
    if( !Row || !Col ) return; 	// cursor is outside the gameboard

    // left to right line:
    P1.x = LowerLeftPlace.x;			// left edge
    P2.x = LowerLeftPlace.x + (Columns - 3)*dist;	// right edge
    P1.y = P2.y = LowerLeftPlace.y + (Row - 1)*dist;	// same height as DrawPoint
    OneLine( hps, P1, P2 );

    // bottom to top line:
    P1.x = P2.x = LowerLeftPlace.x + (Col - 1)*dist;
    P1.y = LowerLeftPlace.y;
    P2.y = LowerLeftPlace.y + (Rows - 3)*dist;
    OneLine( hps, P1, P2 );

    // left bottom to right top line:
    BorderDist = Row < Col ? Row : Col;
    P1.x = LowerLeftPlace.x + ( Col - BorderDist )*dist;
    P1.y = LowerLeftPlace.y + ( Row - BorderDist )*dist;
    BorderDist = ( Rows - 3 - Row ) < (Columns - 3 - Col ) ?
	( Rows - 3 - Row ) : (Columns - 3 - Col );
    P2.x = LowerLeftPlace.x + ( Col + BorderDist )*dist;
    P2.y = LowerLeftPlace.y + ( Row + BorderDist )*dist;
    OneLine( hps, P1, P2 );

    // right bottom to left top line:
    BorderDist = (Row )  < (Columns - 1 - Col ) ?
	(Row ) : (Columns - 1 - Col );
    P1.x = LowerLeftPlace.x + ( Col + BorderDist - 2 )*dist;
    P1.y = LowerLeftPlace.y + ( Row - BorderDist )*dist;
    BorderDist = ( Rows - 1 - Row ) < (Col )  ?
	( Rows - 1 - Row ) : (Col );
    P2.x = LowerLeftPlace.x + ( Col - BorderDist )*dist;
    P2.y = LowerLeftPlace.y + ( Row + BorderDist - 2 )*dist;
	
    OneLine( hps, P1, P2 );
}

VOID GRAPHBOARD::DrawScanLines( const HPS hps, const INT Row, const INT Col )
{
    POINTL left, right, top, bot, lefttop, leftbot, righttop, rightbot, center;
    INT BorderDist;

    center.x = LowerLeftPlace.x + ( Col - 1) * dist;
    center.y = LowerLeftPlace.y + (Row - 1) * dist;

    // left to right line:
    left.x = LowerLeftPlace.x;			// left edge
    right.x = LowerLeftPlace.x + (Columns - 3)*dist;	// right edge
    left.y = right.y = LowerLeftPlace.y + (Row - 1)*dist;	// same height as DrawPoint
    // bottom to top line:
    bot.x = top.x = LowerLeftPlace.x + (Col - 1)*dist;
    bot.y = LowerLeftPlace.y;
    top.y = LowerLeftPlace.y + (Rows - 3)*dist;
    // left bottom to right top line:
    BorderDist = Row < Col ? Row : Col;
    leftbot.x = LowerLeftPlace.x + ( Col - BorderDist )*dist;
    leftbot.y = LowerLeftPlace.y + ( Row - BorderDist )*dist;
    BorderDist = ( Rows - 3 - Row ) < (Columns - 3 - Col ) ?
	( Rows - 3 - Row ) : (Columns - 3 - Col );
    righttop.x = LowerLeftPlace.x + ( Col + BorderDist )*dist;
    righttop.y = LowerLeftPlace.y + ( Row + BorderDist )*dist;
    // right bottom to left top line:
    BorderDist = (Row )  < (Columns - 1 - Col ) ?
	(Row ) : (Columns - 1 - Col );
    rightbot.x = LowerLeftPlace.x + ( Col + BorderDist - 2 )*dist;
    rightbot.y = LowerLeftPlace.y + ( Row - BorderDist )*dist;
    BorderDist = ( Rows - 1 - Row ) < (Col )  ?
	( Rows - 1 - Row ) : (Col );
    lefttop.x = LowerLeftPlace.x + ( Col - BorderDist )*dist;
    lefttop.y = LowerLeftPlace.y + ( Row + BorderDist - 2 )*dist;
	
    OneLine( hps, center, right );
    DosSleep( 50 );
    OneLine( hps, center, right );
    DosSleep( 50 );
    OneLine( hps, center, righttop );
    DosSleep( 50 );
    OneLine( hps, center, righttop );
    DosSleep( 50 );
    OneLine( hps, center, top );
    DosSleep( 50 );
    OneLine( hps, center, top );
    DosSleep( 50 );
    OneLine( hps, center, lefttop );
    DosSleep( 50 );
    OneLine( hps, center, lefttop );
    DosSleep( 50 );
    OneLine( hps, center, left );
    DosSleep( 50 );
    OneLine( hps, center, left );
    DosSleep( 50 );
    OneLine( hps, center, leftbot );
    DosSleep( 50 );
    OneLine( hps, center, leftbot );
    DosSleep( 50 );
    OneLine( hps, center, bot );
    DosSleep( 50 );
    OneLine( hps, center, bot );
    DosSleep( 50 );
    OneLine( hps, center, rightbot );
    DosSleep( 50 );
    OneLine( hps, center, rightbot );

}

VOID ShowStatusLine( const HPS hps, const char Moves,
							const char Ships, const LONG Width,
							const LONG Height )
{
    RECTL Rect = { 0, Height, Width, Height + 30 };
    POINTL Point = { Width, Height };
    CHAR text[256];
 
    WinFillRect( hps, &Rect, CLR_WHITE ); //SYSCLR_WINDOW );	// clear background
    GpiSetColor( hps, CLR_BLACK );
//    GpiSetLineType( hps, LINETYPE_SHORTDASH );
//    GpiSetLineWidth( hps, LINEWIDTH_NORMAL );
    GpiSetMix( hps, FM_OVERPAINT );
//    GpiMove( hps, &Point );
    Point.x = 0;
//    GpiLine( hps, &Point );
    Point.x += 10;
    Point.y += 10;
    GpiMove( hps, &Point );
    if( Ships )
	sprintf( text, "%d %s made     %d %s still lost", Moves,
		 Moves == 1 ? "move" : "moves", Ships, Ships == 1 ? "ship" : "ships" );
    else
	sprintf( text, "%d %s made     All ships found", Moves,
		 Moves == 1 ? "move" : "moves" );

    GpiCharString( hps, strlen(text), text );
    DosSleep( 1 );	// free time slice 
} 
	

VOID GRAPHBOARD::CalcScanLines( char Row, char Col, POINTL &left, 
				POINTL &right,
				POINTL &top, POINTL &bot, POINTL &lefttop,
				POINTL &leftbot, POINTL &righttop,
				POINTL &rightbot, POINTL &center )
{				
    INT BorderDist;

    center.x = LowerLeftPlace.x + ( Col - 1) * dist;
    center.y = LowerLeftPlace.y + (Row - 1) * dist;

    // left to right line:
    left.x = LowerLeftPlace.x;			// left edge
    right.x = LowerLeftPlace.x + (Columns - 3)*dist;	// right edge
    left.y = right.y = LowerLeftPlace.y + (Row - 1)*dist;	// same height as DrawPoint
    // bottom to top line:
    bot.x = top.x = LowerLeftPlace.x + (Col - 1)*dist;
    bot.y = LowerLeftPlace.y;
    top.y = LowerLeftPlace.y + (Rows - 3)*dist;
    // left bottom to right top line:
    BorderDist = Row < Col ? Row : Col;
    leftbot.x = LowerLeftPlace.x + ( Col - BorderDist )*dist;
    leftbot.y = LowerLeftPlace.y + ( Row - BorderDist )*dist;
    BorderDist = ( Rows - 3 - Row ) < (Columns - 3 - Col ) ?
	( Rows - 3 - Row ) : (Columns - 3 - Col );
    righttop.x = LowerLeftPlace.x + ( Col + BorderDist )*dist;
    righttop.y = LowerLeftPlace.y + ( Row + BorderDist )*dist;
    // right bottom to left top line:
    BorderDist = (Row )  < (Columns - 1 - Col ) ?
	(Row ) : (Columns - 1 - Col );
    rightbot.x = LowerLeftPlace.x + ( Col + BorderDist - 2 )*dist;
    rightbot.y = LowerLeftPlace.y + ( Row - BorderDist )*dist;
    BorderDist = ( Rows - 1 - Row ) < (Col )  ?
	( Rows - 1 - Row ) : (Col );
    lefttop.x = LowerLeftPlace.x + ( Col - BorderDist )*dist;
    lefttop.y = LowerLeftPlace.y + ( Row + BorderDist - 2 )*dist;
}

// initializes the window PS and the off-screen buffer
VOID InitPS( const HWND hwnd )
{
    SIZEL g = { 0, 0 };

    /* we are called before the global hwndFrame is valid */
    hwndFrame = WinQueryWindow ( hwnd , QW_PARENT) ;
    wcprintf("Muell: %x", WinGetLastError( hab ) );
    hdcGlob = WinOpenWindowDC( hwnd );
    hpsGlob = GpiCreatePS( hab, hdcGlob, &g, PU_ARBITRARY | GPIT_MICRO |
			   GPIA_ASSOC );
    /*
     * Create our off-screen 'buffer'.
     */
    // hdcGlob als letzten Parameter ist neu:
    hdcBufferGlob = DevOpenDC( (HAB)0L, OD_MEMORY, "*", 0L, NULL, hdcGlob );
    hpsBufferGlob = GpiCreatePS( hab, hdcBufferGlob, &g, PU_ARBITRARY |
				 GPIT_MICRO | GPIA_ASSOC );

    /* get the device resolutions  */
    DevQueryCaps( hdcGlob, CAPS_COLOR_PLANES, 1L, &cColorPlanes );
    DevQueryCaps( hdcGlob, CAPS_COLOR_BITCOUNT, 1L, &cColorBitCount );
}

// updates the buffer
VOID DrawPSBuffer()
{
    RECTL rectl;
    rectl.xLeft = rectl.yBottom = 0;
    rectl.xRight = GBoard.GetWinWidth();
    rectl.yTop = GBoard.GetWinHeight();
wcprintf("Das Board denkt: Breite: %d H”he: %d", rectl.xRight, rectl.yTop );

    WinFillRect( hpsGlob, &rectl, CLR_WHITE );
    WinFillRect( hpsBufferGlob, &rectl, CLR_WHITE );
    wcprintf("Jetzt wird der Hintergrund gezeichnet");      

    ShowStatusLine( hpsGlob, GBoard.MovesNeeded(), 
		    InfoData.ShipsNotFound,
		    GBoard.GetWinWidth(), GBoard.GetWinHeight() );
    ShowStatusLine( hpsBufferGlob, GBoard.MovesNeeded(), 
		    InfoData.ShipsNotFound,
		    GBoard.GetWinWidth(), GBoard.GetWinHeight() );

    GBoard.DrawPMBoard( hpsGlob, hpsBufferGlob );
}


/****************************************************************\
 *  Name:UpdateScreen()
 *
 *  Purpose: Update the screen area.
\****************************************************************/
VOID UpdateScreen (RECTL *prclUpdate)
{
    POINTL aptl[3];

    if ( prclUpdate != NULL)
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
        WinQueryWindowRect (hwndMain, (PRECTL)aptl);
        aptl[2].x = aptl[2].y = 0;
    }

    // als letzten Parameter mal 0L einsetzen? (war so in clock)
    if( GpiBitBlt( hpsGlob, hpsBufferGlob, 3L, aptl, ROP_SRCCOPY,
		   BBO_IGNORE ) == GPI_ERROR )
	wcprintf(" BitBlt error!");

}



// manages changes in size
VOID WndResize( const HWND hwnd )
{
    wcprintf("WndResize");
    RECTL rWindow;
    LONG cyHeight, cxWidth;
    BITMAPINFOHEADER bmp;

    if( (hbmGlob = GpiSetBitmap( hpsBufferGlob, NULLHANDLE )) != NULLHANDLE ){
	wcprintf("wndresize: there was an old bitmap");
	GpiDeleteBitmap( hbmGlob );        // delete old stuff
    } else 
	wcprintf("wndresize: there was no old bitmap");

    // build the bitmap:
    WinQueryWindowRect( hwnd, &rWindow );
    cxWidth = rWindow.xRight - rWindow.xLeft;// - 2;
    cyHeight = rWindow.yTop - rWindow.yBottom; // - 2;
    wcprintf("wndResize: Breite: %d, H”he: %d", rWindow.xRight, rWindow.yTop );

    memset(&bmp, 0, sizeof(BITMAPINFOHEADER));
    bmp.cbFix = sizeof(BITMAPINFOHEADER);
    bmp.cx = (SHORT)cxWidth;
    bmp.cy = (SHORT)cyHeight;
    bmp.cPlanes = (SHORT)cColorPlanes;
    bmp.cBitCount = (SHORT)cColorBitCount;
    hbmGlob = GpiCreateBitmap(hpsBufferGlob, (PBITMAPINFOHEADER2)&bmp, 
			      0L, NULL, NULL); 
    if( hbmGlob == GPI_ERROR )
	DosBeep( 100, 500);
    GpiSetBitmap( hpsBufferGlob, hbmGlob);

    // calculate the size

//    if( !GpiSetPageViewport(hpsGlob, &rWindow) )
//	wcprintf("error 1");
//    if( !GpiSetPageViewport(hpsBufferGlob, &rWindow) )
//	wcprintf("error 2");
}


VOID DestroyPS()
{
    HBITMAP hbm;
    
    hbm = GpiSetBitmap (hpsBufferGlob, NULLHANDLE);
    
    if (hbm != NULLHANDLE)
	GpiDeleteBitmap (hbm);
    
    GpiDestroyPS (hpsBufferGlob);
    DevCloseDC (hdcBufferGlob);
}









