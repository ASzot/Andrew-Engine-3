#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include <list>

static const int PROCESS_FLAG_ATTACHED			=	0x00000001;

enum PROCESS_TYPE
{
	PROC_NONE,
	PROC_WAIT,
	PROC_SOUNDFX,
	PROC_MUSIC,
	PROC_GRAPHICS_EFFECT,
};


class CProcess
{
protected:
	int m_iType;
	bool m_bKill;
	bool m_bActive;
	bool m_bPaused;
	bool m_bInitialUpdate;
	CProcess* m_pNext;

private:
	unsigned int m_uProcessFlags;

public:
	CProcess(int ntype, unsigned int uOrder = 0);
	virtual ~CProcess(void);

public:
	bool IsDead() const { return m_bKill; }
	
	int GetType() const { return m_iType; }
	void SetType(const int t) { m_iType = t; }

	bool IsActive() const { return m_bActive; }
	void SetActive(const bool b) { m_bActive = b; }
	bool IsAttached()const;
	void SetAttached(const bool wantAttached);

	void DeleteChildren()
	{
		std::list<CProcess*> children;
		CProcess* p = this;
		while ( p->GetNext() )
		{
			children.push_front( p->GetNext() );
			p = p->GetNext();
		}
		// If next has children delete them.
		if ( children.size() != 0)
		{
			for ( std::list<CProcess*>::iterator i = children.begin(); i != children.end(); ++i )
			{
				delete (*i);
				(*i) = 0;
			}
		}
	}

	bool IsPaused() const { return m_bPaused; }

	bool IsInitialized()const { return !m_bInitialUpdate; }

	CProcess* const GetNext() const { return m_pNext; }
	void SetNext(CProcess* next);

	// Methods to overload.
	virtual void VOnUpdate( const int deltaMiliseconds );
	virtual void VOnInitialize() { }
	virtual void VKill();
	virtual void VTogglePause() { m_bPaused = !m_bPaused; }
};

