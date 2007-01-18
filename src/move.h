/*
	This file is part of Warzone 2100.
	Copyright (C) 1999-2004  Eidos Interactive
	Copyright (C) 2005-2007  Warzone Resurrection Project

	Warzone 2100 is free software; you can redistribute it and/or modify
	it under the terms of the GNU General Public License as published by
	the Free Software Foundation; either version 2 of the License, or
	(at your option) any later version.

	Warzone 2100 is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with Warzone 2100; if not, write to the Free Software
	Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA
*/
/*
 * Move.h
 *
 * Interface for the unit movement system
 *
 */
#ifndef _move_h
#define _move_h

#include "objectdef.h"
#include "findpath.h"
// #include "lib/sound/sound.h" // Don't tell me this file needs to be included in a header

/* The base movement speed */
extern FRACT	baseSpeed;

// The next object that should get the router when a lot of units are
// in a MOVEROUTE state
extern DROID	*psNextRouteDroid;

/* Initialise the movement system */
extern BOOL moveInitialise(void);

/* Update the base speed for all movement */
extern void moveUpdateBaseSpeed(void);

/* Set a target location for a droid to move to  - returns a BOOL based on if there is a path to the destination (TRUE if there is a path)*/
extern BOOL moveDroidTo(DROID *psDroid, UDWORD x, UDWORD y);

/* Set a target location for a droid to move to  - returns a BOOL based on if there is a path to the destination (TRUE if there is a path)*/
// the droid will not join a formation when it gets to the location
extern BOOL moveDroidToNoFormation(DROID *psDroid, UDWORD x, UDWORD y);

// move a droid directly to a location (used by vtols only)
extern void moveDroidToDirect(DROID *psDroid, UDWORD x, UDWORD y);

// Get a droid to turn towards a locaton
extern void moveTurnDroid(DROID *psDroid, UDWORD x, UDWORD y);

/* Stop a droid */
extern void moveStopDroid(DROID *psDroid);

/*Stops a droid dead in its tracks - doesn't allow for any little skidding bits*/
extern void moveReallyStopDroid(DROID *psDroid);

/* Get a droid to do a frame's worth of moving */
extern void moveUpdateDroid(DROID *psDroid);

SDWORD moveCalcDroidSpeed(DROID *psDroid);

/* Frame update for the movement of a tracked droid */
extern void moveUpdateTracked(DROID *psDroid);

extern void fillNewBlocks(DROID *psDroid);
extern void fillInitialView(DROID *psDroid);

/* update body and turret to local slope */
extern void updateDroidOrientation(DROID *psDroid);

extern void moveSetFormationSpeedLimiting( BOOL );
extern void moveToggleFormationSpeedLimiting( void );
extern BOOL moveFormationSpeedLimitingOn( void );


/* audio callback used to kill movement sounds */
extern BOOL moveCheckDroidMovingAndVisible( AUDIO_SAMPLE *psSample );

// set a vtol to be hovering in the air
void moveMakeVtolHover( DROID *psDroid );

//BOOL StartDriverMode(void);
//void StopDriverMode(void);
//BOOL driveModeActive(void);
//BOOL driveIsDriven(DROID *psDroid);
//BOOL driveIsFollower(DROID *psDroid);
//DROID *driveGetDriven(void);
//void driveDroidKilled(DROID *psDroid);
//void driveSelectionChanged(void);
//void driveNextDriver(void);
//void driveUpdate(void);
//extern void	setDrivingStatus( BOOL val );
//extern BOOL	getDrivingStatus( void );

#endif

