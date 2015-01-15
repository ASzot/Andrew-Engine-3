//=======================================================================
// Copyright Andrew Szot 2015.
// Distributed under the MIT License.
// (See accompanying file LICENSE.txt)
//=======================================================================

#ifndef HASHED_STRING_H
#define HASHED_STRING_H

#include <Windows.h>
#include <string>
#include <vector>

// The types of messages dictating where the message system will route messages sent through hashed strings.
enum gEEventTypes { eInputEventType, eSystemEventType, eObjectEventType, eNothingEventType, eErrorEventType, eParticleSystem, eEffectEventType, eAnimEventType, eTerrainEventType, eControlEventType };

class CHashedString
{
public:
	explicit CHashedString(char const * const pIdentStr)
		: mIdent(hash_name(pIdentStr)), mIdentStr(pIdentStr), mEventType(eErrorEventType)
	{
		size_t found = mIdentStr.find("1");
		if(found != std::string::npos)
			mEventType = eInputEventType;
		found = mIdentStr.find("2");
		if(found != std::string::npos)
			mEventType = eSystemEventType;
		found = mIdentStr.find("3");
		if(found != std::string::npos)
			mEventType = eNothingEventType;
		found = mIdentStr.find("4");
		if(found != std::string::npos)
			mEventType = eObjectEventType;
		found = mIdentStr.find("5");
		if(found != std::string::npos)
			mEventType = eParticleSystem;
		found = mIdentStr.find("6");
		if(found != std::string::npos)
			mEventType = eEffectEventType;
		found = mIdentStr.find("7");
		if(found != std::string::npos)
			mEventType = eAnimEventType;
		found = mIdentStr.find("8");
		if(found != std::string::npos)
			mEventType = eTerrainEventType;

		// # is the control character.
		found = mIdentStr.find("#");
		if ( found != std::string::npos ) 
		{
			mEventType = eControlEventType;
		}

		if(mEventType == eErrorEventType)
		{
			MessageBoxA(NULL,"Initialized a hashed string with a invalid identifier","Fatal Error",MB_OK); 
		}

		BypassFilter = false;
	}

	CHashedString()
	{
		BypassFilter = false;
	}

	unsigned long GetIdent() const
	{
		return reinterpret_cast<unsigned int>(mIdent);
	}

	bool operator< (CHashedString const & rhs) const
	{
		bool r = ( GetIdent() < rhs.GetIdent() );
		return r;
	}

	bool operator== (CHashedString const& rhs) const
	{
		bool r = (GetIdent() == rhs.GetIdent());
		return r;
	}

	bool operator== (const char* rhs) const
	{
		bool r = (mIdentStr == rhs);
		return r;
	}

	static void* hash_name(char const* pIdentStr);

	std::string const GetStr() const { return mIdentStr; }

	gEEventTypes GetEventType() const
	{
		return mEventType;
	}

	// If true, this message will bypass all of the filters in our event manager,
	// so this message will go to all listeners.
	bool BypassFilter;

private:
	void* mIdent;
	std::string mIdentStr;
	gEEventTypes mEventType;
};



#endif