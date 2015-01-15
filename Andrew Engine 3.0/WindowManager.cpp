//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "DXUT.h"
#include "WindowManager.h"

namespace
{
	CWindowManager* g_pWindowManager = 0;
}

BOOL CALLBACK BasicWindowProcDummy(HWND hwnd, UINT message,WPARAM wparam,LPARAM lparam);
BOOL CALLBACK BasicModalWindowProcDummy(HWND hwnd,UINT message, WPARAM wparam, LPARAM lParam);

BOOL CALLBACK BasicWindowProcDummy(HWND hwnd, UINT message,WPARAM wparam, LPARAM lparam)
{
	return g_pWindowManager->BasicWindowProc(hwnd,message,wparam,lparam);
}

BOOL CALLBACK BasicModalWindowProcDummy(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	return g_pWindowManager->BasicModalWindowProc(hwnd,message,wParam,lParam);
}



CWindowManager::CWindowManager(void)
{
	g_pWindowManager = this;
	mCurrentWindow = 0;
	mMostRecentlyPressedButton = 0;
	mIsMainWndMenu = false;
}


CWindowManager::~CWindowManager(void)
{
}


// Used to create generic windows style windows
void CWindowManager::CreateDialogWindow(unsigned int dialogResource, HINSTANCE hInstance, HWND parentWindow)
{
	mCurrentWindow = CreateDialog(hInstance, MAKEINTRESOURCE(dialogResource),parentWindow,BasicWindowProcDummy);
	ShowWindow(mCurrentWindow, SW_SHOW);
	UpdateWindow(mCurrentWindow);
}


// Called by the basic dummy window proc through the global version
BOOL CWindowManager::BasicWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_INITDIALOG:
		break;
		
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_RESUME:
			mMostRecentlyPressedButton = IDC_BUTTON_RESUME;
			DestroyWindow(mCurrentWindow);
			break;
		case IDC_BUTTON_PD_EXIT:
			mMostRecentlyPressedButton = IDC_BUTTON_PD_EXIT;
			DestroyWindow(mCurrentWindow);
			break;
		case IDC_BUTTON_PD_BUILD_LEVEL:
			mMostRecentlyPressedButton = IDC_BUTTON_PD_BUILD_LEVEL;
			DestroyWindow(mCurrentWindow);
			break;
		case IDC_BUTTON_TB_ADD_OBJECT:
			mMostRecentlyPressedButton = IDC_BUTTON_TB_ADD_OBJECT;
			break;
		case IDC_BUTTON_TB_DELETE_SELECTED_OBJECT:
			mMostRecentlyPressedButton = IDC_BUTTON_TB_DELETE_SELECTED_OBJECT;
			break;
		case IDC_BUTTON_TB_ADD_CUSTOM_OBJECT:
			mMostRecentlyPressedButton = IDC_BUTTON_TB_ADD_CUSTOM_OBJECT;
			break;
		}
		break;
		
	default:
		return FALSE;
	}

	return TRUE;
}


BOOL CWindowManager::DestroyCurrentWindow(HWND windowHandle)
{
	return DestroyWindow(windowHandle);
}


// Gets the most recently pressed button
UINT CWindowManager::ProccessWindows(void)
{
	UINT returnValue = mMostRecentlyPressedButton;
	mMostRecentlyPressedButton = 0;
	return returnValue;
}

// Used to create the menu for the main window
BOOL CWindowManager::CreateMainWindowMenu(HWND parentWindow)
{
	HMENU hMenu, hSubMenu;
	hMenu = CreateMenu();

	mMainWindow = parentWindow;

	hSubMenu = CreatePopupMenu();
	AppendMenu(hSubMenu,MF_STRING,ID_FILE_SAVE,L"Save");
	AppendMenu(hSubMenu,MF_STRING,ID_FILE_OPEN,L"Open");
	AppendMenu(hMenu,MF_STRING | MF_POPUP,(UINT)hSubMenu,L"File");

	mMainMenu = hMenu;
	mIsMainWndMenu = true;
	return SetMenu(parentWindow,hMenu);
}

BOOL CWindowManager::DestroyMainWindowMenu()
{
	mIsMainWndMenu = false;
	return DestroyMenu(mMainMenu);
}

INT_PTR CWindowManager::CreateModalDialogBox(unsigned int dialogResource, HINSTANCE hInstance, HWND hwnd)
{
	return DialogBox(hInstance,MAKEINTRESOURCE(dialogResource),hwnd, BasicModalWindowProcDummy);
}

BOOL CWindowManager::BasicModalWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
	case WM_COMMAND:
		switch(LOWORD(wParam))
		{
		case IDC_BUTTON_SD_ENTER:
			if(!GetDlgItemTextA(hwnd,IDC_TEXT_BOX_SD_FILE_NAME,mTextBuffer[INDEX_FILE_NAME_LOCATION],MAX_TEXT_BUFFER_SIZE))
				*mTextBuffer[INDEX_FILE_NAME_LOCATION] = 0;
			if(!GetDlgItemTextA(hwnd,IDC_TEXT_BOX_SD_LEVEL_NAME,mTextBuffer[INDEX_LEVEL_NAME_LOCATION],MAX_TEXT_BUFFER_SIZE))
				*mTextBuffer[INDEX_LEVEL_NAME_LOCATION] = 0;
			//Fall through to exit the dialog
		case IDC_BUTTON_SD_EXIT:
			EndDialog(hwnd,wParam);
			return TRUE;

		case IDC_BUTTON_OD_OK:
			if(!GetDlgItemTextA(hwnd,IDC_TEXT_BOX_OD_FILE_NAME,mTextBuffer[INDEX_FILE_NAME_LOCATION],MAX_TEXT_BUFFER_SIZE))
				*mTextBuffer[INDEX_FILE_NAME_LOCATION] = 0;
			//Fall through
		case IDC_BUTTON_OD_EXIT:
			EndDialog(hwnd,wParam);
			return TRUE;

		case IDC_BUTTON_AO_ADD_OBJECT:
			if(!GetDlgItemTextA(hwnd,IDC_EDIT_AO_OBJECT_NAME,mTextBuffer[INDEX_FILE_NAME_LOCATION],MAX_TEXT_BUFFER_SIZE))
				*mTextBuffer[INDEX_FILE_NAME_LOCATION] = 0;
		case IDC_BUTTON_AO_EXIT:
			EndDialog(hwnd,wParam);
			return TRUE;
			
		case IDC_BUTTON_UP_SAVE:
			if(!GetDlgItemTextA(hwnd,IDC_EDIT_UP_START_UP_LEVEL,mTextBuffer[INDEX_FILE_NAME_LOCATION],MAX_TEXT_BUFFER_SIZE))
				*mTextBuffer[INDEX_FILE_NAME_LOCATION] = 0;
		case IDC_BUTTON_UP_EXIT:
			EndDialog(hwnd,wParam);
			return TRUE;

		}
	}
	return FALSE;
}

char* CWindowManager::GetTextBuffer(int index)
{
	return mTextBuffer[index];
}

// This will be created on entering the builder state
HWND CWindowManager::CreateToolbox(HWND parentWindow,HINSTANCE hinstance)
{
	HWND hWnd = CreateDialog(hinstance,MAKEINTRESOURCE(IDD_DIALOG_TOOLBOX),parentWindow,BasicWindowProcDummy);
	if(!hWnd)
	{
		return 0;
	}
	ShowWindow(hWnd,SW_SHOW);
	UpdateWindow(hWnd);
	return hWnd;
}
