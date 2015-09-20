//////////////////////////////////////////////////////////////////////
//		pmgame.h

class GRAPHBOARD : public BOARD {
    int dist;		// distance between two places in pels
    POINTL LowerLeftPlace;	// Position of Place[1][1] in the window
    int WinSizeX;		// size of the window in pels
    int WinSizeY;
    BOOL fDrag;				// for the drag painting:
    DIRECTION DrawDir;		
    POINTL DrawPoint;
    BOOL FirstDraw;
    BOOL fShowLines;
public:
    BOOL OneLine( const HPS hps, POINTL from, POINTL to, INT time_on = 0,
		  INT time_off = 0 );
    int GetWinWidth() { return WinSizeX; }
    int GetWinHeight() { return WinSizeY; }
    GRAPHBOARD( char Row = 9, char Col = 20 );
    VOID SetPMBoardValues( const short Width, const short Height );
    VOID DrawPMBoard( const HPS hps );
    VOID DrawPMPlace( const HPS hps, const char Row,
		      const char Col, INT Value, const BOOL fSwap );
    VOID DrawPMMark( const HPS hps, const char Row, const char Col );
    char GetBoardCol( LONG ptrx );
    char GetBoardRow(	LONG ptry );
    VOID SetfDrag( BOOL Status ){ fDrag = Status; }
    BOOL GetfDrag() { return fDrag; }
    VOID SetfShowLines( BOOL Status ){ fShowLines = Status; }
    BOOL GetfShowLines() { return fShowLines; }
    VOID DrawDragLine( const HPS hps, const char BeginRow,
		       const char BeginCol, POINTL ptl );
    VOID MarkDragLine( const HPS hps,
		       const char BeginRow, const char BeginCol );
    VOID ResetFirstDraw() { FirstDraw = TRUE; }						 
    VOID DisplayLines( const HPS hps );
    VOID DrawScanLines( const HPS hps, const INT Row, const INT Col );
    VOID ClearDrawPoint() { DrawPoint.x = 0; DrawPoint.y = 0; }
    VOID ShowPointerPos( HPS hps, LONG ptx, LONG pty );
    VOID CalcScanLines( char Row, char Col, POINTL &left, POINTL &right,
			POINTL &top, POINTL &bot, POINTL &lefttop,
			POINTL &leftbot, POINTL &righttop,
			POINTL &rightbot, POINTL &center );
};

VOID ShowStatusLine( const HPS hps, const char Moves,
		     const char Ships, const LONG Width,
		     const LONG Height );


VOID InitPS( const HWND hwnd );
VOID WndResize( const HWND hwnd );

extern HPS hpsGlob, hpsBufferGlob;
