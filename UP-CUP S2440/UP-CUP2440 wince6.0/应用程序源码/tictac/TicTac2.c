//======================================================================
// TicTac2 - Simple tic-tac-toe game with menus
//
// Written for the book Programming Windows CE
// Copyright (C) 1998 Douglas Boling
//
//======================================================================
#include <windows.h>                 // For all that Windows stuff
#include <commctrl.h>                // Command bar includes
#include "tictac2.h"                 // Program-specific stuff

//----------------------------------------------------------------------
// Global data
//
const TCHAR szAppName[] = TEXT ("TicTac2");
HINSTANCE hInst;                     // Program instance handle

// State data for game
RECT rectBoard = {0, 0, 0, 0};       // Used to place game board.
RECT rectPrompt;                     // Used to place prompt.
BYTE bBoard[9];                      // Keeps track of Xs and Os.
BYTE bTurn;                          // Keeps track of the turn.
char bLastMove;                      // Last cell changed

// Message dispatch table for MainWindowProc
const struct decodeUINT MainMessages[] = {
    WM_CREATE, DoCreateMain,
    WM_SIZE, DoSizeMain,
    WM_PAINT, DoPaintMain,
    WM_INITMENUPOPUP, DoInitMenuPopMain,
    WM_COMMAND, DoCommandMain,
    WM_LBUTTONUP, DoLButtonUpMain,
    WM_DESTROY, DoDestroyMain,
};

// Command Message dispatch for MainWindowProc
const struct decodeCMD MainCommandItems[] = {
    IDM_NEWGAME, DoMainCommandNewGame,
    IDM_UNDO, DoMainCommandUndo,
    IDM_EXIT, DoMainCommandExit,
};

//======================================================================
//
// Program entry point
//
int WINAPI WinMain (HINSTANCE hInstance, HINSTANCE hPrevInstance,
                    LPWSTR lpCmdLine, int nCmdShow) {
    MSG msg;
    int rc = 0;
    HWND hwndMain;
    HACCEL hAccel;

    // Initialize application.
    rc = InitApp (hInstance);
    if (rc) return rc;

    // Initialize this instance.
    hwndMain = InitInstance (hInstance, lpCmdLine, nCmdShow);
    if (hwndMain == 0)
        return 0x10;

    // Load accelerator table.
    hAccel = LoadAccelerators (hInst, MAKEINTRESOURCE (ID_ACCEL));

    // Application message loop
    while (GetMessage (&msg, NULL, 0, 0)) {
        // Translate accelerators 
        if (!TranslateAccelerator (hwndMain, hAccel, &msg)) {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }
    }
    // Instance cleanup
    return TermInstance (hInstance, msg.wParam);
}
//----------------------------------------------------------------------
// InitApp - Application initialization
//
int InitApp (HINSTANCE hInstance) {
    WNDCLASS wc;

    // Register application main window class.
    wc.style = 0;                             // Window style
    wc.lpfnWndProc = MainWndProc;             // Callback function
    wc.cbClsExtra = 0;                        // Extra class data
    wc.cbWndExtra = 0;                        // Extra window data
    wc.hInstance = hInstance;                 // Owner handle
    wc.hIcon = NULL,                          // Application icon
    wc.hCursor = NULL;                        // Default cursor
    wc.hbrBackground = (HBRUSH) GetStockObject (WHITE_BRUSH);
    wc.lpszMenuName =  NULL;                  // Menu name
    wc.lpszClassName = szAppName;             // Window class name

    if (RegisterClass(&wc) == 0) return 1;

    return 0;
}
//----------------------------------------------------------------------
// InitInstance - Instance initialization
//
HWND InitInstance (HINSTANCE hInstance, LPWSTR lpCmdLine, int nCmdShow) {
    HWND hWnd;

    // Save program instance handle in global variable.
    hInst = hInstance;

    // Create main window.
    hWnd = CreateWindow (szAppName,           // Window class
                         TEXT ("TicTac2"),    // Window title
                         WS_VISIBLE,          // Style flags
                         CW_USEDEFAULT,       // x position
                         CW_USEDEFAULT,       // y position
                         CW_USEDEFAULT,       // Initial width
                         CW_USEDEFAULT,       // Initial height
                         NULL,                // Parent                
                         NULL,                // Menu, must be null
                         hInstance,           // Application instance
                         NULL);               // Pointer to create
                                              // parameters
    // Return fail code if window not created.
    if (!IsWindow (hWnd)) return 0;

    // Standard show and update calls
    ShowWindow (hWnd, nCmdShow);
    UpdateWindow (hWnd);
    return hWnd;
}
//----------------------------------------------------------------------
// TermInstance - Program cleanup
//
int TermInstance (HINSTANCE hInstance, int nDefRC) {

    return nDefRC;
}
//======================================================================
// Message handling procedures for MainWindow
//

