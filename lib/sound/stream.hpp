/*
	This file is part of Warzone 2100.
	Copyright (C) 2007  Warzone Resurrection Project

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

#ifndef SOUND_STREAM_HPP
#define SOUND_STREAM_HPP

#include "openal/buffer.hpp"
#include "openal/source.hpp"
#include <boost/smart_ptr.hpp>
#include "decoding.hpp"

namespace Sound
{
    class Stream : public OpenAL::Source
    {
        public:

            /** Creates and sets up all resources required for a sound stream
             *  \param PCM class to provide decoded sound data on request
             */
            Stream(boost::shared_ptr<OpenAL::Context> sndContext, boost::shared_ptr<Decoding> PCM);
            ~Stream();

            /** keep all required buffers filled
             *  needs to be called more often if buffer size is smaller, and less often if larger
             *  \return true if there is still enough data left to fill all buffers, false otherwise
             */
            bool update();

            /** determines wether the stream is currently playing
             *  \return true if the stream is playing, false otherwise
             */
            inline bool isPlaying()
            {
                return (Source::getState() == Source::playing);
            }

            /** initiates playing of the stream
             *  starts playing the stream or continues it
             *  \return true on succes, false otherwise
             */
            virtual bool play();

            /** stops playing of the stream and resets the stream back to the beginning
             */
            virtual void stop();

            /** sets the buffersize for the streaming buffers
             *  \param size size of the buffers in bytes, minimum is 4096, which is still not recommended, 4096 is very likely to introduce clipping
             */
            void setBufferSize(unsigned int size);

            /** returns the currently used buffer size
             *  \return the currently used buffer size in bytes
             */
            unsigned int getBufferSize();

        private:

            /** fill buffer with the next data
             *  \param buffer buffer to fill
             *  \return       true if the buffer is filled, false if there was no data left to fill a buffer with
             */
            bool stream(boost::shared_ptr<OpenAL::Buffer> buffer);

            boost::shared_ptr<Decoding> decoder;

            // Internal state
            unsigned int bufferSize;    // size of the buffers in bytes/octets, default is 16384 bytes or 16kB
    };
}

#endif // SOUND_STREAM_HPP
