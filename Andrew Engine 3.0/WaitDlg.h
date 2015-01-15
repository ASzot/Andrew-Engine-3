//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include <process.h>
#include <Windows.h>
#include "DXUT.h"
#include "resource.h"

//--------------------------------------------------------------------------------------
INT_PTR CALLBACK WaitDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam );
unsigned int __stdcall WaitThread( void* pArg );

//--------------------------------------------------------------------------------------
class CWaitDlg
{
private:
    HWND	m_hDialogWnd;
    HANDLE	m_hThread;
    HWND	m_hProgressWnd;
    int		m_iProgress;
    BOOL	m_bDone;
    RECT	m_AppRect;
    WCHAR	m_szText[MAX_PATH];

public:
    CWaitDlg();
    ~CWaitDlg();

    bool IsRunning() { return !m_bDone; }

    void UpdateProgressBar();

    bool GetDialogControls();

    bool ShowDialog( WCHAR* pszInputText );

    void DestroyDialog();
};


