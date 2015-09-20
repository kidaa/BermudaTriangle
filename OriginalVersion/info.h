/////////////////////////////////////////////////////////////
//	info.h
// Highscore Facilities

class INFO {
    ULONG LineStyle;

    struct HISCORE {
	char Name[30];
	char Score;
	HISCORE() { strcpy( Name, "<nobody>"); Score = 0; }
	HISCORE& operator=( const HISCORE& right )
	{
	    if( this != &right ){
		strcpy( Name, right.Name);
		Score = right.Score;
	    }
	    return *this;
	}
    };

    HISCORE ScoreList[10];
public:
    char ShipsNotFound;
    BOOL IsLineStyle( ULONG style ) { return LineStyle == style; }
    ULONG GetLineStyle() { return LineStyle; }
    VOID SetLineStyle( ULONG l ) { LineStyle = l; }

    char ReturnLastHigh() { return ScoreList[9].Score; }	
    VOID InsertHigh( char *n, char s );
    INFO() { LineStyle = IDC_DIAGONALS_AND_VERTICALS; }
    VOID SaveHigh(const ULONG seed);
    char *GetHighName( char i ) { return ScoreList[i].Name; }
    char GetHighScore( char i ) { return ScoreList[i].Score; }
    BOOL LoadHigh();
    VOID ResetHigh();
};

BOOL FileExists( PSZ FileName );


// extern globals

extern INFO InfoData;

