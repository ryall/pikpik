/**
* @file Menu.cpp
* @author Nat Ryall
* @date 11/04/2008
* @brief Manages menu transitions.
*
* Copyright � SAPIAN
*/

#pragma region Include
//##############################################################################
//
//                                   INCLUDE
//
//##############################################################################

// Global.
#include <Global.h>

// Local.
#include <Menu.h>

// Other.
#include <Resource.h>
#include <Font.h>
#include <Selection.h>
#include <Component.h>
#include <Lobby.h>

//##############################################################################
#pragma endregion

//##############################################################################
//
//                                   MACROS
//
//##############################################################################

// The menu transition values.
#define MENU_TRANSITION_DELAY 0
#define MENU_TRANSITION_TIME 200
#define MENU_TRANSITION_DISTANCE _SWIDTH

//##############################################################################

#pragma region Elements
//##############################################################################
//
//                                  ELEMENTS
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         13-Apr-2008
// =============================================================================
CMenuLink::CMenuLink(xuint iGroupIndex, CFontMetadata* pFont, const xchar* pText, t_fpLinkSelectedCallback fpCallback) : CTextLink(pFont, pText, fpCallback),
	m_iGroupIndex(iGroupIndex),
	m_iElementIndex(0)
{
	m_iType = ElementType_MenuLink;
}

// =============================================================================
// Nat Ryall                                                         18-Apr-2008
// =============================================================================
void CMenuLink::RePosition(xuint iElementIndex, xuint iNumElements)
{
	m_iElementIndex = iElementIndex;
	m_xLinkPosition = xpoint(_HSWIDTH - (GetWidth() / 2), _HSHEIGHT - (iNumElements * (GetHeight() / 2)) + iElementIndex * (GetHeight() + 5));

	SetPosition(m_xLinkPosition);
}

//##############################################################################
#pragma endregion

#pragma region Definition
//##############################################################################
//
//                                 DEFINITION
//
//##############################################################################

