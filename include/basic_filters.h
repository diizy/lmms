/*
 * basic_filters.h - simple but powerful filter-class with most used filters
 *
 * original file by ???
 * modified and enhanced by Tobias Doerffel
 * 
 * Lowpass_SV code originally from Nekobee, Copyright (C) 2004 Sean Bolton and others
 * adapted & modified for use in LMMS
 *
 * Copyright (c) 2004-2009 Tobias Doerffel <tobydox/at/users.sourceforge.net>
 *
 * This file is part of LMMS - http://lmms.io
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


#ifndef BASIC_FILTERS_H
#define BASIC_FILTERS_H

#ifndef __USE_XOPEN
#define __USE_XOPEN
#endif

#include <math.h>

#include "lmms_basics.h"
#include "Mixer.h"
#include "templates.h"
#include "lmms_constants.h"
#include "interpolation.h"
#include "MemoryManager.h"

//#include <iostream>
//#include <cstdlib>

template<ch_cnt_t CHANNELS/* = DEFAULT_CHANNELS*/>
class basicFilters
{
	MM_OPERATORS
public:
	enum FilterTypes
	{
		LowPass,
		HiPass,
		BandPass_CSG,
		BandPass_CZPG,
		Notch,
		AllPass,
		Moog,
		DoubleLowPass,
		Lowpass_RC12,
		Bandpass_RC12,
		Highpass_RC12,
		Lowpass_RC24,
		Bandpass_RC24,
		Highpass_RC24,
		Formantfilter,
		DoubleMoog,
		Lowpass_SV,
		Bandpass_SV,
		Highpass_SV,
		Notch_SV,
		FastFormant,
		Tripole,
		Hex_LP,
		NumFilters
	};

	static inline float minFreq()
	{
		return( 5.0f );
	}

	static inline float minQ()
	{
		return( 0.01f );
	}

	inline void setFilterType( const int _idx )
	{
		m_doubleFilter = _idx == DoubleLowPass || _idx == DoubleMoog;
		if( !m_doubleFilter )
		{
			m_type = static_cast<FilterTypes>( _idx );
			return;
		}

		// Double lowpass mode, backwards-compat for the goofy
		// Add-NumFilters to signify doubleFilter stuff
		m_type = _idx == DoubleLowPass 
			? LowPass
			: Moog;
		if( m_subFilter == NULL )
		{
			m_subFilter = new basicFilters<CHANNELS>(
						static_cast<sample_rate_t>(
							m_sampleRate ) );
		}
		m_subFilter->m_type = m_type;
	}

	inline basicFilters( const sample_rate_t _sample_rate ) :
		m_doubleFilter( false ),
		m_sampleRate( (float) _sample_rate ),
		m_sampleRatio( 1.0f / m_sampleRate ),
		m_subFilter( NULL )
	{
		m_svsr = 1.0f - expf( -4646.39874051f / m_sampleRate );
		clearHistory();
	}

	inline ~basicFilters()
	{
		delete m_subFilter;
	}

	inline void clearHistory()
	{
		// reset in/out history
		for( ch_cnt_t _chnl = 0; _chnl < CHANNELS; ++_chnl )
		{
			// reset in/out history for simple filters
			m_z1[_chnl] = m_z2[_chnl] = 0.0f;

			// reset in/out history for moog-filter
			m_y1[_chnl] = m_y2[_chnl] = m_y3[_chnl] = m_y4[_chnl] =
					m_oldx[_chnl] = m_oldy1[_chnl] =
					m_oldy2[_chnl] = m_oldy3[_chnl] = 0.0f;
			
			// hex
			m_y5[_chnl] = m_y6[_chnl] = 0.0f;
			
			// tripole
			m_last[_chnl] = 0.0f;

			// reset in/out history for RC-filters
			m_rclp0[_chnl] = m_rcbp0[_chnl] = m_rchp0[_chnl] = m_rclast0[_chnl] = 0.0f;
			m_rclp1[_chnl] = m_rcbp1[_chnl] = m_rchp1[_chnl] = m_rclast1[_chnl] = 0.0f;

			for(int i=0; i<6; i++)
			   m_vfbp[i][_chnl] = m_vfhp[i][_chnl] = m_vflast[i][_chnl] = 0.0f;
			   
			// reset in/out history for SV-filters
			m_delay1[_chnl] = 0.0f;
			m_delay2[_chnl] = 0.0f;
			m_delay3[_chnl] = 0.0f;
			m_delay4[_chnl] = 0.0f;
			m_sva[_chnl] = 0.0f;
		}
	}