//----------------------------------------------------------------------
// MainWndProc - Callback function for application window
//
LRESULT CALLBACK MainWndProc (HWND hWnd, UINT wMsg, WPARAM wParam,
                              LPARAM lParam) {
    INT i;
    //
    // Search message list to see if we need to handle this
    // message.  If in list, call procedure.
    //
    for (i = 0; i < dim(MainMessages); i++) {
        if (wMsg == MainMessages[i].Code)
            return (*MainMessages[i].Fxn)(hWnd, wMsg, wParam, lParam);
    }
    return DefWindowProc (hWnd, wMsg, wParam, lParam);
}
//----------------------------------------------------------------------
// DoCreateMain - Process WM_CREATE message for window.
//
LRESULT DoCreateMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                      LPARAM lParam) {
    HWND hwndCB;
    HICON hIcon;

    // Create a command bar.
    hwndCB = CommandBar_Create (hInst, hWnd, IDC_CMDBAR);
    // Add the menu.
    CommandBar_InsertMenubar (hwndCB, hInst, ID_MENU, 0);
    // Add exit button to command bar. 
    CommandBar_AddAdornments (hwndCB, 0, 0);

    hIcon = (HICON) SendMessage (hWnd, WM_GETICON, 0, 0);
    if (hIcon == 0) {
        hIcon = LoadImage (hInst, MAKEINTRESOURCE (ID_ICON),
                           IMAGE_ICON, 16, 16, 0);
        SendMessage (hWnd, WM_SETICON, FALSE, (LPARAM)hIcon);
    }

    // Initialize game.
    ResetGame ();
    return 0;
}
//----------------------------------------------------------------------
// DoSizeMain - Process WM_SIZE message for window.
//
LRESULT DoSizeMain (HWND hWnd, UINT wMsg, WPARAM wParam,
                    LPARAM lParam) {
    RECT rect;

    // Adjust the size of the client rect to take into account
    // the command bar height.
    GetClientRect (hWnd, &rect);
    rect.top += CommandBar_Height (GetDlgItem (hWnd, IDC_CMDBAR));

    // Define the playing board rect.
    rectBoard = rect;
    rectPrompt = rect;
    // Layout depends on portrait or landscape screen.
    if (rect.right - rect.left > rect.bottom - rect.top) {
        rectBoard.left += 20;
        rectBoard.top += 10;
        rectBoard.bottom -= 10;
        rectBoard.right = rectBoard.bottom - rectBoard.top + 10; 

        rectPrompt.left = rectBoard.right + 10;

    } else {
        rectBoard.left += 20;
        rectBoard.right -= 20;
        rectBoard.top += 10;
        rectBoard.bottom = rectBoard.right - rectBoard.left + 10;

        rectPrompt.top = rectBoard.bottom + 10;
    }
    return 0;
}
//----------------------------------------------------------------------
// DoPaintMain - Process WM_PAINT message for window.
//
LRESULT DoPaintMain (HWND hWnd, UINT wMsg, WPARAM wParam,
                     LPARAM lParam) {
    PAINTSTRUCT ps;
    RECT rect;
    HFONT hFont, hOldFont;
    TCHAR szPrompt[32];
    HDC hdc;

    // Adjust the size of the client rect to take into account
    // the command bar height.
    GetClientRect (hWnd, &rect);
    rect.top += CommandBar_Height (GetDlgItem (hWnd, IDC_CMDBAR));

    hdc = BeginPaint (hWnd, &ps);

    // Draw the board.
    DrawBoard (hdc, &rectBoard);

    // Write the prompt to the screen.
    hFont = GetStockObject (SYSTEM_FONT);
    hOldFont = SelectObject (hdc, hFont);

    if (bTurn == 0)
        LoadString (hInst, IDS_XTURN, szPrompt, sizeof (szPrompt));
    else
        LoadString (hInst, IDS_OTURN, szPrompt, sizeof (szPrompt));

    DrawText (hdc, szPrompt, -1, &rectPrompt, 
              DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject (hdc, hOldFont);
    EndPaint (hWnd, &ps);
    return 0;
}
//----------------------------------------------------------------------
// DoInitMenuPopMain - Process WM_INITMENUPOPUP message for window.
//
LRESULT DoInitMenuPopMain (HWND hWnd, UINT wMsg, WPARAM wParam,
                           LPARAM lParam) {
    HMENU hMenu;

    hMenu = CommandBar_GetMenu (GetDlgItem (hWnd, IDC_CMDBAR), 0);

    if (bLastMove == -1) 
        EnableMenuItem (hMenu, IDM_UNDO, MF_BYCOMMAND | MF_GRAYED);
    else
        EnableMenuItem (hMenu, IDM_UNDO,  MF_BYCOMMAND | MF_ENABLED);
    return 0;
}
//----------------------------------------------------------------------
// DoCommandMain - Process WM_COMMAND message for window. 
// 
//
LRESULT DoCommandMain (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {
    WORD idItem, wNotifyCode;
    HWND hwndCtl;
    INT  i;

    // Parse the parameters.
    idItem = (WORD) LOWORD (wParam);
    wNotifyCode = (WORD) HIWORD(wParam);
    hwndCtl = (HWND) lParam;

    // Call routine to handle control message.
    for (i = 0; i < dim(MainCommandItems); i++) {
        if (idItem == MainCommandItems[i].Code)
            return (*MainCommandItems[i].Fxn)(hWnd, idItem, hwndCtl, 
                                           wNotifyCode);
    }
    return 0;
}
//----------------------------------------------------------------------
// DoLButtonUpMain - Process WM_LBUTTONUP message for window.
//
LRESULT DoLButtonUpMain (HWND hWnd, UINT wMsg, WPARAM wParam, 
                         LPARAM lParam) {
    POINT pt;
    INT cx, cy, nCell = 0;

    pt.x = LOWORD (lParam);
    pt.y = HIWORD (lParam);

    // See if pen on board.  If so, determine which cell.
    if (PtInRect (&rectBoard, pt)){
        // Normalize point to upper left corner of board.
        pt.x -= rectBoard.left;
        pt.y -= rectBoard.top;

        // Compute size of each cell.
        cx = (rectBoard.right - rectBoard.left)/3;
        cy = (rectBoard.bottom - rectBoard.top)/3;

        // Find column.
        nCell = (pt.x / cx); 

        // Find row.
        nCell += (pt.y / cy) * 3; 

        // If cell empty, fill it with mark.
        if (bBoard[nCell] == 0) {
            if (bTurn) {
                bBoard[nCell] = 2;
                bTurn = 0;
            } else {
                bBoard[nCell] = 1;
                bTurn = 1;
            }
            // Save the cell for the undo command.
            bLastMove = nCell;
            // Force the screen to be repainted.
            InvalidateRect (hWnd, NULL, FALSE);
        } else {
            // Inform the user of the filled cell.
            MessageBeep (0);
            return 0;
        }
    }
    return 0;
}
//----------------------------------------------------------------------
// DoDestroyMain - Process WM_DESTROY message for window.
//
LRESULT DoDestroyMain (HWND hWnd, UINT wMsg, WPARAM wParam,
                       LPARAM lParam) {
    PostQuitMessage (0);
    return 0;
}
//======================================================================
// Command handler routines
//
//----------------------------------------------------------------------
// DoMainCommandNewGame - Process New Game command.
//
LPARAM DoMainCommandNewGame (HWND hWnd, WORD idItem, HWND hwndCtl,
                             WORD wNotifyCode) {
    INT i, j = 0, rc;

    // Count the number of used spaces.
    for (i = 0; i < 9; i++)
        if (bBoard[i])
            j++;

    // If not new game or complete game, ask user before clearing.
    if (j && (j != 9)) {
        rc = MessageBox (hWnd,
                         TEXT ("Are you sure you want to clear the board?"),
                         TEXT ("New Game"), MB_YESNO | MB_ICONQUESTION);
        if (rc == IDNO)
            return 0;
    }
    ResetGame ();
    InvalidateRect (hWnd, NULL, TRUE);
    return 0;
}
//----------------------------------------------------------------------
// DoMainCommandUndo - Process Undo Last Move command.
//
LPARAM DoMainCommandUndo (HWND hWnd, WORD idItem, HWND hwndCtl,
                          WORD wNotifyCode) {

    if (bLastMove != -1) {
        bBoard[bLastMove] = 0;
        if (bTurn) {
            bTurn = 0;
        } else {
            bTurn = 1;
        }
        // Only one level of undo
        bLastMove = -1;
        InvalidateRect (hWnd, NULL, TRUE);
    }
    return 0;
}
//----------------------------------------------------------------------
// DoMainCommandExit - Process Program Exit command.
//
LPARAM DoMainCommandExit (HWND hWnd, WORD idItem, HWND hwndCtl, 
                          WORD wNotifyCode) {

    SendMessage (hWnd, WM_CLOSE, 0, 0);
    return 0;
}
//======================================================================
// Game-specific routines
//
//----------------------------------------------------------------------
// ResetGame - Initialize the structures for a game.
//
void ResetGame (void) {
    INT i;

    // Initialize the board.
    for (i = 0; i < dim(bBoard); i++)
        bBoard[i] = 0;

    bTurn = 0;
    bLastMove = -1;
    return;
}
//----------------------------------------------------------------------
// DrawXO - Draw a single X or O in a square.
//
void DrawXO (HDC hdc, HPEN hPen, RECT *prect, INT nCell, INT nType) {
    POINT pt[2];
    INT cx, cy;
    RECT rect;

    cx = (prect->right - prect->left)/3;
    cy = (prect->bottom - prect->top)/3;

    // Compute the dimensions of the target cell.
    rect.left = (cx * (nCell % 3) + prect->left) + 10;
    rect.right = rect.left + cx - 20,
    rect.top = cy * (nCell / 3) + prect->top + 10,
    rect.bottom =  rect.top + cy - 20;

    // Draw an X?
    if (nType == 1) {
        pt[0].x = rect.left;
        pt[0].y = rect.top;
        pt[1].x = rect.right;
        pt[1].y = rect.bottom;
        Polyline (hdc, pt, 2);

        pt[0].x = rect.right;
        pt[1].x = rect.left;
        Polyline (hdc, pt, 2);
    // How about an O?
    } else if (nType == 2) {
        Ellipse (hdc, rect.left, rect.top, rect.right, rect.bottom);
    }
    return;
}
//----------------------------------------------------------------------
// DrawBoard - Draw the tic-tac-toe board.
//
void DrawBoard (HDC hdc, RECT *prect) {
    HPEN hPen, hOldPen;
    POINT pt[2];
    LOGPEN lp;
    INT i, cx, cy;

    // Create a nice thick pen.
    lp.lopnStyle = PS_SOLID;
    lp.lopnWidth.x = 5;
    lp.lopnWidth.y = 5;
    lp.lopnColor = RGB (0, 0, 0);
    hPen = CreatePenIndirect (&lp);

    hOldPen = SelectObject (hdc, hPen);

    cx = (prect->right - prect->left)/3;
    cy = (prect->bottom - prect->top)/3;

    // Draw lines down.
    pt[0].x = cx + prect->left;
    pt[1].x = cx + prect->left;
    pt[0].y = prect->top;
    pt[1].y = prect->bottom;
    Polyline (hdc, pt, 2);

    pt[0].x += cx;
    pt[1].x += cx;
    Polyline (hdc, pt, 2);

    // Draw lines across.
    pt[0].x = prect->left;
    pt[1].x = prect->right;
    pt[0].y = cy + prect->top;
    pt[1].y = cy + prect->top;
    Polyline (hdc, pt, 2);

    pt[0].y += cy;
    pt[1].y += cy;
    Polyline (hdc, pt, 2);

    // Fill in Xs and Os.
    for (i = 0; i < dim (bBoard); i++)
        DrawXO (hdc, hPen, &rectBoard, i, bBoard[i]);

    SelectObject (hdc, hOldPen);
    DeleteObject (hPen);
    return;
}
