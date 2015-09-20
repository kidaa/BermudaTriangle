////////////////////////////////////////////////////////////
//	game.cc


//#include <stdlib.h>
#include <stdio.h>
#include <time.h>		// needed for the random seed

typedef void VOID;
#include "game.h"
#include "rand.h"

unsigned next_rand = 10;

BOARD::BOARD(char Rw, char Col ) // default arguments defined in game.h
{
    Rows = (char)(Rw + 2);
    Columns = (char)(Col + 2);

    ShipNumber = (char)(Rw * Col / 45);
    Hide = new HIDE[ShipNumber];
    Place = new PLACE*[Rows];
    for( int i = 0; i < Rows; i++ )
	Place[i] = new PLACE[Columns];
		

    for( i = 0; i < Columns; i++ )
	Place[0][i].Contents= BORDER;
    for( i = 0; i < Columns; i++ )
	Place[Rows - 1][i].Contents= BORDER;
    for( i = 1; i < Rows - 1; i++ )
	Place[i][0].Contents= BORDER;
    for( i = 1; i < Rows - 1; i++ )
	Place[i][Columns - 1].Contents = BORDER;

    NewGame();	
}		

VOID BOARD::NewGame()
{
    int i;
    char Nr = ShipNumber;
    char z, s;
    
    srand(time(NULL));	
    MoveCount = 0;
    for( i = 1; i < Rows - 1; i++ )
	for( int j = 1; j < Columns - 1; j++ ){
	    Place[i][j].Contents = EMPTY;
	    Place[i][j].Discovered = -1;
	    Place[i][j].Marked = FALSE;
	}
	
    while( Nr ) {
	z = (char)(rand() % (Rows - 2) + 1);
	s = (char)(rand() % (Columns - 2) + 1);
	if( Place[z][s].Contents == EMPTY ) {
	    Place[z][s].Contents = SHIP;
	    Nr--;
	    Hide[Nr].Row = z;		// for testing only
	    Hide[Nr].Col = s;
	}
    }
}


int BOARD::Scan( char Row, char Col )
{
    char i, j, Count = 0;
    CONT PlaceCont;
	
    if( !Row || !Col || Row >= (Rows - 1) || Col >= (Columns - 1))
	return -1;
    if( GetContents( Row, Col ) == SHIP ){
	SetDiscovered( Row, Col, (char)(ShipNumber + 10) );
	return ShipNumber + 10;
	// this number will not be reached otherwise
    }
    // left
    for( i = Row, j = (char)(Col-1); !(PlaceCont = GetContents( i, j )); j-- );
    if( PlaceCont == SHIP ) Count++;
    // left-top
    for( i = (char)(Row+1), j = (char)(Col-1); !(PlaceCont = GetContents( i, j )); i++, j-- );
    if( PlaceCont == SHIP ) Count++;
    // top
    for( i = (char)(Row+1), j = Col; !(PlaceCont = GetContents( i, j )); i++ );
    if( PlaceCont == SHIP ) Count++;
    // top-right
    for( i = (char)(Row+1), j = (char)(Col+1); !(PlaceCont = GetContents( i, j )); i++, j++ );
    if( PlaceCont == SHIP ) Count++;
    // right
    for( i = Row, j = (char)(Col+1); !(PlaceCont = GetContents( i, j )); j++ );
    if( PlaceCont == SHIP ) Count++;
    // right-bottom
    for( i = (char)(Row-1), j = (char)(Col+1); !(PlaceCont = GetContents( i, j )); i--, j++ );
    if( PlaceCont == SHIP ) Count++;
    // bottom
    for( i = Row-1, j = Col; !(PlaceCont = GetContents( i, j )); i-- );
    if( PlaceCont == SHIP ) Count++;
    // bottom-left
    for( i = Row-1, j = Col-1; !(PlaceCont = GetContents( i, j )); i--, j-- );
    if( PlaceCont == SHIP ) Count++;
    SetDiscovered( Row, Col, Count );
    MoveCount++;		// if a ship is found, the move is not counted
    return Count;
}
		

		
