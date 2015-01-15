#pragma once

//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================


#include <time.h>

class CTimer
{
public:
	CTimer()
	{
		
	}
	double GetDifference()
	{
		time(&end);
		double diff = difftime(end,start);
		return diff;
	}
	~CTimer()
	{

	}
	void Start()
	{
		time(&start);
	}
private:
	time_t start,end;
};