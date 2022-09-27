//======================================================================
// Header file
//
// Written for the book Programming Windows CE
// Copyright (C) 1998 Douglas Boling
//
//======================================================================
// Returns number of elements
#define dim(x) (sizeof(x) / sizeof(x[0]))

//----------------------------------------------------------------------
// Generic defines and data types
//
struct decodeUINT {                             // Structure associates
    UINT Code;                                  // messages 
                                                // with a function. 
    LRESULT (*Fxn)(HWND, UINT, WPARAM, LPARAM);
}; 
struct decodeCMD {                              // Structure associates
    UINT Code;                                  // menu IDs with a 
    LRESULT (*Fxn)(HWND, WORD, HWND, WORD);     // function.
};

//----------------------------------------------------------------------
// Generic defines used by application
#define  IDC_CMDBAR 1                           // Command bar ID

#define  ID_ICON           10                   // Icon resource ID
#define  ID_MENU           11                   // Main menu resource ID
#define  ID_ACCEL          12                   // Main menu resource ID

#define  IDM_NEWGAME       100                  // Menu item ID
#define  IDM_UNDO          101                  // Menu item ID
#define  IDM_EXIT          102                  // Menu item ID

#define  IDS_XTURN         201                  // String ID
#define  IDS_OTURN         202                  // String ID

//----------------------------------------------------------------------
// Function prototypes
//
int InitApp (HINSTANCE);
HWND InitInstance (HINSTANCE, LPWSTR, int);
int TermInstance (HINSTANCE, int);

// Window procedures
LRESULT CALLBACK MainWndProc (HWND, UINT, WPARAM, LPARAM);

// Message handlers
LRESULT DoCreateMain (HWND, UINT, WPARAM, LPARAM);
LRESULT DoSizeMain (HWND, UINT, WPARAM, LPARAM);
LRESULT DoPaintMain (HWND, UINT, WPARAM, LPARAM);
LRESULT DoInitMenuPopMain (HWND, UINT, WPARAM, LPARAM);
LRESULT DoCommandMain (HWND, UINT, WPARAM, LPARAM);
LRESULT DoLButtonUpMain (HWND, UINT, WPARAM, LPARAM);
LRESULT DoDestroyMain (HWND, UINT, WPARAM, LPARAM);

// Command functions
LPARAM DoMainCommandNewGame (HWND, WORD, HWND, WORD);
LPARAM DoMainCommandUndo (HWND, WORD, HWND, WORD);
LPARAM DoMainCommandExit (HWND, WORD, HWND, WORD);

// Game functon prototypes
void ResetGame (void);
void DrawXO (HDC hdc, HPEN hPen, RECT *prect, INT nCell, INT nType);
void DrawBoard (HDC hdc, RECT *prect);

