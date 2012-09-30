///////////////////////////////////////////////////////////////////
// bermuda.h

struct GRAPHICS_PARAMETERS {
	char Row;
	char Col;
	char aktscan;
	BOOL flag;
};

extern GRAPHICS_PARAMETERS graphparms;


// messages


// global variables

extern HAB hab;
extern HMQ hmq;
extern HWND hwndFrame;
extern HWND hwndMain;

extern INFO InfoData;
extern GRAPHBOARD GBoard;

// Semaphore handles
extern HEV hevWaitAfterScan;
extern HEV hevHiScoreWin;
extern HEV hevWaitAfterSound;	// blocks the next scanning until the previous is done
extern HEV hevWaitSoundReady;

