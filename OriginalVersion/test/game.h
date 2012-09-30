////////////////////////////////////////////////////////////////////
//	game.h

// typedef unsigned char char;
// typedef void VOID;
#define FALSE 0
#define TRUE 1
enum CONT { EMPTY=0, SHIP = 5, BORDER = 100 };
enum DIRECTION { LEFT, LEFT_TOP, TOP, TOP_RIGHT, RIGHT, RIGHT_DOWN,
					  DOWN, DOWN_LEFT };



class BOARD {
	private:
		struct PLACE {
			CONT Contents;
			short Discovered;
			char Marked;
		};
		PLACE **Place;
		char ShipNumber;
	protected:
		char Rows;
		char Columns;		// REAL number of rows / columns !!
		char MoveCount;
	public:
		BOARD( char Rw = 9, char Col = 20 );
		VOID NewGame();		// creates an empty gameboard of the same size
		struct HIDE {		// for testing purposes only
			char Row;
			char Col;
		};
		HIDE *Hide;
		char GetShipNumber() { return ShipNumber; }
		char GetRows() { return Rows; }
		char GetColumns() { return Columns; }
		CONT GetContents( char Row, char Col )
			{ return Place[Row][Col].Contents;	}
		VOID SetContents( char Row, char Col, CONT NewCont )
			{	Place[Row][Col].Contents = NewCont;	}
		short GetDiscovered( char Row, char Col )
			{	return Place[Row][Col].Discovered;	}
		VOID SetDiscovered( char Row, char Col, short Count )
			{	Place[Row][Col].Discovered = Count;	}
		char GetMarked( char Row, char Col ){ return Place[Row][Col].Marked; }    
		VOID ToggleMarked( char Row, char Col ){ Place[Row][Col].Marked ^= TRUE; }             
		int Scan( char Row, char Col );
		char MovesNeeded(){ return MoveCount; }
};

// global variables

	
		
		
