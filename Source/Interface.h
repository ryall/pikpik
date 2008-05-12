#pragma once

/**
* @file Interface.h
* @author Nat Ryall
* @date 30/04/2008
* @brief Interface manager and elements.
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

// Other.
#include <Renderer.h>
#include <Resource.h>
#include <Sprite.h>
#include <Font.h>

//##############################################################################

//##############################################################################
//
//                                   MACROS
//
//##############################################################################

// Shortcuts.
#define InterfaceManager CInterfaceManager::Get()

//##############################################################################

//##############################################################################
//
//                                   TYPES
//
//##############################################################################

// Predeclare.
class CInterfaceElement;
class CScreenElement;

// The types of interface elements.
enum t_ElementType
{
  ElementType_Unknown,
	ElementType_Screen,
	ElementType_Label,
	ElementType_Image,
	ElementType_Button,
	ElementType_Input,
	ElementType_Progress,
  ElementType_Window,
  ElementType_List,
  ElementType_Check,
  ElementType_Radio,
	ElementType_Scroll,
  ElementType_MenuLink,
  /*MAX*/ElementType_Max,
};

// Lists.
typedef XLIST<CInterfaceElement*> t_ElementList;

//##############################################################################

//##############################################################################
//
//                             INTERFACE MANAGER
//
//##############################################################################
class CInterfaceManager : public Templates::CSingletonT<CInterfaceManager>
{
public:
  // Initialise the interface manager.
  CInterfaceManager();

  // Deinitialise the interface manager.
  ~CInterfaceManager();

	// Reset the settings to defaults and clear all elements.
	void Reset();

  // Called to update and fire events for elements.
  virtual void Update();

  // Called to render elements.
  virtual void Render();

  // Get the base screen element that all other elements should attach to.
  CScreenElement* GetScreen()
  {
    return m_pScreen;
  }

  // Set the cursor image for a specific element type.
  void SetCursor(t_ElementType iType, CSpriteMetadata* pMetadata);

  // Set the default cursor image.
  void SetCursor(CSpriteMetadata* pMetadata)
  {
    SetCursor(ElementType_Unknown, pMetadata);
  }

	// Get the first visible or "active" element that is currently under the mouse.
	CInterfaceElement* GetActiveElement()
	{
		return m_pActiveElement;
	}

	// Check if a specific element is the active element.
	XBOOL IsActiveElement(CInterfaceElement* pElement)
	{
		return m_pActiveElement == pElement;
	}

	// Focus the specified element. This is called automatically when an element is clicked.
	void SetFocus(CInterfaceElement* pElement);

	// Get the currently focused element or NULL if there is no focus.
	CInterfaceElement* GetFocusedElement()
	{
		return m_pFocusedElement;
	}

  // Check if a specific element is the focused element.
  XBOOL IsFocusedElement(CInterfaceElement* pElement)
  {
    return m_pFocusedElement == pElement;
  }

	// Get the current mouse position.
	XPOINT GetMousePosition()
	{
		return m_xMousePos;
	}

	// Check if the mouse cursor is over a specific element.
	XBOOL IsMouseOver(CInterfaceElement* pElement);

	// Check if the left-mouse button is pressed.
	XBOOL IsMouseDown()
	{
		return _HGE->Input_GetKeyState(HGEK_LBUTTON);
	}

  // Switch the debug renderer on or off.
  void SetDebugRender(XBOOL bDebug)
  {
    m_bDebugRender = bDebug;
  }

protected:
  // Recursive function called on each element. Called in reverse render order.
  void UpdateElement(CInterfaceElement* pElement);

  // Recursive function called on each element.
  void RenderElement(CInterfaceElement* pElement);

  // The base container object.
  CScreenElement* m_pScreen;

  // The cursor sprite.
  CBasicSprite* m_pCursor[ElementType_Max];

  // The current mouse position.
  XPOINT m_xMousePos;

  // The last mouse position.
  XPOINT m_xLastMousePos;

  // The active element the mouse is over.
  CInterfaceElement* m_pActiveElement;

	// The last focused element.
	CInterfaceElement* m_pFocusedElement;

private:
  // Tracking variable for finding the current active element.
  XBOOL m_bFoundActive;

  // Specifies if debug rendering is used.
  XBOOL m_bDebugRender;
};

//##############################################################################

//##############################################################################
//
//                             INTERFACE ELEMENT
//
//##############################################################################
class CInterfaceElement
{
public:
  // Friend.
  friend CInterfaceManager;

  // Virtual destructor to ensure proper cleanup of all child classes.
  virtual ~CInterfaceElement() {}

  // Optional update virtual function for this element. Called each frame when visible.
  virtual void Update() {}
  
