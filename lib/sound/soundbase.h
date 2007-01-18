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

	$Revision$
	$Id$
	$HeadURL$
*/

#ifndef SOUNDBASE_H
#define SOUNDBASE_H

// Include the OpenAL libraries
#include <AL/al.h>
#include <AL/alc.h>

#include "types.h"
#include "stream.h"

#include <map>
#include <memory>

class soundBase
{
    public:
        /** Default constructor
         *  This function constructs the soundBase class,
         *  and optionally intializes the sound device already.
         *  \param init whether the default (system/os default) sound device should be initialized at construction.
         */
        soundBase(bool init = false);
        ~soundBase();

        void setListenerPos(float x, float y, float z);
        void setListenerVel(float x, float y, float z);

        /** Sets the listener orientation.
         *  Sets the orientation of the listener to "look" at a certain direction,
         *  see http://en.wikipedia.org/wiki/Flight_dynamics for more information.
         *  \param pitch the "height" the listener is pointed at (i.e. in on screen terms)
         *  \param yaw the orientation to the "left and right" (rotation about vertical axis)
         *  \param roll just see the wikipedia article ;-) it has a nice pic explaining this way better
         */
        void setListenerRot(float pitch, float yaw, float roll);

        /**
         */


        /** updates all streams and keeps them filled
         *  this function calls update() on every soundStream class
         */
        void updateStreams();

    private:
        ALCdevice* sndDevice;
        ALCcontext* sndContext;

        bool sndExtEAX;

        std::map<sndStreamID, std::auto_ptr<soundStream> > sndStreams;
};

#endif // SOUNDBASE_H