// =============================================================================
// Nat Ryall                                                         11-Apr-2008
// =============================================================================
void CMenuScreen::Load()
{
	_GLOBAL.pMenu = this;

	// Initialise the render resources.
	m_pBackground = new CBackgroundImage("Menu-Background");

	m_pMenuDefault = _FONT("Menu-Default");
	m_pMenuHighlight = _FONT("Menu-Highlighted");

	// Initialise interface.
	Interface.SetCursor(_SPRITE("Cursor-Main"));
	Interface.SetCursor(ElementType_Button, _SPRITE("Cursor-Click"));
	Interface.SetCursor(ElementType_Input, _SPRITE("Cursor-Write"));
	Interface.SetCursor(ElementType_MenuLink, _SPRITE("Cursor-Click"));
	Interface.SetCursor(ElementType_SessionBox, _SPRITE("Cursor-Click"));

	// Initialise the menu links.
	CMenuLink* pLinkList[] = 
	{
		// Main.
		new CMenuLink(MenuGroup_Main,		m_pMenuHighlight,	_LOCALE("Menu_Online"),			xbind(this, &CMenuScreen::Callback_ShowOnlineMenu)),
		new CMenuLink(MenuGroup_Main,		m_pMenuDefault,		_LOCALE("Menu_Offline"),		xbind(this, &CMenuScreen::Callback_StartGame)),
		new CMenuLink(MenuGroup_Main,		m_pMenuDefault,		_LOCALE("Menu_Tutorial"),		NULL),
		new CMenuLink(MenuGroup_Main,		m_pMenuDefault,		_LOCALE("Menu_Options"),		NULL),
		new CMenuLink(MenuGroup_Main,		m_pMenuDefault,		_LOCALE("Menu_Credits"),		NULL),
		new CMenuLink(MenuGroup_Main,		m_pMenuDefault,		_LOCALE("Menu_Exit"),			xbind(this, &CMenuScreen::Callback_QuitGame)),

		// Online.
		new CMenuLink(MenuGroup_Online,		m_pMenuDefault,		_LOCALE("Menu_OnlineJoin"),		xbind(this, &CMenuScreen::Callback_ShowJoinMenu)),
		new CMenuLink(MenuGroup_Online,		m_pMenuDefault,		_LOCALE("Menu_OnlineCreate"),	xbind(this, &CMenuScreen::Callback_ShowCreateMenu)),
		new CMenuLink(MenuGroup_Online,		m_pMenuHighlight,	_LOCALE("Menu_Back"),			xbind(this, &CMenuScreen::Callback_ShowMainMenu)),

		// Find.
		new CMenuLink(MenuGroup_Join,		m_pMenuDefault,		_LOCALE("Menu_JoinPublic"),		xbind(this, &CMenuScreen::Callback_JoinPublic)),
		new CMenuLink(MenuGroup_Join,		m_pMenuDefault,		_LOCALE("Menu_JoinPrivate"),	xbind(this, &CMenuScreen::Callback_JoinPrivate)),
		new CMenuLink(MenuGroup_Join,		m_pMenuHighlight,	_LOCALE("Menu_Back"),			xbind(this, &CMenuScreen::Callback_ShowOnlineMenu)),

		// Create.
		new CMenuLink(MenuGroup_Create,		m_pMenuDefault,		_LOCALE("Menu_CreatePublic"),	xbind(this, &CMenuScreen::Callback_CreatePublic)),
		new CMenuLink(MenuGroup_Create,		m_pMenuDefault,		_LOCALE("Menu_CreatePrivate"),	xbind(this, &CMenuScreen::Callback_CreatePrivate)),
		new CMenuLink(MenuGroup_Create,		m_pMenuHighlight,	_LOCALE("Menu_Back"),			xbind(this, &CMenuScreen::Callback_ShowOnlineMenu)),
	};

	for (xuint iA = 0; iA < (sizeof(pLinkList) / sizeof(CMenuLink*)); ++iA)
		m_lpMenuLinks[pLinkList[iA]->m_iGroupIndex].push_back(pLinkList[iA]);

	for (xuint iGroup = 0; iGroup < MenuGroup_Max; ++iGroup)
	{
		xuint iElementCount = (xuint)m_lpMenuLinks[iGroup].size();
		xuint iElement = 0;

		XEN_LIST_FOREACH(t_MenuLinkList, ppMenuLink, m_lpMenuLinks[iGroup])
			(*ppMenuLink)->RePosition(iElement++, iElementCount);
	}

	// Initialise transition variables.
	m_iState = MenuState_None;
	m_iMenuGroup = MenuGroup_None;
	m_iPendingMenuGroup = MenuGroup_None;
	m_iLastMenuGroup = MenuGroup_Main;
	m_iNextScreen = ScreenIndex_Invalid;
	m_iLobbyMode = LobbyStartMode_CreatePrivate;
}