  // Required render virtual function for this element. Called each frame when visible.
  virtual void Render() = 0;

  // Get the parent element that this element belongs to. Returns NULL if top-level.
  CInterfaceElement* GetParent() 
  { 
    return m_pParent; 
  }

  // Get the type of the element assigned at construction time.
  inline xuint GetType()
  {
    return m_iType;
  }

  // Set the visibility of the element. This will also disable the element when invisible.
  inline void SetVisible(xbool bVisible)
  {
    m_bVisible = bVisible;
  }

  // Check if the element is visible.
  inline xbool IsVisible()
  {
    return m_bVisible;
  }

  // Set the enabled status of the element. This will prevent input notifications for this element.
  inline void SetEnabled(xbool bEnabled)
  {
    m_bEnabled = bEnabled;
  }

  // Check if the element is enabled.
  inline xbool IsEnabled()
  {
    return m_bEnabled;
  }

  // Get the width of the element. This must be overloaded.
  virtual xint GetWidth() = 0;

	// Get the height of the element. This must be overloaded.
	virtual xint GetHeight() = 0;

	// Get the size of the element.
	inline xpoint GetSize()
	{
		return xpoint(GetWidth(), GetHeight());
	}

	// Get the element area for collision and event purposes.
	inline xrect GetArea()
	{
		return xrect(GetPosition(), GetPosition() + GetSize());
	}

	// Get the focus area of the element. By default, this is the element area.
	virtual xrect GetFocusArea()
	{
		return GetArea();
	}

  // Set the screen position of the element.
  inline void SetPosition(xpoint xPosition)
	{
		Move(xPosition - m_xPosition);
	}

	// CHange the position of the element by specifying an offset.
	void Move(xpoint xOffset);

	// Get the screen position of the element.
  inline xpoint GetPosition()
  {
    return m_xPosition;
  }

	// Get the position of the element relative to it's parent element.
	inline xpoint GetLocalPosition()
	{
		if (m_pParent)
			return GetPosition() - m_pParent->GetPosition();
		else
			return GetPosition();
	}

  // Attach a child element to this element.
  void Attach(CInterfaceElement* pElement);
  
  // Detach a specific child element from this element.
  void Detach(CInterfaceElement* pElement);

	// Detach all child elements from this element.
	inline void DetachAll()
	{
		m_lpChildElements.clear();
	}

	// Bring the element to the front on the parent element.
	void ToFront();

protected:
  // Set the element type and initialise the element.
  CInterfaceElement(t_ElementType iType);

  // Triggered when focus is applied to the element.
  virtual void OnFocus() {}

	// Triggered when focus is lost from the element.
  virtual void OnBlur() {}

	// Triggered when the mouse enters this element.
  virtual void OnMouseEnter() {}

	// Triggered when the mouse leaves this element. 
  virtual void OnMouseLeave() {}

  // Triggered when the left mouse-button is pressed within the element area.
  virtual void OnMouseDown(xpoint xPosition) {}

	// Triggered when the left mouse-button is released within the element area.
  virtual void OnMouseUp(xpoint xPosition) {}

	// Triggered when the mouse is moved within the element area.
	virtual void OnMouseMove(xpoint xDifference, xbool bMouseDown) {}

	// Triggered when a keyboard key is pressed whilst the element is in focus.
  virtual void OnKeyDown(xint iVirtualKey) {}

	// Triggered when a keyboard key is released whilst the element is in focus.
  virtual void OnKeyUp(xint iVirtualKey) {}

	// Triggered when an ASCII key is input on the keyboard whilst the element is in focus.
	virtual void OnKeyChar(xchar cChar) {}

  // The parent element or NULL if top level.
  CInterfaceElement* m_pParent;

  // The type of the element.
  t_ElementType m_iType;

  // Specifies if the element is visible.
  xbool m_bVisible;

  // Specifies if the element is enabled and will accept events.
  xbool m_bEnabled;

  // A list of child elements.
  t_ElementList m_lpChildElements;

private:
	// The screen position of the element.
	xpoint m_xPosition;
};

//##############################################################################

//##############################################################################
//
//                               SCREEN ELEMENT
//
//##############################################################################
class CScreenElement : public CInterfaceElement
{
public:
	// Constructor to initialise the element.
	CScreenElement() : CInterfaceElement(ElementType_Screen) {}

	// Virtual destructor to ensure proper cleanup of all child classes.
	virtual ~CScreenElement() {}

	// Render the screen element (does nothing).
	virtual void Render() {}

	// Get the width of the element. This must be overloaded.
	virtual xint GetWidth() 
	{
		return _SWIDTH;
	};

	// Get the height of the element. This must be overloaded.
	virtual xint GetHeight()
	{
		return _SHEIGHT;
	}
};

//##############################################################################