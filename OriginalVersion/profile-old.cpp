/*************************************************************************\
 * profile.c
 *	Any profile (crazy.ini) related stuff is located here.
 * Set tabs to 3 to get a readable source code.
\*************************************************************************/
#define INCL_WINSHELLDATA
#include <os2.h>
#include <string.h>
#include <malloc.h>

#include "defs.h"		// needed for info.h
#include "info.h"
#include "sound.h"
#include "profile.h"

#define MaxVersionLen 8

struct {
	PSZ pszVersion;
	PSZ pszWinPos;
	PSZ pszLineStyle;
	PSZ pszSound;
} PrfKeys = {
	"Version",
	"WindowPos",
	"LineStyle",
	"SoundFlag"
};

PSZ pszAppName = "Bermuda Triangle Ship Rescue";
PSZ pszIniName = "bermuda.ini";
PSZ pszVersion = "1.0.0";



/*************************************************************************\
 * function StoreWindowPos
 * This function is meant as a replacement for the os/2 library function
 * WinStoreWindowPos. It stores the current size and postion of the window
 * hwnd (window handle) to the profile hini (profile handle).
 * I use my own function because the original always stores it's
 * information in os2.ini which slows down system performace. 
 * Note: The function WinStoreWindowPos does a bit more. It also saves
 * the presentation parameters (i.e. color, fonts etc) of the window.
 * Because I don't need this it's not included here. (It would be a lot of
 * stupid work.)
 * Like the original this function returns TRUE if it was successful and
 * FALSE otherwise.
\*************************************************************************/
BOOL StoreWindowPos(	const HINI hini, const PSZ pszAppName,
							const PSZ pszKeyName, const HWND hwnd )
{
    SWP swp;				/* structure to save window information returned
					 * by WinQueryWindowPos 									*/
    LONG lWinPos[4];
	
	
    if ( !WinQueryWindowPos( hwnd, &swp ) ) return FALSE;
    lWinPos[0] = swp.x;
    lWinPos[1] = swp.y;
    lWinPos[2] = swp.cx;
    lWinPos[3] = swp.cy;
    /* write the values of swp.cy, swp.cx, swp.y and swp.x only */
    if ( !PrfWriteProfileData( hini, pszAppName, pszKeyName,
			       lWinPos, 4 * sizeof( LONG ) )
	) return FALSE;
    return TRUE;
}

/*************************************************************************\
 * function RestoreWindowPos
 * Analoguous to StoreWindowPos but the contrary action.
\*************************************************************************/
BOOL RestoreWindowPos( const HINI hini, const PSZ pszAppName,
										const PSZ pszKeyName, const HWND hwnd )
{
	LONG lWinPos[4];
	ULONG ulDataLen = 4 * sizeof( LONG );

	/* Overwrite the values of swp.cy, swp.cx, swp.y and swp.x. */
	if ( !PrfQueryProfileData( hini, pszAppName, pszKeyName,
										lWinPos, &ulDataLen )
		) return FALSE;
	/* Set new position */
	if ( !WinSetWindowPos( hwnd, NULLHANDLE,
								  lWinPos[0], lWinPos[1], lWinPos[2], lWinPos[3],
								  SWP_SIZE | SWP_MOVE )
		) return FALSE;
	return TRUE;
}


/*************************************************************************\
 * function ReadProfile()
 * Tries to open the file "bermuda.ini" and sets game specific options taken
 * from the profile data. If the file isn't found or the data is invalid
 * FALSE is returned otherwise the function returns TRUE.
\*************************************************************************/
BOOL ReadProfile1( HAB hab )
{
	CHAR pszVersionFound[MaxVersionLen] = "";
	ULONG lNumRead;
	BOOL  tmpSound;
	HINI hini;
	PSZ pszIniNameCopy;


	pszIniNameCopy = (PSZ)alloca( strlen( pszIniName ) + 1);
	strcpy( pszIniNameCopy, pszIniName );
	if ( !(hini = PrfOpenProfile( hab, pszIniNameCopy )) ) goto Error;

	// query version string
	PrfQueryProfileString( hini, pszAppName, PrfKeys.pszVersion, NULL,
			       pszVersionFound, MaxVersionLen );
	// and compare with current version								  
	if ( strcmp( pszVersion, pszVersionFound ) != 0 ){
		PrfCloseProfile( hini );
		return FALSE;
	}

	// retrieve sound flag
	lNumRead = sizeof( BOOL );
	if( !PrfQueryProfileData( hini, pszAppName, PrfKeys.pszSound,
		 &tmpSound, &lNumRead ) )
		 goto Error;
	Sound::SetSoundWanted( tmpSound );		 

	PrfCloseProfile( hini );
	return TRUE;

Error:
	PrfCloseProfile( hini );
	return FALSE;
}

