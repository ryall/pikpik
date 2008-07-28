//##############################################################################
//
//                                   INCLUDE
//
//##############################################################################

// Global.
#include <Global.h>

// Local.
#include <Property.h>

//##############################################################################

//##############################################################################
//
//                                 DEFINITION
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         10-Apr-2008
// =============================================================================
xchar CProperty::GetChar(xuint iIndex)
{
	return m_lpValues[iIndex][0];
}

// =============================================================================
// Author: Nat Ryall                                           Date: 30-Jan-2008
// =============================================================================
xint CProperty::GetInt(xuint iIndex)
{
	return atoi(m_lpValues[iIndex]);
}

// =============================================================================
// Author: Nat Ryall                                           Date: 30-Jan-2008
// =============================================================================
xfloat CProperty::GetFloat(xuint iIndex)
{
	return (float)atof(m_lpValues[iIndex]);
}

// =============================================================================
// Author: Nat Ryall                                           Date: 30-Jan-2008
// =============================================================================
xbool CProperty::GetBool(xuint iIndex)
{
	return (strcmp(GetString(iIndex), "false") == 0 || strcmp(GetString(iIndex), "0")) ? false : true;
}

// =============================================================================
// Author: Nat Ryall                                           Date: 30-Jan-2008
// =============================================================================
const xchar* CProperty::GetString(xuint iIndex)
{
	return (const xchar*)m_lpValues[iIndex];
}

//##############################################################################