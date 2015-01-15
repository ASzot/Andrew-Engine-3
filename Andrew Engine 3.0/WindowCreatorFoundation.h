//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#pragma once

#include <Windows.h>
#include "ConstantsList.h"
#include <vector>
#include "resource.h"
#include <string>
#include <sstream>
#include <CommCtrl.h>
#include <WindowsX.h>
#include "StaticObject.h"
#include "ParticleSystem.h"
#include <float.h>
#include <cmath>

#pragma warning(disable : 4505)
namespace WCF
{
	namespace ConversionTools
	{
		static std::wstring StrToWStr( std::string in )
		{
			std::wstring r;
			r.resize( in.size() );
			for ( int i = 0; i < in.size(); ++i )
			{
				r.at(i) = in.at(i);
			}

			return r;
		}

		static std::string WStrToStr( std::wstring in )
		{
			std::string r;
			r.resize( in.size() );
			for ( int i = 0; i < in.size(); ++i )
			{
				r.at(i) = in.at(i);
			}

			return r;
		}

		static std::wstring IntToWStr( int i )
		{
			std::stringstream ss;
			ss << i;
			std::string str = ss.str();
			std::wstring wstr = StrToWStr( str );
			return wstr;
		}

		static inline D3DXVECTOR3 XMFloat3ToD3DVector3(XMFLOAT3 in)
		{
			return D3DXVECTOR3(in.x,in.y,in.z);
		}
		static float DegreesToRadians(float degrees)
		{
			return (degrees * (XM_PI / 180));
		}
		static XMFLOAT3 XMVectorToXMFloat3(XMVECTOR v)
		{
			XMFLOAT3 r;
			r.x = XMVectorGetX(v);
			r.y = XMVectorGetY(v);
			r.z = XMVectorGetZ(v);
			return r;
		}
		static XMVECTOR XMFloat3ToXMVector(XMFLOAT3 f)
		{
			XMVECTOR r = XMVectorSet(f.x,f.y,f.z,0.0f);
			return r;
		}
		static XMFLOAT4 XMVectorToXMFloat4(XMVECTOR v)
		{
			XMFLOAT4 f = XMFLOAT4(XMVectorGetX(v),XMVectorGetY(v),XMVectorGetZ(v),XMVectorGetW(v));
			return f;
		}
		static XMVECTOR XMFloat4ToXMVector(XMFLOAT4 f)
		{
			XMVECTOR v = XMVectorSet(f.x,f.y,f.z,f.w);
			return v;
		}
		static bool XMFloat4Equals(XMFLOAT4 f1,XMFLOAT4 f2)
		{
			if( (f1.x == f1.x) && (f1.y == f2.y) && (f1.z == f2.z))
				return true;
			return false;
		}

		static std::string FloatToString( float in )
		{
			std::stringstream ss( stringstream::in | stringstream::out );
			ss << in;
			return ss.str();
		}
	};

	namespace MathHelper
	{
		const float Infinity = FLT_MAX;

		template<typename T>
		static T Min(const T& a,const T& b)
		{
			return (a < b) ? a : b; 
		}

		template<typename T>
		static T Max(const T& a,const T& b)
		{
			return (a > b) ? a : b;
		}
	};

	class IWindowInformation
	{
	public:
		IWindowInformation(HINSTANCE hInstance,HWND hwnd,UINT idd)
		{
			this->hInstance = hInstance;
			this->hWnd = hwnd;
			this->idd = idd;
		}

		HINSTANCE hInstance;
		HWND hWnd;
		UINT idd;
	};

	namespace Tools
	{
		class ComboBox
		{
		public:
			ComboBox(HINSTANCE hinstance)
			{
				mhInstance = hinstance;
			}
			~ComboBox()
			{

			}

			bool Initialize(int idc, HWND parentWindow,std::vector<std::wstring> text,int startingIndex)
			{
				mhWnd = GetDlgItem( parentWindow, idc );
				for ( std::vector<std::wstring>::iterator i = text.begin(); i != text.end(); ++i )
				{
					LPCWSTR textToAdd = (*i).c_str();
					ComboBox_AddString( mhWnd, textToAdd );
				}

				ComboBox_SetCurSel( mhWnd, startingIndex );
				//ComboBox_SetExtendedUI( mhWnd, CBS_DROPDOWNLIST );

				return true;
			}

			int GetSelection()
			{
				return ComboBox_GetCurSel( mhWnd );
			}

			HWND GetHwnd()
			{
				return mhWnd;
			}
		private:
			HINSTANCE mhInstance;
			HWND mhWnd;
		};
		class Slider
		{
		public:
			Slider(UINT id,HWND mainWnd,int lowerRange,int upperRange)
			{
				m_hWnd = GetDlgItem(mainWnd,id);
				SetRange(lowerRange,upperRange);
			}

			void SetRange(int lowerRange, int upperRange)
			{
				SendMessage(m_hWnd,TBM_SETRANGEMIN,TRUE,lowerRange);
				SendMessage(m_hWnd,TBM_SETRANGEMAX,TRUE,upperRange);
			}
			void SetPosition(int position)
			{
				SendMessage(m_hWnd,TBM_SETPOS,TRUE,position);
			}
			int GetPosition()
			{
				return SendMessage(m_hWnd,TBM_GETPOS,0,0);
			}
		private:
			HWND m_hWnd;
		};

		static void SetDlgItemAsFloat( HWND hwnd, int idDlgItem, float value )
		{
			std::string valueAsString = WCF::ConversionTools::FloatToString( value );
			LPCSTR lpcValue = valueAsString.c_str();
			SetDlgItemTextA( hwnd, idDlgItem, lpcValue );
		}

