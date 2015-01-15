//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "WindowCreatorFoundation.h"
#include "EventManager.h"

// Global instances for easy access in the message procs.
WCF::ModalessWindow* g_ModalessWindow;
WCF::ModalWindow* g_pModalWindow;

namespace 
{
	WCF::LoadingBox* g_pLoadingBox;
};


namespace WCF
{
	// A ton of message procs

	BOOL DialogLightPropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_LP_EXIT:
				if(!GetDlgItemTextA(hwnd,IDC_EDIT_LP_LIGHT_RANGE,g_ModalessWindow->mTextBuffer[0],TEXT_BUFFER_SIZE))
					*(g_ModalessWindow->mTextBuffer[0]) = 0;
				if(!GetDlgItemTextA(hwnd,IDC_EDIT_LP_POSITION,g_ModalessWindow->mTextBuffer[1],TEXT_BUFFER_SIZE))
					*(g_ModalessWindow->mTextBuffer[1]) = 0;
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_LP_EXIT;
				return TRUE;
				break;
			}
		}
		return FALSE;
	}

	BOOL DialogObjectPropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
		case WM_COMMAND:
			
			if(HIWORD(wParam) == CBN_SELCHANGE)
			{
				g_ModalessWindow->mHasChanged = true;

				g_ModalessWindow->mNumberBuffer = SendMessage((HWND)lParam,(UINT)CB_GETCURSEL,(WPARAM)0,(LPARAM)0);

				return TRUE;
				break;
			}
			
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_OP_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_OP_OK;
				return TRUE;
				break;
			case IDC_BUTTON_OP_EXIT:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_OP_EXIT;
				return TRUE;
				break;
			}
		}
		return FALSE;
	}

	BOOL DialogObjectPhysicsPropProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_OPP_EXIT:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_OPP_EXIT;
				return TRUE;
				break;
			}
		}
		return FALSE;
	}

	BOOL DialogParticlePropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_PP_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_PP_OK;
				return TRUE;
				break;
			case IDC_BUTTON_PP_EXIT:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_PP_EXIT;
				return TRUE;
				break;
			case IDC_BUTTON_PP_ADD_SYSTEM:
				g_ModalessWindow->mMessages.push_back( "AddSystem" );
				break;
			case IDC_BUTTON_PP_DELETE_SYSTEM:
				g_ModalessWindow->mMessages.push_back( "DeleteSystem" );
			}
			break;
		}
		return FALSE;
	}

	BOOL DialogBlurPropDialog(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_BP_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_BP_OK;
				return TRUE;
				break;
			case IDC_BUTTON_BP_EXIT:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_BP_EXIT;
				return TRUE;
				break;
			}
			break;
		}
		return FALSE;
	}

	BOOL DialogAnimPropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_AP_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_AP_OK;
				return TRUE;
				break;
			case IDC_BUTTON_AP_EXIT:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_AP_EXIT;
				return TRUE;
				break;
			case IDC_BUTTON_AP_ADD_OBJECT:
				g_ModalessWindow->mMessages.push_back("AddObject");
				break;
			case IDC_BUTTON_AP_DELETE_OBJECT:
				g_ModalessWindow->mMessages.push_back("DeleteObject");
				break;
			}
			break;
		}
		return FALSE;
	}

	BOOL DialogTerrainProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_TM_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_TM_OK;
				return TRUE;
				break;
			}
			break;
		}
		return FALSE;
	}

	BOOL DialogWaterSystemOptionsProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_WSO_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_WSO_OK;
				return TRUE;
				break;
			case IDC_BUTTON_WSO_ADD_SYSTEM:
				g_ModalessWindow->mMessages.push_back( "AddSystem" );
				break;
			case IDC_BUTTON_WSO_DELETE_SYSTEM:
				g_ModalessWindow->mMessages.push_back( "SubSystem" );
				break;
			}
			break;
		}
		return FALSE;
	}

	BOOL PropGraphicsProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_G_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_G_OK;
				return TRUE;
				break;
			}
			break;
		}
		return FALSE;
	}

	BOOL DialogKinematicsProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )
	{
		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_K_OK:
				g_ModalessWindow->mIsWindow = false;
				g_ModalessWindow->mRecentButton = IDC_BUTTON_K_OK;
				return TRUE;
				break;

			case IDC_BUTTON_K_TOGGLE_PHYSICS:
				g_ModalessWindow->mRecentButton = IDC_BUTTON_K_TOGGLE_PHYSICS;
				g_GetEventManager()->SetIsBuilderPhysicsEnabled( !g_GetEventManager()->IsBuilderPhysicsEnabled() );
				break;

			case IDC_BUTTON_K_EDIT_CONSTRAINT:
				{
					g_ModalessWindow->mRecentButton = IDC_BUTTON_K_EDIT_CONSTRAINT;
					ModalWindow dlg;
					WCF::KinematicsWindow* window = (WCF::KinematicsWindow*)g_ModalessWindow;
					BulletConstraint* constraint = window->GetConstraint();
					if ( !constraint )
						DebugWriter::WriteToDebugFile( "Invalid constraint!", "BOOL DialogKinematicsProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam )" );
					BulletConstraintType type = constraint->m_Type;
					switch (type)
					{
					case BulletConstraintType::HingeType:
						// Create the window, it will be populated with information in the window proc.
						ModalWindow dlg;
						INT_PTR result = dlg.RunWindow( DXUTGetHINSTANCE(), hwnd, MAKEINTRESOURCE( IDD_DIALOG_ADD_CONSTRAINT_HINGE ), HingeConstraintModalWindowProc );
						if ( result == IDC_BUTTON_HCO_OK )
						{
							// Get the data from the constraint window.
							g_ModalessWindow->m_FloatData = g_pModalWindow->m_FloatData;
							g_ModalessWindow->mNumberBuffer = g_pModalWindow->m_NumberBuffer;
						}
						break;
					}

					break;
				}

			case IDC_BUTTON_K_ADD_CONSTRAINT:
				{
					g_ModalessWindow->mRecentButton = IDC_BUTTON_K_ADD_CONSTRAINT;
					ModalWindow dlg;
					INT_PTR result = dlg.RunWindow( DXUTGetHINSTANCE(), hwnd, MAKEINTRESOURCE( IDD_DIALOG_ADD_CONSTRAINT_HINGE ) );
					if ( result == IDC_BUTTON_HCO_OK )
					{
						// Get the data from the constraint window.
						g_ModalessWindow->m_FloatData = g_pModalWindow->m_FloatData;
						g_ModalessWindow->mNumberBuffer = g_pModalWindow->m_NumberBuffer;
					}
					break;
				}

			

			case IDC_BUTTON_K_DELETE_CONSTRAINT:
				g_ModalessWindow->mRecentButton = IDC_BUTTON_K_DELETE_CONSTRAINT;
				break;
			}
			break;
		}
		return FALSE;
	}

	BOOL ModalessWindow::CreateModalessWindow(UINT idd, HINSTANCE hInstance, HWND parentWindow, DLGPROC windowProc)
	{
		mCurrentWindow = CreateDialog(hInstance,MAKEINTRESOURCE(idd),parentWindow,windowProc);
		if(!mCurrentWindow)
		{
			MessageBoxA(0,"No current window.","Fatal Error",MB_OK);
			return FALSE;
		}
		ShowWindow(mCurrentWindow,SW_SHOW);
		UpdateWindow(mCurrentWindow);
		if(AdditionalSetup(mCurrentWindow) == FALSE)
			return FALSE;
		mIsWindow = true;
		g_ModalessWindow = this;
		return TRUE;
	}
	
	BOOL ModalessWindow::DefMessageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		switch(mDialogBoxType)
		{
		case eDIALOG_BOX_TYPE_LIGHT_PROPERTIES:
			return DialogLightPropertiesProc(hwnd,message,wParam,lParam);
			break;
		case eDIALOG_BOX_TYPE_OBJECT_PROPERTIES:
			return DialogObjectPropertiesProc(hwnd,message,wParam,lParam);
			break;
		case eDIALOG_BOX_TYPE_PARTICLE_PROPERTIES:
			return DialogParticlePropertiesProc(hwnd,message,wParam,lParam);
			break;
		case eDIALOG_BOX_TYPE_BLUR_PROPS:
			return DialogBlurPropDialog(hwnd,message,wParam,lParam);
			break;
		case eDIALOG_BOX_TYPE_ANIM_PROPS:
			return DialogAnimPropertiesProc(hwnd,message,wParam,lParam);
			break;
		case eDIALOG_BOX_TYPE_OBJECT_PHYSICS_PROP:
			return DialogObjectPhysicsPropProc(hwnd,message,wParam,lParam);
			break;
		case eDIALOG_BOX_TYPE_TERRAIN:
			return DialogTerrainProc( hwnd, message, wParam, lParam );
			break;
		case eDIALOG_BOX_TYPE_WATER_SYSTEM_OPTIONS:
			return DialogWaterSystemOptionsProc( hwnd, message, wParam, lParam );
			break;
		case eDIALOG_BOX_TYPE_GRAPHICS:
			return PropGraphicsProc( hwnd, message, wParam, lParam );
			break;
		case eDIALOG_BOX_TYPE_KINEMATICS:
			return DialogKinematicsProc( hwnd, message, wParam, lParam );
			break;
		default:
			return DefWindowProc(hwnd,message,wParam,lParam);
			break;
		}
	}

	BOOL CALLBACK LoadingDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
	{
		switch( message )
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case WM_CREATE:
				break;
			}
			break;
		}

		return FALSE;
	}
	
	LoadingBox::LoadingBox()
	{
		g_pLoadingBox = this;
		m_bIsWindow = false;
		m_iNumSteps = 0;
	}

	ModalessWindow::ModalessWindow(DialogBoxType type)
	{
		mIsWindow = false;
		mRecentButton = 0;
		mNumberBuffer = 0;
		mHasChanged = false;
		g_ModalessWindow = this;
		mDialogBoxType = type;
	}

	BOOL CALLBACK fnBWPDummy(HWND hwnd, UINT message,WPARAM wparam, LPARAM lparam)
	{
		return g_ModalessWindow->DefMessageProc(hwnd, message, wparam, lparam);
	}

	BOOL ModalessWindow::AdditionalSetup(HWND hwnd)
	{
		return TRUE;
	}

	BOOL CALLBACK HingeConstraintModalWindowProc( HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam )
	{


		switch (message)
		{
		case WM_INITDIALOG:
			{
				// Since I couldn't think of a better way to do this.
				g_pModalWindow->m_hWindow = hwnd;
				// Populate the window with the constraint info.
				WCF::KinematicsWindow* window = (WCF::KinematicsWindow*)g_ModalessWindow;
				HingeBulletConstraint* constraint = (HingeBulletConstraint*)window->GetConstraint();
				assert(constraint);

				// Axis.
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_X, constraint->m_Axis.X() );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Y, constraint->m_Axis.Y() );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Z, constraint->m_Axis.Z() );

				// Axis position.
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_X, constraint->m_AxisPos.X() );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Y, constraint->m_AxisPos.Y() );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Z, constraint->m_AxisPos.Z() );

				// Limits.
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_X, constraint->m_AxisPos.X() );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Y, constraint->m_AxisPos.Y() );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Z, constraint->m_AxisPos.Z() );

				break;
			}
		case WM_COMMAND:
			switch (LOWORD(wParam))
			{
			
			case WM_CLOSE:
				g_pModalWindow->m_bIsWindow = false;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;
			case WM_INITDIALOG:
				{
					// Since I couldn't think of a better way to do this.
					g_pModalWindow->m_hWindow = hwnd;
					// Populate the window with the constraint info.
					WCF::KinematicsWindow* window = (WCF::KinematicsWindow*)g_pModalWindow;
					HingeBulletConstraint* constraint = (HingeBulletConstraint*)window->GetConstraint();
					assert(constraint);

					// Axis.
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_X, constraint->m_Axis.X() );
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Y, constraint->m_Axis.Y() );
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Z, constraint->m_Axis.Z() );

					// Axis position.
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_X, constraint->m_AxisPos.X() );
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Y, constraint->m_AxisPos.Y() );
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Z, constraint->m_AxisPos.Z() );

					// Limits.
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_X, constraint->m_AxisPos.X() );
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Y, constraint->m_AxisPos.Y() );
					WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Z, constraint->m_AxisPos.Z() );

					break;
				}

			case IDC_BUTTON_HCO_OK:
				// Get all of the window data.

				// Axis.
				g_pModalWindow->m_FloatData.x1 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_X );
				g_pModalWindow->m_FloatData.y1 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Y );
				g_pModalWindow->m_FloatData.z1 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Z );

				// Axis position.
				g_pModalWindow->m_FloatData.x2 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_X );
				g_pModalWindow->m_FloatData.y2 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Y );
				g_pModalWindow->m_FloatData.z2 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Z );

				// Limits.
				g_pModalWindow->m_FloatData.x3 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_X );
				g_pModalWindow->m_FloatData.y3 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Y );
				g_pModalWindow->m_FloatData.z3 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Z );

				g_pModalWindow->m_NumberBuffer = (int)( BulletConstraintType::HingeType );

				g_pModalWindow->m_bIsWindow = false;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;

			case IDC_BUTTON_HCO_CANCEL:
				g_pModalWindow->m_bIsWindow = false;
				g_pModalWindow->m_NumberBuffer = -1;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;
			}
		}
	}

	BOOL CALLBACK ModalWindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam)
	{

		switch(message)
		{
		case WM_COMMAND:
			switch(LOWORD(wParam))
			{
			case IDC_BUTTON_AAO_OK:
 				GetDlgItemTextA(hwnd,IDC_EDIT_AAO_OBJECT_FILENAME,g_pModalWindow->m_pcTextBuffer1,50);
				GetDlgItemText(hwnd,IDC_EDIT_AAO_TEXTURE_DIRECTORY,g_pModalWindow->m_pwcTextBuffer,50);
				GetDlgItemTextA(hwnd,IDC_EDIT_AAO_ANIM_CLIP_NAME,g_pModalWindow->m_pcTextBuffer2,50);
				EndDialog(hwnd,wParam);
				return TRUE;
				break;
			case IDC_BUTTON_AAO_EXIT:
				g_pModalWindow->m_bIsWindow = false;
				EndDialog(hwnd,wParam);
				return TRUE;
				break;
			case WM_CREATE:
				// Since I couldn't think of a better way to do this.
				g_pModalWindow->m_hWindow = hwnd;
				break;
			case IDC_BUTTON_AL_ADD_PL:
				g_pModalWindow->m_bIsWindow = false;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;
			case IDC_BUTTON_AL_ADD_SCPL:
				g_pModalWindow->m_bIsWindow = false;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;

			case IDC_BUTTON_HCO_OK:
				// Get all of the window data.

				// Axis.
				g_pModalWindow->m_FloatData.x1 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_X );
				g_pModalWindow->m_FloatData.y1 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Y );
				g_pModalWindow->m_FloatData.z1 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_Z );

				// Axis position.
				g_pModalWindow->m_FloatData.x2 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_X );
				g_pModalWindow->m_FloatData.y2 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Y );
				g_pModalWindow->m_FloatData.z2 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_AXIS_POS_Z );

				// Limits.
				g_pModalWindow->m_FloatData.x3 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_X );
				g_pModalWindow->m_FloatData.y3 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Y );
				g_pModalWindow->m_FloatData.z3 = WCF::Tools::GetDlgItemAsFloat( hwnd, IDC_EDIT_HCO_LIMITS_Z );

				g_pModalWindow->m_NumberBuffer = (int)( BulletConstraintType::HingeType );

				g_pModalWindow->m_bIsWindow = false;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;

			case IDC_BUTTON_HCO_CANCEL:
				g_pModalWindow->m_bIsWindow = false;
				g_pModalWindow->m_NumberBuffer = -1;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;

			
			case WM_CLOSE:
				g_pModalWindow->m_bIsWindow = false;
				EndDialog( hwnd, wParam );
				return TRUE;
				break;
			}
		}
		return FALSE;
	}

	ModalWindow::ModalWindow()
	{
		g_pModalWindow = this;
		m_bIsWindow = true;
		m_hWindow = 0;
	}
};