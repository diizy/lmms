/*
 * ImpulseConvolver.h - A native impulse convolver effect plugin for LMMS
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
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
 
#ifndef IMPULSE_CONVOLVER_H
#define IMPULSE_CONVOLVER_H

#include <fftw3.h>
#include "Effect.h"
#include "ImpulseConvolverControls.h"

const int MAX_FFT_SIZE = 1 << 14;
const int SEG_SIZE = 256;

class ImpulseConvolverEffect : public Effect
{
public:
	ImpulseConvolverEffect( Model* parent, const Descriptor::SubPluginFeatures::Key* key );
	virtual ~ImpulseConvolverEffect();
	virtual bool processAudioBuffer( sampleFrame* buf, const fpp_t frames );

	virtual EffectControls* controls()
	{
		return &m_icControls;
	}
	
	struct ImpulseData
	{
		ImpulseData()
		{
			m_impulseFreq = static_cast<float*>( fftwf_malloc( MAX_FFT_SIZE * sizeof( float ) ) );
			m_blockFreq = static_cast<float*>( fftwf_malloc( MAX_FFT_SIZE * sizeof( float ) ) );
			m_blockTime = static_cast<float*>( fftwf_malloc( MAX_FFT_SIZE * sizeof( float ) ) );
			m_op = static_cast<float*>( fftwf_malloc( MAX_FFT_SIZE * sizeof( float ) ) );
			m_overlap = static_cast<float*>( fftwf_malloc( MAX_FFT_SIZE * sizeof( float ) ) );
			m_opc = static_cast<float*>( fftwf_malloc( MAX_FFT_SIZE * sizeof( float ) ) );
		}
		virtual ~ImpulseData() 
		{
			fftwf_free( m_impulseFreq );
			fftwf_free( m_blockFreq );
			fftwf_free( m_blockTime );
			fftwf_free( m_op );
			fftwf_free( m_overlap );
			fftwf_free( m_opc );
		}
		float * m_impulseFreq;
		float * m_blockFreq;
		float * m_blockTime;
		float * m_op;
		float * m_overlap;
		float * m_opc;
	};

protected:
	void updateImpulse();

private:
	unsigned int m_count;
	unsigned long m_in_ptr;
	unsigned long m_out_ptr;

	ImpulseData * m_leftImp;
	ImpulseData * m_rightImp;

	f_cnt_t m_impLength;
	unsigned int m_fftLength;
	
	fftwf_plan m_leftPlanRc;
	fftwf_plan m_leftPlanCr;

	fftwf_plan m_rightPlanRc;
	fftwf_plan m_rightPlanCr;


	ImpulseConvolverControls m_icControls;

	friend class ImpulseConvolverControls;

} ;

#endif

