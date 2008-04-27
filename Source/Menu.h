#pragma once

/**
* @file Menu.h
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

// Other.
#include <Sprite.h>
#include <Background.h>
#include <Font.h>

//##############################################################################
#pragma endregion

#pragma region Types
//##############################################################################
//
//                                   TYPES
//
//##############################################################################

// Predeclare.
class CMenuScreen;
class CMenuLink;

// The different group types.
enum t_MenuGroupIndex
{
	MenuGroupIndex_None = -1,
	MenuGroupIndex_Main,
	MenuGroupIndex_Play,
	MenuGroupIndex_Online,
	/*MAX*/MenuGroupIndex_Max,
};

// The different transition states.
enum t_MenuElementState
{
	MenuElementState_None = -1,
	MenuElementState_In,
	MenuElementState_Normal,
	MenuElementState_Out,
};

// Lists.
typedef XLIST<CMenuLink*> t_MenuLinkList;

//##############################################################################
#pragma endregion

#pragma region Elements
//##############################################################################
//
//                                  ELEMENTS
//
//##############################################################################

// Text link element.
class CMenuLink : public hgeGUIObject
{
public:
	// Friends.
	friend CMenuScreen;

	// Callbacks.
	typedef void (*t_fpLinkSelectedCallback)();

	/**
	* Constructor.
	*/
	CMenuLink(XUINT iGroupIndex, CFontMetadata* pFont, const XCHAR* pText, t_fpLinkSelectedCallback fpCallback = NULL);

	/**
	* Destructor.
	*/
	virtual ~CMenuLink();

	/**
	* Update the element.
	*/
	virtual void Update(float fDeltaTime);

	/**
	* Render the element.
	*/
	virtual void Render();

	/**
	* Called when the element is clicked or released.
	*/
	virtual bool MouseLButton(bool bDown);

protected:
	/**
	* Re-position the menu link in relation to other links.
	*/
	void RePosition(XUINT iElementIndex, XUINT iNumElements);

	/**
	* Execute the internal callback.
	*/
	void ExecuteCallback()
	{
		if (m_fpLinkSelectedCallback)
			m_fpLinkSelectedCallback();
	}

	// The group index of the element. This us used to enable/disable elements in groups.
	XUINT m_iGroupIndex;

	// The text to render for the link.
	CText* m_pText;

	// The link callback.
	t_fpLinkSelectedCallback m_fpLinkSelectedCallback;
};

//##############################################################################
#pragma endregion

#pragma region Declaration
//##############################################################################
//
//                                 DECLARATION
//
//##############################################################################
class CMenuScreen : public CScreen
{
public:
	CMenuScreen() : CScreen(ScreenIndex_MenuScreen) {}

	/**
	* Called when the screen is registered in the constructor.
	*/
	virtual void Load();

	/**
	* Called when the state is dismissed in the destructor.
	*/
	virtual void Unload();

	/**
	* Called once when the screen is placed at the top of the stack.
	*/
	virtual void Wake() {}

	/**
	* Called once when the screen goes out of scope either through destruction or another screen is placed above this on the stack.
	*/
	virtual void Sleep() {}

	/**
	* Called each frame to update the screen when active.
	*/
	virtual void Update();

	/**
	* Called each frame to render the screen when active.
	*/
	virtual void Render();

	/**
	* Called when a game-specific event is executed when active.
	*/
	virtual void Event(XIN xuint iEventType, XIN void* pEventInfo) {}

  /**
  * Set the menu group to render.
  */
  void SetMenuGroup(t_MenuGroupIndex iMenuGroup);

protected:
	// The GUI control object.
	hgeGUI* m_pGUI;

	// The font metadata to render the links with.
	CFontMetadata* m_pFont;

	// A list of all the menu elements.
	t_MenuLinkList m_lpMenuLinks[MenuGroupIndex_Max];

	// The background image to scroll.
	CBackgroundImage* m_pBackground;

	// The cursor image to use.
	CBasicSprite* m_pCursor;

	// The currently active menu group.
	t_MenuGroupIndex m_iMenuGroup;
};

//##############################################################################
#pragma endregion