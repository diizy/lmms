/*
 * ImpulseConvolver.cpp - A native impulse convolver effect plugin for LMMS
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2006-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 * 
 * Some of the code adapted from / inspired by the 
 * impulse convolver plugin "imp_1199" by Steve Harris <steve/at/plugin.org.uk>
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

#include "ImpulseConvolver.h"

#include "embed.cpp"


extern "C"
{

Plugin::Descriptor PLUGIN_EXPORT impulse_convolver_plugin_descriptor =
{
	STRINGIFY( PLUGIN_NAME ),
	"Impulse Convolver",
	QT_TRANSLATE_NOOP( "pluginBrowser", "A native impulse convolver" ),
	"Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>",
	0x0100,
	Plugin::Effect,
	new PluginPixmapLoader( "logo" ),
	"wav,ogg,ds,spx,au,voc,aif,aiff,flac,raw",
	NULL
};

}

ImpulseConvolverEffect::ImpulseConvolverEffect( Model* parent, const Descriptor::SubPluginFeatures::Key* key ) :
	Effect( &impulse_convolver_plugin_descriptor, parent, key ),
	m_icControls( this )
{
	m_count = 0;
	m_in_ptr = 0;
	m_out_ptr = 0;
	
	m_leftImp = new ImpulseData();
	m_rightImp = new ImpulseData();
	
	updateImpulse();
}


void ImpulseConvolverEffect::updateImpulse()
{

	if( m_icControls.m_impulseBuffer.frames() > 0 )
	{
		const float freq_ratio = 2.0f * 44100.0f / m_icControls.m_sampleRate * powf( 2.0f, m_icControls.m_freqModel.value() );
		
		int real_imp_len = m_icControls.m_impulseBuffer.frames();
		m_impLength = qMin( static_cast<int>( real_imp_len / freq_ratio ), MAX_FFT_SIZE - SEG_SIZE );
		
		const sampleFrame * imp = m_icControls.m_impulseBuffer.data();
		
		float impulse_time[ MAX_FFT_SIZE ];
		fftwf_plan tmp_plan;
		
		unsigned int fftl = 128;
		unsigned int i, j;
		
		while( fftl < m_impLength + SEG_SIZE )
		{
			fftl <<= 1;
		}
		m_fftLength = fftl;
		
	// do left	
		m_leftPlanRc = fftwf_plan_r2r_1d( fftl, m_leftImp->m_blockTime, m_leftImp->m_blockFreq, FFTW_R2HC, FFTW_MEASURE );
		m_leftPlanCr = fftwf_plan_r2r_1d( fftl, m_leftImp->m_blockFreq, m_leftImp->m_op, FFTW_HC2R, FFTW_MEASURE );
		tmp_plan = fftwf_plan_r2r_1d( fftl, impulse_time, m_leftImp->m_impulseFreq, FFTW_R2HC, FFTW_MEASURE );	
		for ( i=0; i < fftl; i++ ) 
		{
			j = i * freq_ratio;
			if( j < real_imp_len )
			{
				impulse_time[i] = imp[j][0];
			}
			else
			{
				impulse_time[i] = 0.0f;
			}
		}
		fftwf_execute( tmp_plan );
		fftwf_destroy_plan( tmp_plan );
		
	// do right	
		m_rightPlanRc = fftwf_plan_r2r_1d( fftl, m_rightImp->m_blockTime, m_rightImp->m_blockFreq, FFTW_R2HC, FFTW_MEASURE );
		m_rightPlanCr = fftwf_plan_r2r_1d( fftl, m_rightImp->m_blockFreq, m_rightImp->m_op, FFTW_HC2R, FFTW_MEASURE );
		tmp_plan = fftwf_plan_r2r_1d(fftl, impulse_time, m_rightImp->m_impulseFreq, FFTW_R2HC, FFTW_MEASURE );		
		for ( i=0; i < fftl; i++ ) 
		{
			j = i * freq_ratio;
			if( j < real_imp_len )
			{
				impulse_time[i] = imp[j][1];
			}
			else
			{
				impulse_time[i] = 0.0f;
			}
		}
		fftwf_execute( tmp_plan );
		fftwf_destroy_plan( tmp_plan );		
	}

	// left
	memset( m_leftImp->m_blockTime, 0, MAX_FFT_SIZE * sizeof( float ) );
	memset( m_leftImp->m_blockFreq, 0, MAX_FFT_SIZE * sizeof( float ) );
	memset( m_leftImp->m_op, 0, MAX_FFT_SIZE * sizeof( float ) );
	memset( m_leftImp->m_overlap, 0, ( MAX_FFT_SIZE - SEG_SIZE ) * sizeof( float ) );
	memset( m_leftImp->m_opc, 0, SEG_SIZE * sizeof( float ) );
	
	// right
	memset( m_rightImp->m_blockTime, 0, MAX_FFT_SIZE * sizeof( float ) );
	memset( m_rightImp->m_blockFreq, 0, MAX_FFT_SIZE * sizeof( float ) );
	memset( m_rightImp->m_op, 0, MAX_FFT_SIZE * sizeof( float ) );
	memset( m_rightImp->m_overlap, 0, ( MAX_FFT_SIZE - SEG_SIZE ) * sizeof( float ) );
	memset( m_rightImp->m_opc, 0, SEG_SIZE * sizeof( float ) );


	m_count = 0;
	m_in_ptr = 0;
	m_out_ptr = 0;	
}




ImpulseConvolverEffect::~ImpulseConvolverEffect()
{
	delete m_leftImp;
	delete m_rightImp;
}




bool ImpulseConvolverEffect::processAudioBuffer( sampleFrame* buf, const fpp_t frames )
{
	if( !isEnabled() || !isRunning () )
	{
		return( false );
	}
	double outSum = 0.0;
	if( m_icControls.m_impulseBuffer.frames() <= 0 )
	{
		// if we have no impulse, we'll just keep the buffer as it is
		// and only measure the outsum and checkgate
		for( int f = 0; f < frames; f++ )
		{
			outSum += buf[f][0]*buf[f][0] + buf[f][1]*buf[f][1];
		}
		checkGate( outSum / frames );
		return isRunning();
	}

	const float d = dryLevel();
	const float w = wetLevel();
	
	const float gain = m_icControls.m_gainModel.value() * 0.01f;
	
	unsigned int count = m_count;
	unsigned long out_ptr = m_out_ptr;
	unsigned long in_ptr = m_in_ptr;
	
	sampleFrame output;
	
	unsigned long i, pos, ipos, limit;
	unsigned int len;
	float tmp;
	
	float coef = gain / static_cast<float>( m_fftLength );
	float * left_imp_freq = m_leftImp->m_impulseFreq;
	float * right_imp_freq = m_rightImp->m_impulseFreq;
	
	for ( pos = 0; pos < frames; pos += SEG_SIZE ) 
	{
		limit = pos + SEG_SIZE;
		for ( ipos = pos; ipos < frames && ipos < limit; ipos++ ) 
		{
			m_leftImp->m_blockTime[ in_ptr ] = buf[ ipos ][0];
			m_rightImp->m_blockTime[ in_ptr ] = buf[ ipos ][1];
			in_ptr++;
			if( in_ptr == SEG_SIZE )
			{
				fftwf_execute( m_leftPlanRc );
				fftwf_execute( m_rightPlanRc );
				len = m_fftLength;
				for ( i = 1; i < m_fftLength / 2; i++ ) 
				{
					len--;
					// left
					tmp = m_leftImp->m_blockFreq[i] * left_imp_freq[i] - m_leftImp->m_blockFreq[len] * left_imp_freq[len];
					m_leftImp->m_blockFreq[len] = m_leftImp->m_blockFreq[i] * left_imp_freq[len] +
						m_leftImp->m_blockFreq[len] * left_imp_freq[i];
					m_leftImp->m_blockFreq[i] = tmp;
					// right
					tmp = m_rightImp->m_blockFreq[i] * right_imp_freq[i] - m_rightImp->m_blockFreq[len] * right_imp_freq[len];
					m_rightImp->m_blockFreq[len] = m_rightImp->m_blockFreq[i] * right_imp_freq[len] +
						m_rightImp->m_blockFreq[len] * right_imp_freq[i];
					m_rightImp->m_blockFreq[i] = tmp;
				}
				
				m_leftImp->m_blockFreq[0] = left_imp_freq[0] * m_leftImp->m_blockFreq[0];
				m_rightImp->m_blockFreq[0] = right_imp_freq[0] * m_rightImp->m_blockFreq[0];
				
				fftwf_execute( m_leftPlanCr );
				fftwf_execute( m_rightPlanCr );
				
				for ( i = 0; i < m_fftLength - SEG_SIZE; i++ ) 
				{
					m_leftImp->m_op[i] += m_leftImp->m_overlap[i];
					m_rightImp->m_op[i] += m_rightImp->m_overlap[i];
				}
				for ( i = SEG_SIZE; i < m_fftLength; i++ ) 
				{
					m_leftImp->m_overlap[ i - SEG_SIZE ] = m_leftImp->m_op[i];
					m_rightImp->m_overlap[ i - SEG_SIZE ] = m_rightImp->m_op[i];
				}
				
				in_ptr = 0;
				if( count == 0 )
				{
					count = 1;
					m_count = 1;
					out_ptr = 0;
				}
			}
		}
		// write buffer
		for ( ipos = pos; ipos < frames && ipos < limit; ipos++ ) 
		{
			output[0] = m_leftImp->m_opc[ out_ptr ] * coef;
			output[1] = m_rightImp->m_opc[ out_ptr ] * coef;
			out_ptr++;
			
			// wet/dry
			buf[ipos][0] = d * buf[ipos][0] + w * output[0];
			buf[ipos][1] = d * buf[ipos][1] + w * output[1];
			
			// output checksum
			outSum += buf[ipos][0]*buf[ipos][0] + buf[ipos][1]*buf[ipos][1];

			if ( out_ptr == SEG_SIZE ) 
			{
				memcpy( m_leftImp->m_opc, m_leftImp->m_op, SEG_SIZE * sizeof( float ) );
				memcpy( m_rightImp->m_opc, m_rightImp->m_op, SEG_SIZE * sizeof( float ) );
				/*for ( i = 0; i < SEG_SIZE; i++ ) 
				{
					m_leftImp->m_opc[i] = m_leftImp->m_op[i];
					m_rightImp->m_opc[i] = m_rightImp->m_op[i];
				}*/
				out_ptr = 0;
			}
		}
	}
	
	m_in_ptr = in_ptr;
	m_out_ptr = out_ptr;
	
	checkGate( outSum / frames );

	return isRunning();
}




extern "C"
{

// necessary for getting instance out of shared lib
Plugin * PLUGIN_EXPORT lmms_plugin_main( Model* parent, void* data )
{
	return new ImpulseConvolverEffect( parent, static_cast<const Plugin::Descriptor::SubPluginFeatures::Key *>( data ) );
}

}