// =============================================================================
// Nat Ryall                                                         11-Apr-2008
// =============================================================================
void CMenuScreen::Unload()
{
	delete m_pMenuDefault;
	delete m_pMenuHighlight; 

	delete m_pBackground;
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
void CMenuScreen::Wake()
{
	SetMenuGroup(m_iLastMenuGroup);
}

// =============================================================================
// Nat Ryall                                                         09-Jun-2008
// =============================================================================
void CMenuScreen::Sleep()
{
}

// =============================================================================
// Nat Ryall                                                         13-Apr-2008
// =============================================================================
void CMenuScreen::Update()
{
	// Allow "ESC" to exit the game.
	if (m_iState == MenuState_Idle && _HGE->Input_KeyUp(HGEK_ESCAPE))
	{
		switch (m_iMenuGroup)
		{
		case MenuGroup_Main:
			_TERMINATE;
			return;

		case MenuGroup_Online:
			SetMenuGroup(MenuGroup_Main);
			break;

		case MenuGroup_Join:
		case MenuGroup_Create:
			SetMenuGroup(MenuGroup_Online);
			break;
		}
	}

	// Update based on the state.
	switch (m_iState)
	{
	case MenuState_TransitionIn:
		{
			UpdateTransition(true);
		}
		break;

	case MenuState_TransitionOut:
		{
			UpdateTransition(false);
		}
		break;
	}

	m_pBackground->Update();
}

// =============================================================================
// Nat Ryall                                                         20-Jul-2008
// =============================================================================
void CMenuScreen::UpdateTransition(xbool bTransitionIn)
{
	XEN_LIST_FOREACH(t_MenuLinkList, ppMenuLink, m_lpMenuLinks[m_iMenuGroup])
	{
		CMenuLink* pMenuLink = *ppMenuLink;

		if (pMenuLink->m_xStartTimer.IsExpired())
		{
			// Perform the transition in for each link as it expires.
			xint iOffset = XINTPERCENT(pMenuLink->m_iTransitionTime, MENU_TRANSITION_DISTANCE, MENU_TRANSITION_TIME);
			xint iDirectionalOffset = bTransitionIn ? MENU_TRANSITION_DISTANCE - iOffset : iOffset;

			pMenuLink->m_iTransitionTime = Math::Clamp<xuint>(pMenuLink->m_iTransitionTime + _TIMEDELTA, 0, MENU_TRANSITION_TIME);
			pMenuLink->SetPosition(pMenuLink->m_xLinkPosition + xpoint(pMenuLink->m_bTransitionRight ? iDirectionalOffset : -iDirectionalOffset, 0));

			// If we're the last one to finish the transition, go to 'idle'.
			if (pMenuLink->m_iElementIndex == ((xint)m_lpMenuLinks[m_iMenuGroup].size() - 1) && iOffset == MENU_TRANSITION_DISTANCE)
			{
				SetState(bTransitionIn ? MenuState_Idle : MenuState_None);
				return;
			}
		}
	}
}

// =============================================================================
// Nat Ryall                                                         13-Apr-2008
// =============================================================================
void CMenuScreen::Render()
{
	m_pBackground->Render();
}

// =============================================================================
// Nat Ryall                                                         20-Jul-2008
// =============================================================================
void CMenuScreen::SetState(t_MenuState iState)
{
	m_iState = iState;

	switch (iState)
	{
	case MenuState_None:
		{
			m_iLastMenuGroup = m_iMenuGroup;
			m_iMenuGroup = MenuGroup_None;

			ShowNextScreen();
		}
		break;

	case MenuState_TransitionIn:
		{
			InterfaceRoot->SetEnabled(false);
			InitTransition(true);
		}
		break;

	case MenuState_Idle:
		{
			InterfaceRoot->SetEnabled(true);
		}
		break;

	case MenuState_TransitionOut:
		{
			InterfaceRoot->SetEnabled(false);
			InitTransition(false);
		}
		break;
	}
}

// =============================================================================
// Nat Ryall                                                         20-Jul-2008
// =============================================================================
void CMenuScreen::SetNextScreen(t_ScreenIndex iScreenIndex)
{
	m_iNextScreen = iScreenIndex;
	SetState(MenuState_TransitionOut);
}

// =============================================================================
// Nat Ryall                                                         17-Jul-2008
// =============================================================================
void CMenuScreen::ShowNextScreen()
{
	if (m_iNextScreen != ScreenIndex_Invalid)
	{
		ScreenManager::Push(m_iNextScreen);

		switch (m_iNextScreen)
		{
		case ScreenIndex_LobbyScreen:
			_GLOBAL.pLobby->Start(m_iLobbyMode);
			break;
		}

		m_iNextScreen = ScreenIndex_Invalid;

		return;
	}

	if (m_iPendingMenuGroup != MenuGroup_None)
	{
		SetMenuGroup(m_iPendingMenuGroup);
		m_iPendingMenuGroup = MenuGroup_None;
	}
}

// =============================================================================
// Nat Ryall                                                         13-Apr-2008
// =============================================================================
void CMenuScreen::SetMenuGroup(t_MenuGroup iMenuGroup)
{
	if (iMenuGroup != MenuGroup_None)
	{
		if (m_iMenuGroup == MenuGroup_None)
		{
			AttachMenuGroup(iMenuGroup);
			SetState(MenuState_TransitionIn);
		}
		else
		{
			m_iPendingMenuGroup = iMenuGroup;
			SetState(MenuState_TransitionOut);
		}
	}
	else
	{
		if (m_iMenuGroup != MenuGroup_None)
			SetState(MenuState_TransitionOut);
	}
}

// =============================================================================
// Nat Ryall                                                         17-Jul-2008
// =============================================================================
void CMenuScreen::AttachMenuGroup(t_MenuGroup iMenuGroup)
{
	m_iMenuGroup = iMenuGroup;

	InterfaceRoot->DetachAll();

	if (iMenuGroup != MenuGroup_None)
	{
		XEN_LIST_FOREACH(t_MenuLinkList, ppMenuLink, m_lpMenuLinks[iMenuGroup])
			InterfaceRoot->Attach(*ppMenuLink);
	}
}

// =============================================================================
// Nat Ryall                                                         20-Jul-2008
// =============================================================================
void CMenuScreen::InitTransition(xbool bTransitionIn)
{
	XEN_LIST_FOREACH(t_MenuLinkList, ppMenuLink, m_lpMenuLinks[m_iMenuGroup])
	{
		CMenuLink* pMenuLink = *ppMenuLink;

		pMenuLink->m_bTransitionRight = (pMenuLink->m_iElementIndex % 2 == 0);
		pMenuLink->m_iTransitionTime = 0;
		pMenuLink->m_xStartTimer.ExpireAfter(MENU_TRANSITION_DELAY * pMenuLink->m_iElementIndex);

		if (bTransitionIn)
			pMenuLink->SetPosition(xpoint(pMenuLink->m_xLinkPosition.iX, -pMenuLink->GetHeight()));
		else
			pMenuLink->m_bTransitionRight = !pMenuLink->m_bTransitionRight;
	}
}

// =============================================================================
// Nat Ryall                                                         27-Apr-2008
// =============================================================================
void CMenuScreen::Callback_ShowMainMenu()
{
	SetMenuGroup(MenuGroup_Main);
}

// =============================================================================
// Nat Ryall                                                         27-Apr-2008
// =============================================================================
void CMenuScreen::Callback_ShowOnlineMenu()
{
	SetMenuGroup(MenuGroup_Online);
}

// =============================================================================
// Nat Ryall                                                         09-Jul-2008
// =============================================================================
void CMenuScreen::Callback_ShowJoinMenu()
{
	SetMenuGroup(MenuGroup_Join);
}

// =============================================================================
// Nat Ryall                                                         10-Jul-2008
// =============================================================================
void CMenuScreen::Callback_ShowCreateMenu()
{
	SetMenuGroup(MenuGroup_Create);
}

// =============================================================================
// Nat Ryall                                                         09-Jul-2008
// =============================================================================
void CMenuScreen::Callback_JoinPublic()
{
	SetNextScreen(ScreenIndex_LobbyScreen);
	m_iLobbyMode = LobbyStartMode_JoinPublic;
}

// =============================================================================
// Nat Ryall                                                          5-May-2008
// =============================================================================
void CMenuScreen::Callback_JoinPrivate()
{
	SetNextScreen(ScreenIndex_LobbyScreen);
	m_iLobbyMode = LobbyStartMode_JoinPrivate;
}

// =============================================================================
// Nat Ryall                                                         15-Jun-2008
// =============================================================================
void CMenuScreen::Callback_CreatePublic()
{
	SetNextScreen(ScreenIndex_LobbyScreen);
	m_iLobbyMode = LobbyStartMode_CreatePublic;
}

// =============================================================================
// Nat Ryall                                                         27-Apr-2008
// =============================================================================
void CMenuScreen::Callback_CreatePrivate()
{
	SetNextScreen(ScreenIndex_LobbyScreen);
	m_iLobbyMode = LobbyStartMode_CreatePrivate;
}

// =============================================================================
// Nat Ryall                                                         13-Apr-2008
// =============================================================================
void CMenuScreen::Callback_StartGame()
{
	SetNextScreen(ScreenIndex_GameScreen);
}

// =============================================================================
// Nat Ryall                                                         13-Apr-2008
// =============================================================================
void CMenuScreen::Callback_QuitGame()
{
	_TERMINATE;
}

//##############################################################################
#pragma endregion