		static float GetDlgItemAsFloat( HWND hwnd, int idDlgItem )
		{
			// This has 20 digit precision.
			const int precision = 20;
			char buffer[precision];
			GetDlgItemTextA( hwnd, idDlgItem, buffer, precision );

			float finalValue = (float)atof( buffer );

			return finalValue;
		}

		static void SetDlgChecked( HWND hwnd, int idDlgItem, bool checked )
		{
			WPARAM wpChecked;
			checked ? wpChecked = BST_CHECKED : wpChecked = BST_UNCHECKED;
			SendDlgItemMessage( hwnd, idDlgItem, BM_SETCHECK, wpChecked, 0 );
		}

		static bool GetDlgCheck( HWND hwnd, int idDlgItem )
		{
			if ( IsDlgButtonChecked( hwnd, idDlgItem ) == BST_CHECKED )
				return true;
			return false;
		}
	};
	
	// This is just so we can tell what dialog box we have with the global version of ModalessWindow.
	enum DialogBoxType { eDIALOG_BOX_TYPE_LIGHT_PROPERTIES, eDIALOG_BOX_TYPE_OBJECT_PROPERTIES,
		eDIALOG_BOX_TYPE_PARTICLE_PROPERTIES,eDIALOG_BOX_TYPE_BLUR_PROPS, eDIALOG_BOX_TYPE_ANIM_PROPS, eDIALOG_BOX_TYPE_OBJECT_PHYSICS_PROP,
		eDIALOG_BOX_TYPE_TERRAIN, eDIALOG_BOX_TYPE_WATER_SYSTEM_OPTIONS, eDIALOG_BOX_TYPE_GRAPHICS, eDIALOG_BOX_TYPE_KINEMATICS };

	// All message procedures go here:
	BOOL DialogLightPropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL DialogObjectPropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL DialogParticlePropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL DialogAnimPropertiesProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
	BOOL DialogBlurPropDialog(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
	BOOL DialogObjectPhysicsPropProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
	BOOL DialogTerrainProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
	BOOL DialogWaterSystemOptionsProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
	BOOL PropGraphicsProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );
	BOOL DialogKinematicsProc( HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam );

