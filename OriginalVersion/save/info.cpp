///////////////////////////////////////////////////////
//	info.cc
// Highscore Facilities

#define INCL_WIN
#define INCL_GPI
#define INCL_DOSFILEMGR
#include <os2.h>
#include <fstream.h>		// for ifstream and ofstream
#include <string.h>		// for strcpy()
#include <ctype.h>		// for iscntrl()

#include <stdio.h>			// sprintf()
#include "defs.h"
#include "rand.h"
#include "info.h"


VOID INFO::InsertHigh( char *n, char s )
{
	INT i = 9, j;
	
	while( i >= 0 && (!ScoreList[i].Score || ScoreList[i].Score > s) ) i--;
	i++;
		// must increase i once again to insert after the current score
	for( j = 8; j >= i; j-- )	// push the greater scores one place further
		ScoreList[j+1] = ScoreList[j];
	strcpy( ScoreList[i].Name, n);
	ScoreList[i].Score = s;
}	


VOID INFO::ResetHigh()
{
	for( INT i = 0; i < 10; i++ ){
		strcpy( ScoreList[i].Name, "<nobody>" );
		ScoreList[i].Score = 0;
	}
	return;
}




VOID INFO::SaveHigh(const ULONG seed)
{
	ofstream out("scores.dat");
	CHAR RandomNr, NameLen;
	CHAR ListNr = 0;
	int NameNr, WasteNr;

	srand(seed);
	
	out << "Hey hacker, don't cheat!\n\n";
	RandomNr = CHAR(rand() % 4 + 2);	// between 2 and 5
	out.put(RandomNr);
	for(; ListNr < 10; ListNr++ ){
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			out.put( CHAR (rand() % 226 + 30) );     
		NameLen = strlen( (char*)ScoreList[ListNr].Name );
		out.put( NameLen );	
		for( NameNr = 0; NameNr < NameLen; NameNr++ ){
			for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
				out.put( (unsigned char) (rand() % 226 + 30) ); 
			out.put(  (unsigned char) (ScoreList[ListNr].Name[NameNr] + ListNr + 28) );
 		}
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			out.put( (unsigned char) (rand() % 226 + 30) );
		out.put( (unsigned char) (ScoreList[ListNr].Score + ListNr + 28) );
	}
}

///////////////////////////////////////////////////////
// LoadHigh loads the data from the file scores.dat
// and writes into the ScoreList array in INFO.
// returns TRUE if the file does not exist or if all
// important read operations succeeded and if all data was in its
// allowed range. If something went wrong, it returns FALSE.

BOOL INFO::LoadHigh()
{
	ifstream in("scores.dat");
	unsigned char RandomNr, NameLen, Waste, c;
	int ListNr = 0, NameNr, WasteNr;
	
	if( !in ) return TRUE;		// scores.dat does not yet exist
	for( WasteNr = 0; WasteNr < 26; WasteNr++)	
		 in.get( Waste );		// remove the "Hey hacker..." line
	if( !in.good() ) return FALSE;	
	 in.get( RandomNr );
	if( RandomNr < 2 || RandomNr > 5 ) return FALSE;
	for( ; ListNr < 10; ListNr++) {
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			in.get( Waste );
		if( !in.good() ) return FALSE;	
		in.get( NameLen );
		if( NameLen > 28 ) return FALSE;		// Name is of length 30 (incl. '\0')
		for( NameNr = 0; NameNr < NameLen; NameNr++ ){
			for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
				in.get( Waste );
			if( !in.good() ) return FALSE;	
			in.get(	c );
			if( iscntrl( c -= (ListNr + 28) ) ) return FALSE;	//nothing writeable
			ScoreList[ListNr].Name[NameNr] = c;
		}
		ScoreList[ListNr].Name[NameNr] = '\0';
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			in.get( Waste );
		if( !in.good() ) return FALSE;	
		in.get( c );
		ScoreList[ListNr].Score = CHAR(c - ListNr - 28);
	}
	return TRUE;
}	
	

// Globals

INFO InfoData;


//////// get info about the existence of a file
// returns TRUE if the file exists in the current directory, else
// returns FALSE
BOOL FileExists( PSZ FileName )
{

	HDIR hDir = HDIR_CREATE;
	ULONG ulFileCount = 1L;
	FILEFINDBUF3 FindBuf3;
	
	if( DosFindFirst( FileName, &hDir, 	FILE_NORMAL,
						&FindBuf3, sizeof( FILEFINDBUF3 ), &ulFileCount,
						FIL_STANDARD ) ){		// file not found
		CHAR msg[100];
		sprintf(msg, "Could not find the file %s in the current directory!\nSound will be disabled",
					FileName );
		WinMessageBox(	HWND_DESKTOP, HWND_DESKTOP, msg, "Error", 0,
							MB_ICONEXCLAMATION | MB_OK );
		return FALSE;
	} else
		return TRUE;
}
	