/*************************************************************************\
 * function ReadProfile()
 * Tries to open the file "bermuda.ini" and sets game specific options taken
 * from the profile data. If the file isn't found or the data is invalid
 * FALSE is returned otherwise the function returns TRUE.
\*************************************************************************/
BOOL ReadProfile( HAB hab )
{
	CHAR pszVersionFound[MaxVersionLen] = "";
	ULONG lNumRead;
	ULONG tmpLineStyle;
	BOOL  tmpSound;
	HINI hini;
	PSZ pszIniNameCopy;


	pszIniNameCopy = (PSZ)alloca( strlen( pszIniName ) + 1);
	strcpy( pszIniNameCopy, pszIniName );
	if ( !(hini = PrfOpenProfile( hab, pszIniNameCopy )) ) goto Error;

	// query version string
	PrfQueryProfileString( hini, pszAppName, PrfKeys.pszVersion, NULL,
								  pszVersionFound, MaxVersionLen );
	// and compare with current version								  
	if ( strcmp( pszVersion, pszVersionFound ) != 0 ){
		PrfCloseProfile( hini );
		return FALSE;
	}

	// retrieve line style settings
	lNumRead = sizeof( ULONG );
	if( !PrfQueryProfileData( hini, pszAppName, PrfKeys.pszLineStyle,
		 &tmpLineStyle, &lNumRead ) ) goto Error;
	InfoData.SetLineStyle( tmpLineStyle );		 
	// retrieve sound flag
	lNumRead = sizeof( BOOL );
	if( !PrfQueryProfileData( hini, pszAppName, PrfKeys.pszSound,
		 &tmpSound, &lNumRead ) )
		 goto Error;
	Sound::SetSoundWanted( tmpSound );		 

	if ( !RestoreWindowPos( hini, pszAppName, PrfKeys.pszWinPos, hwndFrame )
		) goto Error;
	PrfCloseProfile( hini );
	return TRUE;

Error:
	PrfCloseProfile( hini );
	return FALSE;
}

/*************************************************************************\
 * function WriteProfile()
 * Trys to open the file "bermuda.ini" and saves game specific options 
 * from the profile data. 
\*************************************************************************/
BOOL WriteProfile( HAB hab )
{
	HINI hini;
	PSZ pszIniNameCopy;
	ULONG tmpLineStyle;
	BOOL  tmpSound;


	pszIniNameCopy = (PSZ)alloca( strlen( pszIniName ) + 1);
	strcpy( pszIniNameCopy, pszIniName );
	if ( !(hini = PrfOpenProfile( hab, pszIniNameCopy )) ) goto Error;
	if( !PrfWriteProfileString( hini, pszAppName,
										 PrfKeys.pszVersion, pszVersion ) )
		goto Error;
	// write line style settings
	tmpLineStyle = InfoData.GetLineStyle();
	if( !PrfWriteProfileData( hini, pszAppName, PrfKeys.pszLineStyle,
									  &tmpLineStyle, (ULONG)sizeof( ULONG ) ) )
		goto Error;
	// write sound flag
	tmpSound = Sound::GetSoundWanted();
	if( !PrfWriteProfileData( hini, pszAppName, PrfKeys.pszSound,
		 &tmpSound, (ULONG)sizeof( BOOL ) ) )
		 goto Error;

	if( !StoreWindowPos( hini, pszAppName, PrfKeys.pszWinPos, hwndFrame ) )
		goto Error;
	PrfCloseProfile( hini );
	return TRUE;
Error:
	PrfCloseProfile( hini );
	return FALSE;
}