	BOOL CALLBACK ModalWindowProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);
	BOOL CALLBACK HingeConstraintModalWindowProc( HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam );
	BOOL CALLBACK LoadingDlgProc(HWND hwnd,UINT message,WPARAM wParam,LPARAM lParam);

	class ModalWindow
	{
	public:
		char m_pcTextBuffer1[50];
		char m_pcTextBuffer2[50];
		wchar_t m_pwcTextBuffer[50];
		PureFloatData m_FloatData;
		int m_NumberBuffer;
		
		bool m_bIsWindow;
		HWND m_hWindow;

		//Limited support
		ModalWindow();
		virtual ~ModalWindow()
		{
		}

		virtual INT_PTR RunWindow(HINSTANCE hInstance, HWND hwnd, LPCTSTR idd,DLGPROC dialogProc = ModalWindowProc)
		{
			return DialogBox(hInstance,idd,hwnd,dialogProc);
		}

		void QuitDialog()
		{
			m_bIsWindow = false;
			// The exit value doesn't matter as we won't be checking it. 
			assert( m_hWindow );
			EndDialog( m_hWindow, 0 );
		}
	};

	// Yes this needed its own independent class
	class LoadingBox
	{
	private:
		HWND m_hCurrentWindow;
		int m_iNumSteps;

	public:

		bool m_bIsWindow;
		HWND m_hProgressBar;

		LoadingBox();

		~LoadingBox() {}

		BOOL RunWindow(HINSTANCE hInstance, HWND parentWindow, int numberOfStep)
		{
			m_hCurrentWindow = CreateDialog( hInstance, MAKEINTRESOURCE(IDD_DIALOG_LOADING), parentWindow, LoadingDlgProc );
			if ( !m_hCurrentWindow )
				return FALSE;

			m_hProgressBar = GetDlgItem( m_hCurrentWindow, IDC_PROGRESS_LOADING );
			if ( !m_hProgressBar )
				return FALSE;

			// To compensate for the one at the bottom.
			SendMessage( m_hProgressBar, PBM_SETRANGE, 0, MAKELPARAM(0, numberOfStep + 1 ) );
			SendMessage( m_hProgressBar, PBM_SETSTEP, (WPARAM)1, 0 );

			m_iNumSteps = numberOfStep;

			ShowWindow( m_hCurrentWindow, SW_SHOW );
			UpdateWindow( m_hCurrentWindow );


			// This one is not included in the parameter. 
			SendMessage( m_hProgressBar, PBM_STEPIT, 0, 0 );
			SendMessage( m_hProgressBar, PBM_SETBARCOLOR, 0, COLORREF(0x00cd00) );


			m_bIsWindow = true;

			return TRUE;
		}

		void QuitWindow()
		{
			m_bIsWindow = false;
			EndDialog( m_hCurrentWindow, 0 );
		}

		void StepDialog()
		{
			if ( m_iNumSteps != 0 )
			{
				--m_iNumSteps;
				SendMessage( m_hProgressBar, PBM_STEPIT, 0, 0 );
			}
		}
	};

	class ModalessWindow
	{
	public:
		ModalessWindow(DialogBoxType type);



		char* GetWindowText(int index);

		friend BOOL CALLBACK fnBWPDummy(HWND hwnd, UINT message,WPARAM wparam, LPARAM lparam);
		char mTextBuffer[NUMBER_OF_TEXT_BUFFER_INDEXS][TEXT_BUFFER_SIZE];
		HWND mCurrentWindow;

		virtual bool WasExitButtonPressed() = 0;

		bool mIsWindow;
		bool mHasChanged;
		int mNumberBuffer;
		UINT mRecentButton;
		PureFloatData m_FloatData;

		virtual void DestroyWindow() = 0;

		std::vector<char*> mMessages;


	protected:
		BOOL CreateModalessWindow(UINT idd, HINSTANCE hInstance, HWND parentWindow, DLGPROC windowProc = fnBWPDummy);
		virtual BOOL AdditionalSetup(HWND hwnd);	

	private:
		BOOL DefMessageProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
		DialogBoxType mDialogBoxType;
	};

	class KinematicsWindow : public ModalessWindow
	{
	public:

		KinematicsWindow() : WCF::ModalessWindow( eDIALOG_BOX_TYPE_KINEMATICS )
		{
			m_Constraint = 0;
		}

		BOOL RunWindow( HINSTANCE hInstance, HWND parentWindow, KinematicsInfo ki )
		{
			m_Info = ki;
			m_Constraint = ki.Constraint;
			return CreateModalessWindow( IDD_DIALOG_KINEMATICS, hInstance, parentWindow );
		}

		virtual bool WasExitButtonPressed()
		{
			if(mRecentButton == IDC_BUTTON_K_OK)
			{
				mRecentButton = 0;
				return true;
			}
			
			// While we are at it check if a new constraint window was opened.
			else if ( mRecentButton == IDC_BUTTON_K_ADD_CONSTRAINT )
			{
				mRecentButton = 0;
				m_Info.HasConstraint = true;
				BulletConstraintType type = (BulletConstraintType)mNumberBuffer;
				if ( type == BulletConstraintType::HingeType )
				{
					HingeBulletConstraint* bcHinge = new HingeBulletConstraint();

					bcHinge->m_Axis			= Vec( m_FloatData.x1, m_FloatData.y1, m_FloatData.z1 );
					bcHinge->m_AxisPos		= Vec( m_FloatData.x2, m_FloatData.y2, m_FloatData.z2 );
					bcHinge->m_Limits		= Vec( m_FloatData.x3, m_FloatData.y3, m_FloatData.z3 );
					bcHinge->m_Type			= type;
					bcHinge->m_TargetedObj	= m_Info.SelectedObject;

					m_Constraint = bcHinge;

					EnableWindow( GetDlgItem( mCurrentWindow, IDC_BUTTON_K_ADD_CONSTRAINT ), FALSE );
					EnableWindow( GetDlgItem( mCurrentWindow, IDC_BUTTON_K_EDIT_CONSTRAINT ), TRUE );
					EnableWindow( GetDlgItem( mCurrentWindow, IDC_BUTTON_K_DELETE_CONSTRAINT ), TRUE );
				}
			}
			else if ( mRecentButton == IDC_BUTTON_K_EDIT_CONSTRAINT )
			{
				mRecentButton = 0;
				BulletConstraintType type = (BulletConstraintType)mNumberBuffer;
				if ( type == BulletConstraintType::HingeType )
				{
					// Delete the previous constraint.
					SafeDelete( m_Constraint );

					HingeBulletConstraint* bcHinge = new HingeBulletConstraint();

					bcHinge->m_Axis			= Vec( m_FloatData.x1, m_FloatData.y1, m_FloatData.z1 );
					bcHinge->m_AxisPos		= Vec( m_FloatData.x2, m_FloatData.y2, m_FloatData.z2 );
					bcHinge->m_Limits		= Vec( m_FloatData.x3, m_FloatData.y3, m_FloatData.z3 );
					bcHinge->m_Type			= type;
					bcHinge->m_TargetedObj	= m_Info.SelectedObject;

					m_Constraint = bcHinge;
				}
			}
			else if ( mRecentButton == IDC_BUTTON_K_DELETE_CONSTRAINT )
			{
				mRecentButton = 0;
				// Delete the constraint.
				m_Info.HasConstraint = false;
				m_Constraint = 0;

				EnableWindow( GetDlgItem( mCurrentWindow, IDC_BUTTON_K_ADD_CONSTRAINT ), TRUE );
				EnableWindow( GetDlgItem( mCurrentWindow, IDC_BUTTON_K_EDIT_CONSTRAINT ), FALSE );
				EnableWindow( GetDlgItem( mCurrentWindow, IDC_BUTTON_K_DELETE_CONSTRAINT ), FALSE );
				//SafeDelete( m_Constraint );
			}

			return false;
		}

		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

		BOOL AdditionalSetup(HWND hwnd)
		{
			
			if ( m_Info.HasConstraint )
			{
				EnableWindow( GetDlgItem( hwnd, IDC_BUTTON_K_ADD_CONSTRAINT ), FALSE );
			}
			else
			{
				EnableWindow( GetDlgItem( hwnd, IDC_BUTTON_K_DELETE_CONSTRAINT ), FALSE );
				EnableWindow( GetDlgItem( hwnd, IDC_BUTTON_K_EDIT_CONSTRAINT ), FALSE );
			}

			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_K_TORQUE, m_Info.AppliedTorque );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_K_FORCE, m_Info.AppliedForce );

			// Angular Velocity.
			WCF::Tools::SetDlgItemAsFloat( hwnd,IDC_EDIT_K_ANGL_VEL_X, m_Info.AngularVelocity.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd,IDC_EDIT_K_ANGL_VEL_Y, m_Info.AngularVelocity.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd,IDC_EDIT_K_ANGL_VEL_Z, m_Info.AngularVelocity.z );

			// Linear velocity.
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_K_LIN_X, m_Info.LinearVelocity.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_K_LIN_Y, m_Info.LinearVelocity.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_K_LIN_Z, m_Info.LinearVelocity.z );

			UpdateWindow(hwnd);

			return TRUE;
		}

		KinematicsInfo GetWindowData()
		{
			KinematicsInfo ki;
			ki.AppliedForce		= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_FORCE );
			ki.AppliedTorque	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_TORQUE );
			ki.SelectedObject	= m_Info.SelectedObject;
			ki.HasConstraint	= ( m_Constraint != 0 );

			// Angular Velocity.
			ki.AngularVelocity.x = ( WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_ANGL_VEL_X ) );
			ki.AngularVelocity.y = ( WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_ANGL_VEL_Y ) );
			ki.AngularVelocity.z = ( WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_ANGL_VEL_Z ) );

			// Linear Velocity.
			ki.LinearVelocity.x = ( WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_LIN_X ) );
			ki.LinearVelocity.y = ( WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_LIN_Y ) );
			ki.LinearVelocity.z = ( WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_K_LIN_Z ) );

			return ki;
		}

		BulletConstraint* GetConstraint()
		{
			return m_Constraint;
		}

		void ResetConstraint()
		{
			m_Constraint = 0;
		}

		// The constraints must be created using the new keyword as that is how they are disposed of.
		BulletConstraint* m_Constraint;

	private:
		KinematicsInfo m_Info;
		
	};

	class LightPropDialog : public ModalessWindow
	{
	public:
		LightPropDialog() : WCF::ModalessWindow(eDIALOG_BOX_TYPE_LIGHT_PROPERTIES)
 		{

		}
		BOOL RunWindow(UINT idd, HINSTANCE hInstance, HWND parentWindow, float x, float y, float z, float range, XMFLOAT3 att, XMFLOAT3 diff, XMFLOAT3 spec, XMFLOAT3 amb, XMFLOAT3 lookAt, DLGPROC windowProc = fnBWPDummy)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->range = range;
			this->diff = diff;
			this->att = att;
			this->spec = spec;
			this->amb = amb;
			this->lookAt = lookAt;

			return CreateModalessWindow(idd,hInstance,parentWindow,windowProc);
		}

		virtual BOOL AdditionalSetup(HWND hwnd)
		{
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_LIGHT_RANGE, range );

			// Position.
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_POSITION, x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_POS_Y, y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_POS_Z, z );

			// Attenuation.
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_ATT_X, att.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_ATT_Y, att.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_ATT_Z, att.z );

			// Diffuse.
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_DIFF_X, diff.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_DIFF_Y, diff.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_DIFF_Z, diff.z );
			
			// Specular.
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_SPEC_X, spec.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_SPEC_Y, spec.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_SPEC_Z, spec.z );

			// Ambient.
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_AMBIENT_X, amb.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_AMBIENT_Y, amb.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_AMBIENT_Z, amb.z );

			if ( lookAt.x != 696969 )
			{
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_LOOK_X, lookAt.x );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_LOOK_Y, lookAt.y );
				WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_LP_LOOK_Z, lookAt.z );
			}

			UpdateWindow(mCurrentWindow);
			return TRUE;
		}

		virtual bool WasExitButtonPressed()
		{
			if(mRecentButton == IDC_BUTTON_LP_EXIT)
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}

		float* GetSpecifiedPosAsFloat()
		{
			float numbers[18];
			numbers[0]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_POSITION);
			numbers[1]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_POS_Y);
			numbers[2]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_POS_Z);
			numbers[3]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_ATT_X);
			numbers[4]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_ATT_Y);
			numbers[5]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_ATT_Z);
			numbers[6]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_DIFF_X);
			numbers[7]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_DIFF_Y);
			numbers[8]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_DIFF_Z);
			
			numbers[9]	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_LOOK_X);
			numbers[10] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_LOOK_Y);
			numbers[11] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_LOOK_Z);

			numbers[12] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_SPEC_X);
			numbers[13] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_SPEC_Y);
			numbers[14] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_SPEC_Z);

			numbers[15] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_AMBIENT_X);
			numbers[16] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_AMBIENT_Y);
			numbers[17] = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_AMBIENT_Z);
			
			return numbers;
		}

		float GetRangeTextbox()
		{
			return WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_LP_LIGHT_RANGE);
		}

		
		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

	private:
		float x,y,z,range;
		XMFLOAT3 diff,att, lookAt, spec, amb;
	};

	class ObjectPropDialog : public ModalessWindow
	{
	public:
		ObjectPropDialog() 
			: WCF::ModalessWindow(eDIALOG_BOX_TYPE_OBJECT_PROPERTIES),
			m_ComboBox( 0 )
		{
		}
		~ObjectPropDialog()
		{
		}
		BOOL RunWindow(HINSTANCE hInstance,HWND parentWindow,StaticObjectData data,RenderTech renderTech,RenderTechData rtd)
		{
			mStartingIndex = (int)renderTech;
			mData = data;
			mRenderTechData = rtd;
			return CreateModalessWindow(IDD_DIALOG_OBJECT_PROPERTIES,hInstance,parentWindow);
		}

		virtual BOOL AdditionalSetup(HWND hwnd)
		{
			m_ComboBox = WCF::Tools::ComboBox( mhInstance );
			std::vector<std::wstring> textList;
			textList.resize(2);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_POS_X,(UINT)mData.x,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_POS_Y,(UINT)mData.y,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_POS_Z,(UINT)mData.z,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_ROT_X,(UINT)mData.rx,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_ROT_Y,(UINT)mData.ry,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_ROT_Z,(UINT)mData.rz,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_SCALE,(UINT)mData.s,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_SCALE_Y,(UINT)mData.sy,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_SCALE_Z,(UINT)mData.sz,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_TEXTURE_X,(UINT)mData.tx,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_TEXTURE_Y,(UINT)mData.ty,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_TEXTURE_Z,(UINT)mData.tz,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_TEXTURE_TX,(UINT)mData.ttx,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_TEXTURE_TY,(UINT)mData.tty,TRUE);
			SetDlgItemInt(hwnd,IDC_EDIT_OP_TEXTURE_TZ,(UINT)mData.ttz,TRUE);

			// Set the check boxes
			WPARAM checked;
			mRenderTechData.Reflections ? checked = BST_CHECKED : checked = BST_UNCHECKED;
			SendDlgItemMessage( hwnd, IDC_CHECK_OP_REFLECTION, BM_SETCHECK, checked, 0 );
			
			textList[0] = L"Basic";
			textList[1] = L"No Dir Light";
			if(!m_ComboBox.Initialize(IDC_COMBO_OP_RENDER_TECH,hwnd,textList,mStartingIndex))
				return FALSE;

			m_Slider = new Tools::Slider(IDC_SLIDER_OP_OPACITY,hwnd,0,100);
			m_Slider->SetPosition(mData.opacity);

			UpdateWindow(hwnd);

			return TRUE;
		}

		float* GetObjectData()
		{
			float numbers[16];
			numbers[0] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_POS_X,0,TRUE);
			numbers[1] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_POS_Y,0,TRUE);
			numbers[2] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_POS_Z,0,TRUE);
			numbers[3] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_ROT_X,0,TRUE);
			numbers[4] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_ROT_Y,0,TRUE);
			numbers[5] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_ROT_Z,0,TRUE);
			numbers[6] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_SCALE,0,TRUE);
			numbers[7] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_SCALE_Y,0,TRUE);
			numbers[8] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_SCALE_Z,0,TRUE);
			numbers[9] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_TEXTURE_X,0,TRUE);
			numbers[10] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_TEXTURE_Y,0,TRUE);
			numbers[11] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_TEXTURE_Z,0,TRUE);
			numbers[12] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_TEXTURE_TX,0,TRUE);
			numbers[13] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_TEXTURE_TY,0,TRUE);
			numbers[14] = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OP_TEXTURE_TZ,0,TRUE);
			numbers[15] = m_Slider->GetPosition();
			

			return numbers;
		}

		RenderTechData GetRenderTechData()
		{
			RenderTechData rtd;
			( IsDlgButtonChecked( mCurrentWindow, IDC_CHECK_OP_REFLECTION ) == BST_CHECKED ) ? rtd.Reflections = true : rtd.Reflections = false;

			rtd.Refractable = false;

			return rtd;
		}

		RenderTech GetRenderTech(RenderTech renderTech)
		{
			int index = m_ComboBox.GetSelection();
			RenderTech rt = ( RenderTech )index;
			
			return rt;
		}
		
		virtual bool WasExitButtonPressed()
		{
			if(mRecentButton == IDC_BUTTON_OP_OK)
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}

		
		void DestroyWindow()
		{
			SAFE_DELETE(m_Slider);
			EndDialog(mCurrentWindow,0);
		}

	private:
		HINSTANCE mhInstance;
		StaticObjectData mData;
		RenderTechData mRenderTechData;
		int mStartingIndex;
		Tools::Slider* m_Slider;
		Tools::ComboBox m_ComboBox;
	};

	class ParticlePropDialog : public ModalessWindow
	{
	public:
		struct ParticleData
		{
			PositionData PosData;
			//More data for the window to display.
		};
		ParticlePropDialog()
			: ModalessWindow(eDIALOG_BOX_TYPE_PARTICLE_PROPERTIES), m_ComboBox( DXUTGetHINSTANCE() )
		{

		}
		BOOL RunWindow(HINSTANCE hInstance,HWND parentWindow,ParticleSystemData data)
		{
			mData = data;
			return CreateModalessWindow(IDD_DIALOG_PARTICLE_PROPERTIES,hInstance,parentWindow);
		}
		virtual bool WasExitButtonPressed()
		{
			if(mRecentButton == IDC_BUTTON_PP_OK)
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}

		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

		BOOL AdditionalSetup(HWND hwnd)
		{
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_POS_X, mData.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_POS_Y, mData.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_POS_Z, mData.z );

			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_EMIT_X, mData.dx );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_EMIT_Y, mData.dy );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_EMIT_Z, mData.dz );

			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_PACCEL_X, mData.pax );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_PACCEL_Y, mData.pay );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_PACCEL_Z, mData.paz );

			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_TEXTURE_SIZE, mData.TextureSize );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_MAX_PARTICLES, mData.MaxParticles );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_VEL, mData.InitVelocity );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_X_MOD, mData.RandomXMod );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_Z_MOD, mData.RandomZMod );

			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_PP_EMIT_TIME, mData.EmitTime );

			std::vector<std::wstring> text(2);
			text[0] = L"fire";
			text[1] = L"spark";

			int selectedIndex;

			if ( mData.code == "fire" )
				selectedIndex = 0;
			else if ( mData.code == "spark" )
				selectedIndex = 1;

			m_ComboBox.Initialize( IDC_COMBO_PP_SYSTEM_TYPE, hwnd, text, selectedIndex );

			UpdateWindow(hwnd);

			return TRUE;
		}

		ParticleSystemData GetWindowData()
		{
			ParticleSystemData d;
			d.dx = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_EMIT_X );
			d.dy = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_EMIT_Y );
			d.dz = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_EMIT_Z );
			
			d.x = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_POS_X );
			d.y = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_POS_Y );
			d.z = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_POS_Z );

			d.pax = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_PACCEL_X );
			d.pay = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_PACCEL_Y );
			d.paz = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_PACCEL_Z );

			d.MaxParticles = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_MAX_PARTICLES );
			d.TextureSize = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_TEXTURE_SIZE );
			d.InitVelocity = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_VEL );
			d.RandomXMod = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_X_MOD );
			d.RandomZMod = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_Z_MOD );

			d.EmitTime = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_PP_EMIT_TIME );

			int selectedIndex = m_ComboBox.GetSelection();
			if ( selectedIndex == 0 )
				d.code = "fire";
			else if ( selectedIndex == 1 )
				d.code = "spark";

			return d;
		}

	private:
		ParticleSystemData mData;
		WCF::Tools::ComboBox m_ComboBox;
	};

	class ObjectPhysicsPropDialog : public ModalessWindow
	{
	private:
		ObjectPhysicsPropData mData;

	public:
		ObjectPhysicsPropDialog()
			: ModalessWindow(eDIALOG_BOX_TYPE_OBJECT_PHYSICS_PROP)
		{

		}

		BOOL RunWindow(HINSTANCE hInstance, HWND parentWindow, ObjectPhysicsPropData data)
		{
			mData = data;
			return CreateModalessWindow( IDD_DIALOG_OBJECT_PHYSICS_PROPERTIES, hInstance, parentWindow );
		}

		virtual bool WasExitButtonPressed()
		{
			if ( mRecentButton == IDC_BUTTON_OPP_EXIT )
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}

		BOOL AdditionalSetup(HWND hwnd)
		{
			SetDlgItemInt( hwnd, IDC_EDIT_OPP_SPEC_GRAV, (UINT)mData.SpecGrav, TRUE );

			// Set the gravity controls.
			SetDlgItemInt( hwnd, IDC_EDIT_OPP_GRAV_X, (UINT)mData.GravityX, TRUE );
			SetDlgItemInt( hwnd, IDC_EDIT_OPP_GRAV_Y, (UINT)mData.GravityY, TRUE );
			SetDlgItemInt( hwnd, IDC_EDIT_OPP_GRAV_Z, (UINT)mData.GravityZ, TRUE );

			HWND matComboHWnd = GetDlgItem( hwnd, IDC_COMBO_OPP_MATERIAL );
			std::wstring temp;
			// There is definitely a more graceful way to do this but whatever. 
			temp = L"Playdough";
			ComboBox_AddString( matComboHWnd, temp.c_str() );
			temp = L"Normal";
			ComboBox_AddString( matComboHWnd, temp.c_str() );
			temp = L"Bouncy";
			ComboBox_AddString( matComboHWnd, temp.c_str() );
			temp = L"Slippery";
			ComboBox_AddString( matComboHWnd, temp.c_str() );


			ComboBox_SetCurSel( matComboHWnd, mData.Material );

			UpdateWindow(hwnd);

			return TRUE;
		}

		// This should probaly just be inherited across all children of ModalessWindow
		// but whatever.
		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

		ObjectPhysicsPropData GetWindowData()
		{
			ObjectPhysicsPropData r;
			r.SpecGrav = (float)(int)GetDlgItemInt(mCurrentWindow,IDC_EDIT_OPP_SPEC_GRAV,0,TRUE);
			r.GravityX = (float)(int)GetDlgItemInt( mCurrentWindow, IDC_EDIT_OPP_GRAV_X, 0, TRUE );
			r.GravityY = (float)(int)GetDlgItemInt( mCurrentWindow, IDC_EDIT_OPP_GRAV_Y, 0, TRUE );
			r.GravityZ = (float)(int)GetDlgItemInt( mCurrentWindow, IDC_EDIT_OPP_GRAV_Z, 0, TRUE );

			HWND matComboHWnd = GetDlgItem( mCurrentWindow, IDC_COMBO_OPP_MATERIAL );
			// Just realized that we already have a class for this stuff... too late.
			r.Material = ComboBox_GetCurSel( matComboHWnd );

			return r;
		}
	};

	class TerrainDialog : public ModalessWindow
	{
	private:
		TerrainDlgData mData;

	public:
		TerrainDialog()
			: ModalessWindow( eDIALOG_BOX_TYPE_TERRAIN )
		{

		}

		BOOL RunWindow(HINSTANCE hInstance, HWND parentWindow, TerrainDlgData data)
		{
			mData = data;
			return CreateModalessWindow( IDD_DIALOG_TERRAIN_MGR, hInstance, parentWindow );
		}

		virtual bool WasExitButtonPressed()
		{
			if ( mRecentButton == IDC_BUTTON_TM_OK )
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}

		BOOL AdditionalSetup(HWND hwnd)
		{
			std::string frictionAsString = WCF::ConversionTools::FloatToString( mData.frictionOfTerrain );
			const char* frictionAsCharArray = frictionAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_TM_TERRAIN_FRICT, frictionAsCharArray );
			SetDlgItemTextA( hwnd, IDC_EDIT_TM_HEIGHTMAP_FILENAME, mData.heightmapFilename.c_str() );
			SetDlgItemTextA( hwnd, IDC_EDIT_TM_BLENDMAP_FILENAME, mData.blendmapFilename.c_str() );

			UpdateWindow(hwnd);

			return TRUE;
		}

		// This should probaly just be inherited across all children of ModalessWindow
		// but whatever.
		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

		TerrainDlgData GetWindowData()
		{
			const int numberBufferMax = 10;
			const int bufferMax = 60;
			TerrainDlgData tdd;


			char blendMapFilename[bufferMax];
			GetDlgItemTextA( mCurrentWindow, IDC_EDIT_TM_BLENDMAP_FILENAME, blendMapFilename, bufferMax );
			tdd.blendmapFilename = std::string( blendMapFilename );

			char heightMapFilename[bufferMax];
			GetDlgItemTextA( mCurrentWindow, IDC_EDIT_TM_HEIGHTMAP_FILENAME, heightMapFilename, bufferMax );
			tdd.heightmapFilename = std::string( heightMapFilename );

			char terrainFrict[numberBufferMax];
			GetDlgItemTextA( mCurrentWindow, IDC_EDIT_TM_TERRAIN_FRICT, terrainFrict, numberBufferMax );
			float fTerrainFrict = (float)atof( terrainFrict );
			tdd.frictionOfTerrain = fTerrainFrict;

			return tdd;
		}
	};

	class WaterSystemOptionDialog : public ModalessWindow
	{
	private:
		WaterSystemOptions mData;

	public:
		WaterSystemOptionDialog()
			: ModalessWindow( eDIALOG_BOX_TYPE_WATER_SYSTEM_OPTIONS )
		{

		}

		BOOL RunWindow(HINSTANCE hInstance, HWND parentWindow, WaterSystemOptions data)
		{
			mData = data;
			return CreateModalessWindow( IDD_DIALOG_WATER_SYSTEM_OPTIONS, hInstance, parentWindow );
		}

		virtual bool WasExitButtonPressed()
		{
			if ( mRecentButton == IDC_BUTTON_WSO_OK )
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}

		BOOL AdditionalSetup(HWND hwnd)
		{
			// So far we only support one targeted object.
			if ( !mData.TargetedObjectsForRefraction.empty() )
				SetDlgItemInt( hwnd, IDC_EDIT_WSO_TOFR, (UINT)mData.TargetedObjectsForRefraction.at( 0 ), TRUE );

			std::string posXAsString = WCF::ConversionTools::FloatToString( mData.x );
			std::string posYAsString = WCF::ConversionTools::FloatToString( mData.y );
			std::string posZAsString = WCF::ConversionTools::FloatToString( mData.z );
			std::string rotXAsString = WCF::ConversionTools::FloatToString( mData.rx );
			std::string rotYAsString = WCF::ConversionTools::FloatToString( mData.ry );
			std::string rotZAsString = WCF::ConversionTools::FloatToString( mData.rz );
			std::string scaleAsString = WCF::ConversionTools::FloatToString( mData.sx );
			std::string scaleYAsString = WCF::ConversionTools::FloatToString( mData.sy );
			std::string scaleZAsString = WCF::ConversionTools::FloatToString( mData.sz );

			std::string waterScaleAsString = WCF::ConversionTools::FloatToString( mData.WaterScale );
			std::string waterTexTransAsStrign = WCF::ConversionTools::FloatToString( mData.TexTrans );

			LPCSTR lpcX = posXAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_X, lpcX );
			LPCSTR lpcY = posYAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_Y, lpcY );
			LPCSTR lpcZ = posZAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_Z, lpcZ );
			LPCSTR lpcRX = rotXAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_RX, lpcRX );
			LPCSTR lpcRY = rotYAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_RY, lpcRY );
			LPCSTR lpcRZ = rotZAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_RZ, lpcRZ );
			LPCSTR lpcS = scaleAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_SCALE, lpcS );
			LPCSTR lpcSZ = scaleZAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_SCALE_Z, lpcSZ );

			LPCSTR lpcWaterScale = waterScaleAsString.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_WATER_SCALE, lpcWaterScale );
			LPCSTR lpcWaterTexTrans = waterTexTransAsStrign.c_str();
			SetDlgItemTextA( hwnd, IDC_EDIT_WSO_WATER_TEX_TRANS_RATE, lpcWaterTexTrans );

			WCF::Tools::SetDlgChecked( hwnd, IDC_CHECK_WSO_USE_CIRCLE, mData.UseCircle );

			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_WSO_DIFF_AB, mData.DiffuseAbsorbity );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_WSO_SPEC_AB, mData.SpecularAbsorbity );

			UpdateWindow(hwnd);

			return TRUE;
		}

		// This should probaly just be inherited across all children of ModalessWindow
		// but whatever.
		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

		WaterSystemOptions GetWindowData()
		{
			WaterSystemOptions wso;

			int targetedObject = (int)GetDlgItemInt( mCurrentWindow, IDC_EDIT_WSO_TOFR, 0, TRUE );
			wso.TargetedObjectsForRefraction.push_back( targetedObject );

			wso.x = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_X );
			wso.y = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_Y );
			wso.z = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_Z );
			wso.rx = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_RX );
			wso.ry = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_RY );
			wso.rz = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_RZ );
			wso.sx = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_SCALE );
			wso.sz = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_SCALE_Z );
			wso.DiffuseAbsorbity = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_DIFF_AB );
			wso.SpecularAbsorbity = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_SPEC_AB );

			wso.TexTrans = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_WATER_TEX_TRANS_RATE );
			wso.WaterScale = WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_WSO_WATER_SCALE );
			wso.UseCircle = WCF::Tools::GetDlgCheck( mCurrentWindow, IDC_CHECK_WSO_USE_CIRCLE );

			return wso;
		}
	};

	class GraphicsPropDialog : public ModalessWindow
	{
	private:
		GraphicsInfo mData;

	public:
		GraphicsPropDialog()
			: ModalessWindow( eDIALOG_BOX_TYPE_GRAPHICS )
		{

		}

		BOOL RunWindow(HINSTANCE hInstance, HWND parentWindow, GraphicsInfo data)
		{
			mData = data;
			return CreateModalessWindow( IDD_DIALOG_GRAPHICS_PROPERTIES, hInstance, parentWindow );
		}

		virtual bool WasExitButtonPressed()
		{
			if ( mRecentButton == IDC_BUTTON_G_OK )
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}

		BOOL AdditionalSetup(HWND hwnd)
		{
			WCF::Tools::SetDlgChecked( hwnd, IDC_CHECK_USE_SKYMAP, mData.UseSkymap );
			WCF::Tools::SetDlgChecked( hwnd, IDC_CHECK_FLASH_SEL, mData.Flash );

			UpdateWindow(hwnd);

			return TRUE;
		}

		// This should probaly just be inherited across all children of ModalessWindow
		// but whatever.
		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

		GraphicsInfo GetWindowData()
		{
			GraphicsInfo data;

			data.UseSkymap = WCF::Tools::GetDlgCheck( mCurrentWindow, IDC_CHECK_USE_SKYMAP );
			data.Flash = WCF::Tools::GetDlgCheck( mCurrentWindow, IDC_CHECK_FLASH_SEL );

			return data;
		}
	};

	class AnimPropDialog : public ModalessWindow
	{
	public:
		struct AnimPropData
		{
			PositionData Data;
			float TS;
			float Speed;
			int Selected;
			float APH;
		};

		AnimPropDialog()
			: ModalessWindow(eDIALOG_BOX_TYPE_ANIM_PROPS)
		{

		}
		BOOL RunWindow(HINSTANCE hInstance,HWND parentWindow,AnimPropData data)
		{
			mData = data;
			return CreateModalessWindow(IDD_DIALOG_ANIM_PREFERENCES,hInstance,parentWindow);
		}
		virtual bool WasExitButtonPressed()
		{
			if(mRecentButton == IDC_BUTTON_AP_OK)
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}
		BOOL AdditionalSetup(HWND hwnd)
		{
			//All position fields.
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_X, mData.Data.x );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_Y, mData.Data.y );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_Z, mData.Data.z );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_RX, mData.Data.rx );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_RY, mData.Data.ry );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_RZ, mData.Data.rz );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_S, mData.Data.s );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_TS, mData.TS );

			bool useSYZ = ( mData.Data.sy != 0.0f );
			float sy = useSYZ ? mData.Data.sy : mData.Data.s;
			float sz = useSYZ ? mData.Data.sz : mData.Data.s;
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_SX, mData.Data.sy );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_SZ, mData.Data.sz );


			//All other fields
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_ANIM_SPEED, mData.Speed );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_SELECTED_OBJECT, mData.Selected );
			WCF::Tools::SetDlgItemAsFloat( hwnd, IDC_EDIT_AP_APH, mData.APH );

			UpdateWindow(hwnd);
			return TRUE;
		}
		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}
		AnimPropData GetWindowData()
		{
			AnimPropData data;
			data.Data.x		= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_X );
			data.Data.y		= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_Y );
			data.Data.z		= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_Z );
			data.Data.rx	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_RX );
			data.Data.ry	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_RY );
			data.Data.rz	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_RZ );
			data.Data.s		= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_S );
			data.Data.sy	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_SX );
			data.Data.sz	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_SZ );
			data.TS			= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_TS );

			data.APH		= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_APH );
			data.Selected	= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_SELECTED_OBJECT );
			data.Speed		= WCF::Tools::GetDlgItemAsFloat( mCurrentWindow, IDC_EDIT_AP_ANIM_SPEED );
			
			return data;
		}

	private:
		AnimPropData mData;
	};

	class BlurPropDialog : public ModalessWindow
	{
	public:
		struct BlurData
		{

		};
		BlurPropDialog()
			: ModalessWindow(eDIALOG_BOX_TYPE_BLUR_PROPS)
		{

		}
		BOOL RunWindow(HINSTANCE hInstance, HWND parentWindow,BlurData data)
		{
			mData = data;
			return CreateModalessWindow(IDD_DIALOG_BLUR_PROPERTIES,hInstance,parentWindow);
		}

		void DestroyWindow()
		{
			EndDialog(mCurrentWindow,0);
		}

		bool WasExitButtonPressed()
		{
			if(mRecentButton == IDC_BUTTON_BP_OK)
			{
				mRecentButton = 0;
				return true;
			}
			return false;
		}
		BOOL AdditionalSetup(HWND hwnd)
		{
			return TRUE;
		}

	private:
		BlurData mData;
	};

	BOOL CALLBACK fnBWPDummy(HWND hwnd, UINT message,WPARAM wparam, LPARAM lparam);

};