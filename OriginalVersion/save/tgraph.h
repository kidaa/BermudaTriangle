///////////////////////////////
// start of file tgraph.h -------------------------------------------------------

// strings
#define APP_CLASS_CLIENT        "APPClient"
#define APP_CLASS_OBJECT1        "APPObjectGph"
#define APP_CLASS_OBJECT2        "APPObjectSnd"

// identifiers
#define ID_APP             3
#define IDM_SLEEP         303
#define IDM_ACTIONS       304

// lengths
#define LEN_WORKSTRING              256
#define LEN_STACK                 18000

// structure to hold globals variables common to both threads
struct GLOBALS {
  BOOL           fPainting;
  HAB            hab;
  HWND           hwndClient;
  HWND           hwndFrame;
  HWND           hwndTitlebar;
  HWND           hwndMenubar;
  HWND           hwndTGraph;
  TID            tidTGraph;
  HWND			  hwndTSound;
  TID				  tidTSound;
};

extern GLOBALS *pg;

// user-defined messages for work items and acknowlegements
#define WM_USER_ACK                    (WM_USER+0)
#define WM_USER_SLEEP                  (WM_USER+1)
#define WM_USER_ENABLE                 (WM_USER+2)
#define WM_USER_DISABLE                (WM_USER+3)
#define WM_USER_PAINT						(WM_USER+4)
#define WM_SOUND_ACK                   (WM_USER+5)

#define WM_SOUND_INTRO                 (WM_USER+6)
#define WM_SOUND_FOUND0						(WM_USER+7)
#define WM_SOUND_FOUNDSHIP					(WM_USER+8)
#define WM_SOUND_FOUND						(WM_USER+9)
#define WM_SOUND_NEWHISCORE				(WM_USER+11)
#define WM_SOUND_HOCHTIEF					(WM_USER+14)
#define WM_SOUND_LOST						(WM_USER+15)

#define WM_GRAPH_SCAN						(WM_USER+100)
#define WM_DRAWPMPLACE						(WM_USER+101)
#define WM_DRAWPMMARK						(WM_USER+102)
#define WM_SHOWPOINTERPOS					(WM_USER+103)
#define WM_DRAWDRAGLINE						(WM_USER+104)
#define WM_MARKDRAGLINE						(WM_USER+105)
#define WM_DISPLAYLINES						(WM_USER+106)
#define WM_SHOWSTATUSLINE					(WM_USER+107)



// function prototypes 
void threadgraph(void *);   // ge„ndert von nichts auf void *
void threadsound(void *);   // ge„ndert von nichts auf void *
GLOBALS *Create( HWND hwnd );
MRESULT EXPENTRY TGraphWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
MRESULT EXPENTRY TSoundWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );
// MRESULT EXPENTRY ClientWinProc( HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2 );

// end of file tgraph.h ----------------------------------------------------------

