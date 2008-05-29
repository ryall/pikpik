#pragma once

/**
* @file Match.h
* @author Nat Ryall
* @date 27/05/2008
* @brief Network matchmaking routines.
*
* Copyright � SAPIAN
*/

//##############################################################################
//
//                                   INCLUDE
//
//##############################################################################

// Global.
#include <Global.h>

//##############################################################################

//##############################################################################
//
//                                   MACROS
//
//##############################################################################

// Singleton accessor.
#define MatchManager CMatchManager::Get()

//##############################################################################

//##############################################################################
//
//                               MATCH MANAGER
//
//##############################################################################
class CMatchManager
{
public:
	// Singleton instance.
	static inline CMatchManager& Get() 
	{
		static CMatchManager s_Instance;
		return s_Instance;
	}

protected:
};

//##############################################################################