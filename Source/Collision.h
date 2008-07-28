#pragma once

/**
* @file Collision.h
* @author Nat Ryall
* @date 25/02/2008
* @brief Manages collisions for all object types.
*
* Copyright � Krome Studios
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

// Shortcuts.
#define CollisionManager CCollisionManager::Get()

//##############################################################################

//##############################################################################
//
//                                   TYPES
//
//##############################################################################

// Predeclare.
class CCollidable;

// The collision check types.
enum t_CollisionType
{
	CollisionType_Point,		// The collision area is a single point.
	CollisionType_Rect,			// The collision area is an axis aligned rectangle.
	CollisionType_Circle,		// The collision area is a circle.
};

// Lists.
typedef xlist<CCollidable*> t_CollidableList; 

//##############################################################################

//##############################################################################
//
//                                 COLLIDABLE
//
//##############################################################################
class CCollidable
{
public:
	// Get the collision group for this collidable.
	xint GetCollisionGroup()
	{
		return m_iCollisionGroup;
	}

	// Get the collision type for this collidable.
	t_CollisionType GetCollisionType()
	{
		return m_iCollisionType;
	}

	// Check if the object is actually collidable at the present time. 
	// This can be used for optimisation and disabling collision per-object.
	// It is important that both objects must remain valid for the duration of the call.
	virtual xbool IsCollidable(CCollidable* pWith) = 0;

	// Get the collision point.
	virtual xpoint GetCollisionPoint() 
	{ 
		return xpoint(); 
	}

	// Get the collision rect.
	virtual xrect GetCollisionRect() 
	{ 
		return xrect(); 
	}

	// Get the collision circle.
	virtual xcircle GetCollisionCircle() 
	{ 
		return xcircle(); 
	}

	// Callback that is executed when a valid collision occurs.
	virtual void OnCollision(CCollidable* pWith) = 0;

protected:
	// Constructor: Initialise the collidable group.
	CCollidable(xint iCollisionGroup, t_CollisionType iCollisionType);

	// Destructor.
	virtual ~CCollidable() {}

private:
	// The object's collision group.
	xint m_iCollisionGroup;

	// The object's collision type.
	t_CollisionType m_iCollisionType;
};

//##############################################################################

//##############################################################################
//
//                                 DECLARATION
//
//##############################################################################
class CCollisionManager : public CModule
{
public:
	// Singleton instance.
	static inline CCollisionManager& Get() 
	{
		static CCollisionManager s_Instance;
		return s_Instance;
	}

	// Check for valid collisions for managed collidables.
	virtual void Update();

	// Remove all currently managed collidables from the system and reset the manager to defaults.
	void Reset();

	// Add a collidable to the manager. Events will be fired between types automatically once added.
	void Add(CCollidable* pCollidable);

	// Remove a collidable from the manager.
	void Remove(CCollidable* pCollidable);

	// Check if two collidables are colliding regardless of type.
	xbool IsColliding(CCollidable* pA, CCollidable* pB);

protected:
	// The list of
	t_CollidableList m_lpCollidables;
};

//##############################################################################