	inline sample_t update( sample_t _in0, ch_cnt_t _chnl )
	{
		sample_t out;
		switch( m_type )
		{
			case Moog:
			{
				sample_t x = _in0 - m_r*m_y4[_chnl];

				// four cascaded onepole filters
				// (bilinear transform)
				m_y1[_chnl] = qBound( -10.0f,
						( x + m_oldx[_chnl] ) * m_p
							- m_k * m_y1[_chnl],
								10.0f );
				m_y2[_chnl] = qBound( -10.0f,
					( m_y1[_chnl] + m_oldy1[_chnl] ) * m_p
							- m_k * m_y2[_chnl],
								10.0f );
				m_y3[_chnl] = qBound( -10.0f,
					( m_y2[_chnl] + m_oldy2[_chnl] ) * m_p
							- m_k * m_y3[_chnl],
								10.0f );
				m_y4[_chnl] = qBound( -10.0f,
					( m_y3[_chnl] + m_oldy3[_chnl] ) * m_p
							- m_k * m_y4[_chnl],
								10.0f );

				m_oldx[_chnl] = x;
				m_oldy1[_chnl] = m_y1[_chnl];
				m_oldy2[_chnl] = m_y2[_chnl];
				m_oldy3[_chnl] = m_y3[_chnl];
				out = m_y4[_chnl] - m_y4[_chnl] * m_y4[_chnl] *
						m_y4[_chnl] * ( 1.0f / 6.0f );
				break;
			}
			
			// 3x onepole filters with 4x oversampling and interpolation of oversampled signal:
			// input signal is linear-interpolated after oversampling, output signal is averaged from oversampled outputs
			case Tripole:
			{
				out = 0.0f;
				float ip = 0.0f;
				for( int i = 0; i < 4; ++i )
				{
					ip += 0.25f;
					sample_t x = linearInterpolate( m_last[_chnl], _in0, ip ) - m_r * m_y3[_chnl];
					
					m_y1[_chnl] = qBound( -10.0f,
						( x + m_oldx[_chnl] ) * m_p
							- m_k * m_y1[_chnl],
								10.0f );
					m_y2[_chnl] = qBound( -10.0f,
						( m_y1[_chnl] + m_oldy1[_chnl] ) * m_p
								- m_k * m_y2[_chnl],
									10.0f );
					m_y3[_chnl] = qBound( -10.0f,
						( m_y2[_chnl] + m_oldy2[_chnl] ) * m_p
								- m_k * m_y3[_chnl],
									10.0f );
					m_oldx[_chnl] = x;
					m_oldy1[_chnl] = m_y1[_chnl];
					m_oldy2[_chnl] = m_y2[_chnl];
					
					out += ( m_y3[_chnl] - m_y3[_chnl] * m_y3[_chnl] * m_y3[_chnl] * ( 1.0f / 6.0f ) );
				}
				out *= 0.25f;
				m_last[_chnl] = _in0;
				return out;
				break;
			}
			
			// 4-pole state-variant lowpass filter, adapted from Nekobee source code
			// and extended to other SV filter types
			// /* Hal Chamberlin's state variable filter */
			
			case Lowpass_SV:
			case Bandpass_SV:
			{
				m_sva[_chnl] += ( qAbs( _in0 ) - m_sva[_chnl] ) * m_svsr;
				float highpass;
				
				for( int i = 0; i < 2; ++i ) // 2x oversample
				{
					m_delay2[_chnl] = m_delay2[_chnl] + m_svf1 * m_delay1[_chnl];				/* delay2/4 = lowpass output */
					highpass = _in0 - m_delay2[_chnl] - m_svq * m_delay1[_chnl];
					m_delay1[_chnl] = m_svf1 * highpass + m_delay1[_chnl];           			/* delay1/3 = bandpass output */

					m_delay4[_chnl] = m_delay4[_chnl] + m_svf2 * m_delay3[_chnl];
					highpass = m_delay2[_chnl] - m_delay4[_chnl] - m_svq * m_delay3[_chnl];
					m_delay3[_chnl] = m_svf2 * highpass + m_delay3[_chnl];
				}

				/* mix filter output into output buffer */
				return m_type == Lowpass_SV 
					? atanf( 3.0f * m_delay4[_chnl] * m_sva[_chnl] )
					: atanf( 3.0f * m_delay3[_chnl] * m_sva[_chnl] );
				break;
			}
			
			case Highpass_SV:
			{
				m_sva[_chnl] += ( qAbs( _in0 ) - m_sva[_chnl] ) * m_svsr;
				float hp;

				for( int i = 0; i < 2; ++i ) // 2x oversample
				{				
					m_delay2[_chnl] = m_delay2[_chnl] + m_svf1 * m_delay1[_chnl];
					hp = _in0 - m_delay2[_chnl] - m_svq * m_delay1[_chnl];
					m_delay1[_chnl] = m_svf1 * hp + m_delay1[_chnl];
				}
				
				return atanf( 3.0f * hp * m_sva[_chnl] );
				break;
			}
			
			case Notch_SV:
			{
				m_sva[_chnl] += ( qAbs( _in0 ) - m_sva[_chnl] ) * m_svsr;
				float hp1, hp2;
				
				for( int i = 0; i < 2; ++i ) // 2x oversample
				{
					m_delay2[_chnl] = m_delay2[_chnl] + m_svf1 * m_delay1[_chnl];				/* delay2/4 = lowpass output */
					hp1 = _in0 - m_delay2[_chnl] - m_svq * m_delay1[_chnl];
					m_delay1[_chnl] = m_svf1 * hp1 + m_delay1[_chnl];           			/* delay1/3 = bandpass output */

					m_delay4[_chnl] = m_delay4[_chnl] + m_svf2 * m_delay3[_chnl];
					hp2 = m_delay2[_chnl] - m_delay4[_chnl] - m_svq * m_delay3[_chnl];
					m_delay3[_chnl] = m_svf2 * hp2 + m_delay3[_chnl];
				}

				/* mix filter output into output buffer */
				return atanf( 1.5f * ( m_delay4[_chnl] + hp1 ) * m_sva[_chnl] );
				break;
			}


			// 4-times oversampled simulation of an active RC-Bandpass,-Lowpass,-Highpass-
			// Filter-Network as it was used in nearly all modern analog synthesizers. This
			// can be driven up to self-oscillation (BTW: do not remove the limits!!!).
			// (C) 1998 ... 2009 S.Fendt. Released under the GPL v2.0  or any later version.

			case Lowpass_RC12:
			{
				sample_t lp, bp, hp, in;
				for( int n = 4; n != 0; --n )
				{
					in = _in0 + m_rcbp0[_chnl] * m_rcq;
					in = qBound( -1.0f, in, 1.0f );

					lp = in * m_rcb + m_rclp0[_chnl] * m_rca;
					lp = qBound( -1.0f, lp, 1.0f );

					hp = m_rcc * ( m_rchp0[_chnl] + in - m_rclast0[_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
					bp = qBound( -1.0f, bp, 1.0f );

					m_rclast0[_chnl] = in;
					m_rclp0[_chnl] = lp;
					m_rchp0[_chnl] = hp;
					m_rcbp0[_chnl] = bp;
				}
				return lp;
				break;
			}
			case Highpass_RC12:
			case Bandpass_RC12:
			{
				sample_t hp, bp, in;
				for( int n = 4; n != 0; --n )
				{
					in = _in0 + m_rcbp0[_chnl] * m_rcq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_rcc * ( m_rchp0[_chnl] + in - m_rclast0[_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
					bp = qBound( -1.0f, bp, 1.0f );

					m_rclast0[_chnl] = in;
					m_rchp0[_chnl] = hp;
					m_rcbp0[_chnl] = bp;
				}
				return m_type == Highpass_RC12 ? hp : bp;
				break;
			}

			case Lowpass_RC24:
			{
				sample_t lp, bp, hp, in;
				for( int n = 4; n != 0; --n )
				{
					// first stage is as for the 12dB case...
					in = _in0 + m_rcbp0[_chnl] * m_rcq;
					in = qBound( -1.0f, in, 1.0f );

					lp = in * m_rcb + m_rclp0[_chnl] * m_rca;
					lp = qBound( -1.0f, lp, 1.0f );

					hp = m_rcc * ( m_rchp0[_chnl] + in - m_rclast0[_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
					bp = qBound( -1.0f, bp, 1.0f );

					m_rclast0[_chnl] = in;
					m_rclp0[_chnl] = lp;
					m_rcbp0[_chnl] = bp;
					m_rchp0[_chnl] = hp;

					// second stage gets the output of the first stage as input...
					in = lp + m_rcbp1[_chnl] * m_rcq;
					in = qBound( -1.0f, in, 1.0f );

					lp = in * m_rcb + m_rclp1[_chnl] * m_rca;
					lp = qBound( -1.0f, lp, 1.0f );

					hp = m_rcc * ( m_rchp1[_chnl] + in - m_rclast1[_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_rcb + m_rcbp1[_chnl] * m_rca;
					bp = qBound( -1.0f, bp, 1.0f );

					m_rclast1[_chnl] = in;
					m_rclp1[_chnl] = lp;
					m_rcbp1[_chnl] = bp;
					m_rchp1[_chnl] = hp;
				}
				return lp;
				break;
			}
			case Highpass_RC24:
			case Bandpass_RC24:
			{
				sample_t hp, bp, in;
				for( int n = 4; n != 0; --n )
				{
					// first stage is as for the 12dB case...
					in = _in0 + m_rcbp0[_chnl] * m_rcq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_rcc * ( m_rchp0[_chnl] + in - m_rclast0[_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_rcb + m_rcbp0[_chnl] * m_rca;
					bp = qBound( -1.0f, bp, 1.0f );

					m_rclast0[_chnl] = in;
					m_rchp0[_chnl] = hp;
					m_rcbp0[_chnl] = bp;

					// second stage gets the output of the first stage as input...
					in = m_type == Highpass_RC24
						? hp + m_rcbp1[_chnl] * m_rcq
						: bp + m_rcbp1[_chnl] * m_rcq;

					in = qBound( -1.0f, in, 1.0f );

					hp = m_rcc * ( m_rchp1[_chnl] + in - m_rclast1[_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_rcb + m_rcbp1[_chnl] * m_rca;
					bp = qBound( -1.0f, bp, 1.0f );

					m_rclast1[_chnl] = in;
					m_rchp1[_chnl] = hp;
					m_rcbp1[_chnl] = bp;
				}
				return m_type == Highpass_RC24 ? hp : bp;
				break;
			}

			case Formantfilter:
			case FastFormant:
			{
				sample_t hp, bp, in;

				out = 0;
				const int os = m_type == FastFormant ? 1 : 4; // no oversampling for fast formant
				for( int o = 0; o < os; ++o )
				{
					// first formant
					in = _in0 + m_vfbp[0][_chnl] * m_vfq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_vfc[0] * ( m_vfhp[0][_chnl] + in - m_vflast[0][_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_vfb[0] + m_vfbp[0][_chnl] * m_vfa[0];
					bp = qBound( -1.0f, bp, 1.0f );

					m_vflast[0][_chnl] = in;
					m_vfhp[0][_chnl] = hp;
					m_vfbp[0][_chnl] = bp;

					in = bp + m_vfbp[2][_chnl] * m_vfq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_vfc[0] * ( m_vfhp[2][_chnl] + in - m_vflast[2][_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_vfb[0] + m_vfbp[2][_chnl] * m_vfa[0];
					bp = qBound( -1.0f, bp, 1.0f );

					m_vflast[2][_chnl] = in;
					m_vfhp[2][_chnl] = hp;
					m_vfbp[2][_chnl] = bp;

					in = bp + m_vfbp[4][_chnl] * m_vfq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_vfc[0] * ( m_vfhp[4][_chnl] + in - m_vflast[4][_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_vfb[0] + m_vfbp[4][_chnl] * m_vfa[0];
					bp = qBound( -1.0f, bp, 1.0f );

					m_vflast[4][_chnl] = in;
					m_vfhp[4][_chnl] = hp;
					m_vfbp[4][_chnl] = bp;

					out += bp;

					// second formant
					in = _in0 + m_vfbp[0][_chnl] * m_vfq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_vfc[1] * ( m_vfhp[1][_chnl] + in - m_vflast[1][_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_vfb[1] + m_vfbp[1][_chnl] * m_vfa[1];
					bp = qBound( -1.0f, bp, 1.0f );

					m_vflast[1][_chnl] = in;
					m_vfhp[1][_chnl] = hp;
					m_vfbp[1][_chnl] = bp;

					in = bp + m_vfbp[3][_chnl] * m_vfq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_vfc[1] * ( m_vfhp[3][_chnl] + in - m_vflast[3][_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_vfb[1] + m_vfbp[3][_chnl] * m_vfa[1];
					bp = qBound( -1.0f, bp, 1.0f );

					m_vflast[3][_chnl] = in;
					m_vfhp[3][_chnl] = hp;
					m_vfbp[3][_chnl] = bp;

					in = bp + m_vfbp[5][_chnl] * m_vfq;
					in = qBound( -1.0f, in, 1.0f );

					hp = m_vfc[1] * ( m_vfhp[5][_chnl] + in - m_vflast[5][_chnl] );
					hp = qBound( -1.0f, hp, 1.0f );

					bp = hp * m_vfb[1] + m_vfbp[5][_chnl] * m_vfa[1];
					bp = qBound( -1.0f, bp, 1.0f );

					m_vflast[5][_chnl] = in;
					m_vfhp[5][_chnl] = hp;
					m_vfbp[5][_chnl] = bp;

					out += bp;
				}
            	return m_type == FastFormant ? out * 2.0f : out * 0.5f;
				break;
			}
			
			case Hex_LP:
			{
				out = 0.0f;
				// 2x oversample
				for( int i = 0; i < 2; ++i )
				{
					// 2x onepole lowpasses
					float r = qBound( -1.0f, m_y6[_chnl] * m_q, 1.0f );
					m_y1[_chnl] = ( _in0 + r ) * m_la + m_y1[_chnl] * m_lb;
					m_y2[_chnl] = ( m_y1[_chnl] + r ) * m_la + m_y2[_chnl] * m_lb;

					// 4x onepole highpasses for resonance					
					m_y3[_chnl] = m_y2[_chnl] * m_ha + m_y3[_chnl] * m_hb;
					m_y4[_chnl] = m_y3[_chnl] * m_ha + m_y4[_chnl] * m_hb;
					m_y5[_chnl] = m_y4[_chnl] * m_ha + m_y5[_chnl] * m_hb;
					m_y6[_chnl] = m_y5[_chnl] * m_ha + m_y6[_chnl] * m_hb;
					
					out += m_y2[_chnl];
				}
				return out * 0.5f;
				break;
			}

			default:
				// biquad filter in transposed form
				out = m_z1[_chnl] + m_b0a0 * _in0;
				m_z1[_chnl] = m_b1a0 * _in0 + m_z2[_chnl] - m_a1a0 * out;
				m_z2[_chnl] = m_b2a0 * _in0 - m_a2a0 * out;
				break;
		}

		if( m_doubleFilter )
		{
			return m_subFilter->update( out, _chnl );
		}

		// Clipper band limited sigmoid
		return out;
	}


	inline void calcFilterCoeffs( float _freq, float _q )
	{
		// temp coef vars
		_q = qMax( _q, minQ() );

		if( m_type == Lowpass_RC12  ||
			m_type == Bandpass_RC12 ||
			m_type == Highpass_RC12 ||
			m_type == Lowpass_RC24 ||
			m_type == Bandpass_RC24 ||
			m_type == Highpass_RC24 )
		{
			_freq = qBound( 50.0f, _freq, 20000.0f );
			const float sr = m_sampleRatio * 0.25f;
			const float f = 1.0f / ( _freq * F_2PI );
			
			m_rca = 1.0f - sr / ( f + sr );
			m_rcb = 1.0f - m_rca;
			m_rcc = f / ( f + sr );

			// Stretch Q/resonance, as self-oscillation reliably starts at a q of ~2.5 - ~2.6
			m_rcq = _q * 0.25f;
			return;
		}

		if( m_type == Formantfilter ||
			m_type == FastFormant )
		{
			_freq = qBound( minFreq(), _freq, 20000.0f ); // limit freq and q for not getting bad noise out of the filter...

			// formats for a, e, i, o, u, a
			static const float _f[6][2] = { { 1000, 1400 }, { 500, 2300 },
							{ 320, 3200 },
							{ 500, 1000 },
							{ 320, 800 },
							{ 1000, 1400 } };
			static const float freqRatio = 4.0f / 14000.0f;

			// Stretch Q/resonance
			m_vfq = _q * 0.25f;

			// frequency in lmms ranges from 1Hz to 14000Hz
			const float vowelf = _freq * freqRatio;
			const int vowel = static_cast<int>( vowelf );
			const float fract = vowelf - vowel;

			// interpolate between formant frequencies
			const float f0 = 1.0f / ( linearInterpolate( _f[vowel+0][0], _f[vowel+1][0], fract ) * F_2PI );
			const float f1 = 1.0f / ( linearInterpolate( _f[vowel+0][1], _f[vowel+1][1], fract ) * F_2PI );

			// samplerate coeff: depends on oversampling
			const float sr = m_type == FastFormant ? m_sampleRatio : m_sampleRatio * 0.25f;

			m_vfa[0] = 1.0f - sr / ( f0 + sr );
			m_vfb[0] = 1.0f - m_vfa[0];
			m_vfc[0] = f0 /	( f0 + sr );
			m_vfa[1] = 1.0f - sr / ( f1 + sr );
			m_vfb[1] = 1.0f - m_vfa[1];
			m_vfc[1] = f1 /	( f1 + sr );
			return;
		}

		if( m_type == Moog ||
			m_type == DoubleMoog )
		{
			// [ 0 - 0.5 ]
			const float f = qBound( minFreq(), _freq, 20000.0f ) * m_sampleRatio;
			// (Empirical tunning)
			m_p = ( 3.6f - 3.2f * f ) * f;
			m_k = 2.0f * m_p - 1;
			m_r = _q * powf( F_E, ( 1 - m_p ) * 1.386249f );

			if( m_doubleFilter )
			{
				m_subFilter->m_r = m_r;
				m_subFilter->m_p = m_p;
				m_subFilter->m_k = m_k;
			}
			return;
		}
		
		if( m_type == Tripole )
		{
			const float f = qBound( 20.0f, _freq, 20000.0f ) * m_sampleRatio * 0.25f;
			
			m_p = ( 3.6f - 3.2f * f ) * f;
			m_k = 2.0f * m_p - 1.0f;
			m_r = _q * 0.1f * powf( F_E, ( 1 - m_p ) * 1.386249f );
			
			return;
		}

		if( m_type == Lowpass_SV || 
			m_type == Bandpass_SV ||
			m_type == Highpass_SV ||
			m_type == Notch_SV )
		{
			const float f = sinf( qMax( minFreq(), _freq ) * m_sampleRatio * F_PI );
			m_svf1 = qMin( f, 0.825f );
			m_svf2 = qMin( f * 2.0f, 0.825f );
			m_svq = qMax( 0.0001f, 2.0f - ( _q * 0.1995f ) );
			return;
		}
		
		if( m_type == Hex_LP )
		{
			const float f = qBound( 20.0f, _freq, 20000.0f ) * m_sampleRatio;
			
			m_q = _q * 0.314f;
			
			m_lb = expf( -1.0f * F_PI * f );
			m_la = 1.0f - m_lb;
			
			m_hb = -expf( -1.0f * F_PI * ( 0.5f - f * 0.995f ) );
			m_ha = 1.0f + m_hb;
			return;
		}

		// other filters
		_freq = qBound( minFreq(), _freq, 20000.0f );
		const float omega = F_2PI * _freq * m_sampleRatio;
		const float tsin = sinf( omega ) * 0.5f;
		const float tcos = cosf( omega );

		const float alpha = tsin / _q;

		const float a0 = 1.0f / ( 1.0f + alpha );

		m_a1a0 = -2.0f * tcos * a0;
		m_a2a0 = ( 1.0f - alpha ) * a0;

		switch( m_type )
		{
			case LowPass:
				m_b1a0 = ( 1.0f - tcos ) * a0;
				m_b0a0 = m_b1a0 * 0.5f;
				m_b2a0 = m_b0a0;//((1.0f-tcos)/2.0f)*a0;
				break;
			case HiPass:
				m_b1a0 = ( -1.0f - tcos ) * a0;
				m_b0a0 = m_b1a0 * -0.5f;
				m_b2a0 = m_b0a0;//((1.0f+tcos)/2.0f)*a0;
				break;
			case BandPass_CSG:
				m_b1a0 = 0.0f;
				m_b0a0 = tsin * a0;
				m_b2a0 = -m_b0a0;
				break;
			case BandPass_CZPG:
				m_b1a0 = 0.0f;
				m_b0a0 = alpha * a0;
				m_b2a0 = -m_b0a0;
				break;
			case Notch:
				m_b1a0 = m_a1a0;
				m_b0a0 = a0;
				m_b2a0 = a0;
				break;
			case AllPass:
				m_b1a0 = m_a1a0;
				m_b0a0 = m_a2a0;
				m_b2a0 = 1.0f;//(1.0f+alpha)*a0;
				break;
			default:
				break;
		}

		if( m_doubleFilter )
		{
			m_subFilter->m_b0a0 = m_b0a0;
			m_subFilter->m_b1a0 = m_b1a0;
			m_subFilter->m_b2a0 = m_b2a0;
			m_subFilter->m_a1a0 = m_a1a0;
			m_subFilter->m_a2a0 = m_a2a0;
		}
	}


private:
	// filter coeffs
	float m_b0a0, m_b1a0, m_b2a0, m_a1a0, m_a2a0;

	// coeffs for moog-filter
	float m_r, m_p, m_k;
	
	// coeffs for hex
	float m_la, m_lb, m_ha, m_hb, m_q;

	// coeffs for RC-type-filters
	float m_rca, m_rcb, m_rcc, m_rcq;

	// coeffs for formant-filters
	float m_vfa[4], m_vfb[4], m_vfc[4], m_vfq;

	// coeffs for Lowpass_SV (state-variant lowpass)
	float m_svf1, m_svf2, m_svq, m_svsr;

	typedef sample_t frame[CHANNELS];

	// in/out history
	frame m_z1, m_z2;

	// in/out history for moog-filter
	frame m_y1, m_y2, m_y3, m_y4, m_oldx, m_oldy1, m_oldy2, m_oldy3;
	// additional one for Tripole filter
	frame m_last;
	// additional ones for hex filters
	frame m_y5, m_y6;

	// in/out history for RC-type-filters
	frame m_rcbp0, m_rclp0, m_rchp0, m_rclast0;
	frame m_rcbp1, m_rclp1, m_rchp1, m_rclast1;

	// in/out history for Formant-filters
	frame m_vfbp[6], m_vfhp[6], m_vflast[6];

	// in/out history for Lowpass_SV (state-variant lowpass)
	frame m_delay1, m_delay2, m_delay3, m_delay4, m_sva;

	FilterTypes m_type;
	bool m_doubleFilter;

	float m_sampleRate;
	float m_sampleRatio;
	basicFilters<CHANNELS> * m_subFilter;

} ;


#endif
