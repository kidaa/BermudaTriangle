#include <fstream.h>
#include <iostream.h>
#include <stdlib.h>
#include <string.h>

class INFO {
public:
	unsigned char Name[30];
	int Score;
	INFO() { 
	//	Name = new char[30];
		strcpy((char*)Name, " \0"); 
		Score = 0; }
};

INFO ScoreList[10];				



void SaveHigh()
{
	ofstream out("Festplatte:scores.dat");
	unsigned char RandomNr, NameLen;
	int ListNr = 0, NameNr, WasteNr;
	
	out << "Hey hacker, don't cheat!\n\n";
	RandomNr = rand() % 3 + 2;	// between 2 and 5
	out.put(RandomNr);
	for(; ListNr < 10; ListNr++ ){
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			out.put(rand() % 256 );
		NameLen = strlen( (char*)ScoreList[ListNr].Name );
		out.put( NameLen );	
		for( NameNr = 0; NameNr < NameLen; NameNr++ ){
			for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
				out.put(rand() % 256 );
			out.put( ScoreList[ListNr].Name[NameNr] + ListNr + 11 );
 		}
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			out.put(rand() % 256 );
		out.put(ScoreList[ListNr].Score + ListNr + 11 );
	}	 		
}

void GetHigh()
{
	ifstream in("Festplatte:scores.dat");
	unsigned char RandomNr, NameLen, Waste, c;
	int ListNr = 0, NameNr, WasteNr;
	
	if( !in ) return;		// scores.dat does not yet exist
	for( WasteNr = 0; WasteNr < 26; WasteNr++)
		in.get( Waste );		// remove the "Hey hacker..." line
	in.get( RandomNr );
	for( ; ListNr < 10; ListNr++) {
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			in.get( Waste );
		in.get( NameLen );
		for( NameNr = 0; NameNr < NameLen; NameNr++ ){
			for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
				in.get( Waste );
			in.get(	c );
			ScoreList[ListNr].Name[NameNr] = c - ListNr - 11;
		}
		ScoreList[ListNr].Name[NameNr] = '\0';
		for( WasteNr = 0; WasteNr < RandomNr; WasteNr++)
			in.get( Waste );
		in.get( c );
		ScoreList[ListNr].Score = c - ListNr - 11;
	}
}	
				
				
				
void main()
{
	
	strcpy((char*)ScoreList[0].Name, "nnnnn");
	ScoreList[0].Score = 10;
	strcpy((char*)ScoreList[1].Name, "nnnnnnnnnnnnnnnnnnnnnn");
	ScoreList[1].Score = 15;
	strcpy((char*)ScoreList[2].Name, "oooooooooooo");
	ScoreList[2].Score = 16;
	strcpy((char*)ScoreList[3].Name, "š„”á");
	ScoreList[3].Score = 20;
	strcpy((char*)ScoreList[4].Name, "Paul");
	ScoreList[4].Score = 30;
 
	cout << "Vorher\n";
	for( int i = 0; i < 10; i++ )
		cout << ScoreList[i].Name << "   " << ScoreList[i].Score << '\n';
	SaveHigh();
	GetHigh();
	cout << "\n\nNachher\n";
	for( i = 0; i < 10; i++ )
		cout << ScoreList[i].Name << "   " << ScoreList[i].Score << '\n';
	return;
}
