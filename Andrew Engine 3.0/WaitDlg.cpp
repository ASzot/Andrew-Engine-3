//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "WaitDlg.h"

CWaitDlg::CWaitDlg() :
		m_hDialogWnd( 0 ),
		m_hThread( 0 ),
		m_hProgressWnd( 0 ),
		m_iProgress( 0 ),
		m_bDone( FALSE )
{

}

CWaitDlg::~CWaitDlg()
{
	DestroyDialog();
}


void CWaitDlg::UpdateProgressBar()
{
    m_iProgress ++;
    if( m_iProgress > 110 )
        m_iProgress = 0;

    SendMessage( m_hProgressWnd, PBM_SETPOS, m_iProgress, 0 );
    InvalidateRect( m_hDialogWnd, NULL, FALSE );
    UpdateWindow( m_hDialogWnd );
}

bool CWaitDlg::GetDialogControls() 
{
    m_bDone = FALSE;

    m_hDialogWnd = CreateDialog( DXUTGetHINSTANCE(), MAKEINTRESOURCE( IDD_DIALOG_LOADING ), NULL, WaitDialogProc );
    if( !m_hDialogWnd )
        return false;

	SetWindowLongPtr( m_hDialogWnd, GWLP_USERDATA, (LONG_PTR)this );

    // Set the position
    int left = ( m_AppRect.left + m_AppRect.right ) / 2;
    int up = ( m_AppRect.top + m_AppRect.bottom ) / 2;

    SetWindowPos( m_hDialogWnd, NULL, left, up, 0, 0, SWP_NOSIZE );
    ShowWindow( m_hDialogWnd, SW_SHOW );

    // Get the progress bar
    m_hProgressWnd = GetDlgItem( m_hDialogWnd, IDC_PROGRESS_LOADING );
    SendMessage( m_hProgressWnd, PBM_SETRANGE, 0, MAKELPARAM( 0, 100 ) );
	// 0x00cd00 is some shade of green.
	SendMessage( m_hProgressWnd, PBM_SETBARCOLOR, 0, COLORREF(0x00cd00) );

    // Update the static text
    HWND hMessage = GetDlgItem( m_hDialogWnd, IDC_LOADING_MESSAGE );
    SetWindowText( hMessage, m_szText );
}

bool CWaitDlg::ShowDialog( WCHAR* pszInputText )
{
    // Get the window rect
    GetWindowRect( DXUTGetHWND(), &m_AppRect );
    wcscpy_s( m_szText, MAX_PATH, pszInputText );

    // spawn a thread that does nothing but update the progress bar
    unsigned int threadAddr;
    m_hThread = (HANDLE)_beginthreadex( NULL, 0, WaitThread, this, 0, &threadAddr );
    return true;
}

void CWaitDlg::DestroyDialog()
{
	m_bDone = TRUE;
    WaitForSingleObject( m_hThread, INFINITE );

    if( m_hDialogWnd )
        DestroyWindow( m_hDialogWnd );
    m_hDialogWnd = NULL;
}

INT_PTR CALLBACK WaitDialogProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    CWaitDlg* pThisDialog = (CWaitDlg*)GetWindowLongPtr( hwndDlg, GWLP_USERDATA );

    switch( uMsg )
    {
    case WM_INITDIALOG:
        return TRUE;
    case WM_CLOSE:
        pThisDialog->DestroyDialog();
        return TRUE;
    }

    return FALSE;
}

unsigned int __stdcall WaitThread( void* pArg )
{
    CWaitDlg* pThisDialog = (CWaitDlg*)pArg;

    // We create the dialog in this thread, so we can call SendMessage without blocking on the
    // main thread's message pump
    pThisDialog->GetDialogControls();

    while( pThisDialog->IsRunning() )
    {
        pThisDialog->UpdateProgressBar();
        Sleep(100);
    }

    return 0;
}
