#include "Input.h"

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

CInput::CInput()
{

}

CInput::~CInput()
{

}



CHashedString CInput::ProcessInput(UINT nChar, bool bKeyDown, bool bAltDown,GameStates gameState)
{
	if( bKeyDown)
	{
		if(bAltDown)
		{
			return ProcessInputWAltDown(nChar,gameState);
		}
		switch( nChar )
		{
		case VK_F9:
			return CHashedString("c2GameStateQuiting");
			break;
		case VK_F8:
			return CHashedString("c2Fullscreen");
			break;
		case VK_F3:
			return CHashedString("c2GameStatePaused");
			break;
		case VK_F4:
			return CHashedString("c2GameStateResumed");
			break;
		case VK_F6:
			return CHashedString("c2GameStateSavingLevel");
			break;
		case VK_F7:
			return CHashedString("c2GameStateOpeningLevel");
			break;

		// For the movement.
		case VK_SPACE:
			if ( gameState == eRunning )
				return CHashedString( "c1MainPlayerJump" );
			break;


		//For modifying the selected object
		case VK_1:
			return CHashedString("c1EditorAddSelectedObject");
			break;
		case VK_2:
			return CHashedString("c1EditorSubtractSelectedObject");
			break;
		case VK_3:
			return CHashedString("c1EditorAddSelectedLight");
			break;
		case VK_4:
			return CHashedString("c1EditorSubtractSelectedLight");
			break;
		case VK_5:
			return CHashedString("c6IncSelectedWaterSystem");
			break;
		case VK_6:
			return CHashedString("c6DecSelectedWaterSystem");
			break;
		case VK_7:
			return CHashedString( "c5IncSelectedSystem" );
			break;
		case VK_8:
			return CHashedString( "c5DecSelectedSystem" );
			break;


		// For changing the selected manager.
		case VK_NUMPAD1:
			return CHashedString( "c#MgrObject" );
			break;
		case VK_NUMPAD2:
			return CHashedString( "c#MgrLight" );
			break;
		case VK_NUMPAD3:
			return CHashedString( "c#MgrParicle" );
			break;

		case VK_T:
			if ( gameState == eRunning )
				return CHashedString( "c1MainPlayerThrowObject" );
			break;

		case VK_J:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorMoveLeft");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_K:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorMoveBackward");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_L:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorMoveRight");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_I:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorMoveForward");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_O:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorMoveUp");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_M:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorMoveDown");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_Y:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorRotateXPos");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_U:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorRotateXNeg");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_B:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorRotateYPos");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_N:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorRotateYNeg");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_G:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorScalePos");
				r.BypassFilter = true;
				return r;
			}
			else if ( gameState == eRunning )
				return CHashedString( "c1MainPlayerDropObject" );
			break;
		case VK_H:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorScaleNeg");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_C:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorTexScaleNeg");
				r.BypassFilter = true;
				return r;
			}
			break;
		case VK_V:
			if(gameState == eBuildingLevel)
			{
				CHashedString r("c1EditorTexScalePos");
				r.BypassFilter = true;
				return r;
			}
			break;
		// VK_Z and VK_X already taken as well.
		}
	}
	return CHashedString("c3Nothing");
}

CHashedString CInput::ProcessInputWAltDown(UINT nChar, GameStates gameState)
{
	switch(nChar)
	{
	case VK_U:
		if(gameState == eBuildingLevel)
			return CHashedString("c2OpenUserPreferences");
		break;
	case VK_O:
		if(gameState == eBuildingLevel)
			return CHashedString("c1OpenObjectProperties");
		break;

	// Creates the terrain manager window.
	case VK_C:
		if ( gameState == eBuildingLevel )
			return CHashedString( "c8CreateTerrainWindow" );
		break;

	// Creates the water manager window.
	case VK_W:
		if ( gameState == eBuildingLevel )
			return CHashedString( "c6CreateWaterMgrDlg" );
		break;

	// Resets the level.
	case VK_R:
		// This will go straight to CApp so it doesn't matter the type of message.
		return CHashedString("c1ResetLevel");
		break;

	// Creates the graphic properties window.
	case VK_G:
		if ( gameState == eBuildingLevel )
			return CHashedString( "c6CreateGraphicsDlg" );
		break;

	case VK_P:
		if(gameState == eBuildingLevel)
			return CHashedString("c1AddPointLight");
		break;
	case VK_N:
		if(gameState == eBuildingLevel)
			return CHashedString("c5CreateWindow");
		break;
	case VK_T:
		if(gameState == eBuildingLevel)
			return CHashedString("c7CreateWindow");
		break;
	case VK_H:
		if ( gameState == eBuildingLevel )
			return CHashedString( "c1CreateOPPWindow");
		break;

	// Creates the constraints window.
	case VK_K:
		if ( gameState == eBuildingLevel )
			return CHashedString( "c1CreatePhysicsWindow" );
		break;

	case VK_1:
		if ( gameState == eBuildingLevel )
			return CHashedString( "c1UpdatePhysics");
		break;
	default:
		return CHashedString("c3Nothing");
		break;
	}
	return CHashedString("c3Nothing");
}
