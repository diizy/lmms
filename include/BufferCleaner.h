/*
 * BufferCleaner.h - an object for cleaning up buffers in multithreads
 *
 * Copyright (c) 2014 Vesa Kivimäki
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of Linux MultiMedia Studio - http://lmms.sourceforge.net
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program (see COPYING); if not, write to the
 * Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301 USA.
 *
 */
 
#ifndef BUFFER_CLEANER_H
#define BUFFER_CLEANER_H

#include "ThreadableJob.h"
#include "lmms_basics.h"
#include "engine.h"
#include "Mixer.h"

class BufferCleaner : public ThreadableJob
{
public:
	BufferCleaner( sampleFrame * target, fpp_t frames ) :
		m_target( target ),
		m_fpp( frames )
	{}
	virtual ~BufferCleaner() {}

	virtual bool requiresProcessing() const
	{
		return true;
	}
	
protected:
	virtual void doProcessing( sampleFrame* workingBuffer )
	{
		engine::mixer()->clearAudioBuffer( m_target, m_fpp );
	}
	
private:
	sampleFrame * m_target;
	fpp_t m_fpp;
};

#endif
