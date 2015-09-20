#include <iostream.h>

#include "game.h"

// global variables

BOARD *Board;
int errno = -1;

VOID ShowBoard()
{
	short Discover;
	cout << "\n";
	for( int i = Board->GetRows() - 2; i; i-- ){
		for( int j = 1; j < Board->GetColumns() - 1; j++){
			Discover = Board->GetDiscovered( i, j );
			cout << "   ";
			if( Discover < 0 )
				cout << ".";
			else cout << Discover;
		}
		cout << "\n";
	}
}

main()
{
	int Row, Col;
	Board = new BOARD( 6, 9 );

	BYTE ShipsNotFound = Board->GetShipNumber();
	while( ShipsNotFound ){
		ShowBoard();
		cout << "\nZeile : ";
		cin >> Row;
		cout << "\nSpalte : ";
		cin >> Col;
		if( Board->GetDiscovered( Row, Col ) != -1) continue;
		if( Board->Scan( Row, Col ) == Board->GetShipNumber() + 10)
			ShipsNotFound--;
	}
	cout << "You needed " << Board->MovesNeeded() << " moves to win!\n";
}
		
		
		
