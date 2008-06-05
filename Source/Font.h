#pragma once

//##############################################################################
//
//                                   INCLUDE
//
//##############################################################################

// Global
#include <Global.h>

// Other.
#include <Renderer.h>
#include <Resource.h>

//##############################################################################

//##############################################################################
//
//                                   MACROS
//
//##############################################################################

// Find a specific sprite metadata using the resource manager.
#define _FONT(NAME) ((CFontMetadata*)ResourceManager::FindResource(ResourceType_Font, NAME))

//##############################################################################

//##############################################################################
//
//                                 FONT FILE
//
//##############################################################################

/**
* The sprite resource file.
*/
class CFontFile : public CResourceFile
{
public:
	/**
	* Constructor: Initialise the file.
	*/
	CFontFile(const XCHAR* pFile);

	/**
	* Destructor: Clean up the file memory.
	*/
	virtual ~CFontFile();

	/**
	* Get the sprite object.
	*/
	hgeFont* GetResource()
	{
		return (hgeFont*)pResource;
	}
};

//##############################################################################

//##############################################################################
//
//                               FONT METADATA
//
//##############################################################################
class CFontMetadata : public CResourceMetadata
{
public:
	/**
	* Constructor: Initialise the metadata.
	*/
	CFontMetadata(CDataset* pDataset);

	/**
	* Destructor: Deinitialise the metadata.
	*/
	virtual ~CFontMetadata();

	/**
	* Get the font object.
	*/
	hgeFont* GetFont()
	{
		return pFile->GetResource();
	}

public:
	CFontFile* pFile;			// The font file used for rendering.

	XFLOAT fScale;				// The font scale.
	XUINT iColour;				// The font colour.
	XFLOAT fSpacing;			// The spacing between lines.
	XFLOAT fTracking;			// The spacing between letters.
	XPOINT xShadowOffset;		// The font shadow offset.
	XUINT iShadowColour;		// The font shadow colour.
};

//##############################################################################

//##############################################################################
//
//                                    FONT
//
//##############################################################################
class CFont
{
public:
	/**
	* Constructor.
	*/
	CFont(CFontMetadata* pMetadata) : m_pMetadata(pMetadata) {}

	/**
	* Destructor.
	*/
	virtual ~CFont() {}

	/**
	* Render the font with the specified attributes.
	* @param pString The text string to render to the screen using this font.
	* @param xRect The aligned screen area to render in.
	* @param iAlign The font alignment relative to the position. Use HGETEXT_*.
	* @param fAlpha The alpha level between 0.0 and 1.0 to render the text at.
	*/
	void Render(const xchar* pString, xrect xRect, xuint iAlign, xfloat fAlpha);

	/**
	* Render the font with the specified attributes.
	* @param pString The text string to render to the screen using this font.
	* @param xPosition The horizontally-aligned screen position to render using.
	* @param iAlign The font alignment relative to the position. Use HGETEXT_*.
	* @param fAlpha The alpha level between 0.0 and 1.0 to render the text at.
	*/
	void Render(const xchar* pString, xpoint xPosition, xuint iAlign, xfloat fAlpha);

	/**
	* Render the font with the specified attributes.
	* @param pString The text string to render to the screen using this font.
	* @param xRect The aligned screen area to render in.
	* @param iAlign The font alignment relative to the position. Use HGETEXT_*.
	*/
	inline void Render(const xchar* pString, xrect xRect, xuint iAlign)
	{
		Render(pString, xRect, iAlign, 1.f);
	}

	/**
	* Render the font with the specified attributes.
	* @param pString The text string to render to the screen using this font.
	* @param xPosition The horizontally-aligned screen position to render using.
	* @param iAlign The font alignment relative to the position. Use HGETEXT_*.
	*/
	inline void Render(const xchar* pString, xpoint xPosition, xuint iAlign)
	{
		Render(pString, xPosition, iAlign, 1.f);
	}

	/**
	* Get the font height.
	*/
	xint GetFontHeight()
	{
		return (xint)m_pMetadata->GetFont()->GetHeight();
	}

	/**
	* Get the width of a string of characters using the specified font.
	*/
	xint GetStringWidth(const XCHAR* pString)
	{
		return (xint)m_pMetadata->GetFont()->GetStringWidth(pString);
	}

	/**
	* Get the metadata structure. This is for advanced use only.
	*/
	CFontMetadata* GetMetadata()
	{
		return m_pMetadata;
	}

protected:
	// The metadata for this object.
	CFontMetadata* m_pMetadata;
};

//##############################################################################

//##############################################################################
//
//                                    TEXT
//
//##############################################################################
class CText : public CFont
{
public:
	/**
	* Constructor.
	*/
	CText(CFontMetadata* pMetadata);

	/**
	* Render the font with all current attributes.
	*/
	virtual void Render();

	/**
	* Set the text string to render.
	*/
	void SetString(const XCHAR* pString)
	{
		m_xString = pString;
	}

	/**
	* Get the text string to be rendered.
	*/
	const XCHAR* GetString()
	{
		return m_xString.c_str();
	}

	/**
	* Set the screen position to render from.
	*/
	void SetPosition(XPOINT xPosition)
	{
		m_xPosition = xPosition;
	}

	/**
	* Get the screen position to render from.
	*/
	XPOINT GetPosition()
	{
		return m_xPosition;
	}

	/**
	* Set the text alignment. Use HGETEXT_*.
	*/
	void SetAlignment(XUINT iAlignment)
	{
		m_iAlignment = iAlignment;
	}

	/**
	* Get the text alignment.
	*/
	XUINT GetAlignment()
	{
		return m_iAlignment;
	}

	/**
	* Get the width of the current text string using the specified font.
	*/
	XUINT GetStringWidth()
	{
		return CFont::GetStringWidth(GetString());
	}

protected:
	// The text string to render.
	XSTRING m_xString;

	// The screen position to render at.
	XPOINT m_xPosition;

	// The text alignment relative to the position.
	XUINT m_iAlignment;
};

//##############################################################################