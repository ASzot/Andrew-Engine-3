//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once


#include <WindowsX.h>
#include <Windows.h>
#include "EventManager.h"
#include "DXUT.h"
#include "DXUTsettingsdlg.h"
#include "DXUTgui.h"
#include "SDKmisc.h"
#include "resource.h"
#include "resource1.h"

//Our menu identifiers
#define ID_FILE_SAVE 9001
#define ID_FILE_OPEN 9002

#define MAX_TEXT_BUFFER_SIZE 80
#define INDEX_FILE_NAME_LOCATION	0
#define INDEX_LEVEL_NAME_LOCATION	1

class CWindowManager
{
public:
	CWindowManager(void);

	~CWindowManager(void);

	// Used to create generic windows style windows
	void CreateDialogWindow(unsigned int dialogResource, HINSTANCE hInstance, HWND parentWindow);

	INT_PTR CreateModalDialogBox(unsigned int dialogResource, HINSTANCE hInstance, HWND parentWindow);

private:
	//Currently this only supports one active window at a time (as it should)
	HWND mCurrentWindow;
	HWND mMainWindow;
	HMENU mMainMenu;
	bool mIsMainWndMenu;
	UINT mMostRecentlyPressedButton;
	char mTextBuffer[2][MAX_TEXT_BUFFER_SIZE];
public:
	// Called by the basic dummy window proc through the global version
	BOOL BasicWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL BasicModalWindowProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL DestroyCurrentWindow(HWND windowHandle);
	// Gets the most recently pressed button
	UINT ProccessWindows(void);
	// Used to create the menu for the main window
	BOOL CreateMainWindowMenu(HWND parentWindow);
	
	BOOL DestroyMainWindowMenu();

	char* GetTextBuffer(int index = INDEX_FILE_NAME_LOCATION);
	// This will be created on entering the builder state
	HWND CreateToolbox(HWND parentWindow,HINSTANCE hinstance);
};

