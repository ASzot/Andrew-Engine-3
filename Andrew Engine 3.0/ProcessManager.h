#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#include "Process.h"
#include <list>
#include <vector>

typedef std::list<CProcess*> ProcessList;
class CProcessManager
{
public:
	CProcessManager();

	void Attach(CProcess* pProcess);

	bool HasProcesses();
	bool IsProcessActive(int nType);

	void UpdateProcesses(int deltaMilliseconds);
	~CProcessManager();
	void Shutdown();

protected:
	ProcessList		m_ProcessList;
	ProcessList		m_DeleteList;

private:
	void Detach(CProcess* pProcess);
};


namespace 
{
	CProcessManager* g_pProcessManager;
}

CProcessManager* g_fGetProcessManager();