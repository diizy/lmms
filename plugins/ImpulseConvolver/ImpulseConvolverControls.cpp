/*
 * ImpulseConvolverControls.cpp - A native impulse convolver effect plugin for LMMS
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

#include <QtXml/QDomElement>
#include <QtXml/QDomDocument>
#include <QtCore/QFileInfo>

#include "ImpulseConvolverControls.h"
#include "ImpulseConvolver.h"
#include "engine.h"
#include "song.h"



ImpulseConvolverControls::ImpulseConvolverControls( ImpulseConvolverEffect* effect ) :
	EffectControls( effect ),
	m_effect( effect ),
	m_impulseBuffer( ),
	m_gainModel( 50.0f, 0.0f, 100.0f, 0.01f, this, tr( "Gain" ) ),
	m_freqModel( 0.0f, -2.0f, 2.0f, 1.0f, this, tr( "Impulse frequency" ) ),
	m_sampleRate( engine::mixer()->processingSampleRate() )
{
	m_gainModel.setScaleLogarithmic( true );
	connect( &m_gainModel, SIGNAL( dataChanged() ), this, SLOT( changeControl() ) );
	connect( &m_freqModel, SIGNAL( dataChanged() ), this, SLOT( changeImpulse() ) );
	connect( engine::mixer(), SIGNAL( sampleRateChanged() ), this, SLOT( changeSampleRate() ) );

	changeControl();
}


void ImpulseConvolverControls::saveSettings( QDomDocument & doc, QDomElement & parent )
{
	parent.setAttribute( "file", m_impulseBuffer.audioFile() );
	m_gainModel.saveSettings( doc, parent, "gain" );
}


void ImpulseConvolverControls::loadSettings( const QDomElement & me )
{
	setAudioFile( me.attribute( "file" ) );
	m_gainModel.loadSettings( me, "gain" );
}


void ImpulseConvolverControls::loadFile( const QString & file )
{
	setAudioFile( file );
}


void ImpulseConvolverControls::changeSampleRate()
{
	m_sampleRate = engine::mixer()->processingSampleRate();
	m_impulseBuffer.normalizeSampleRate( m_sampleRate, true );
}


void ImpulseConvolverControls::changeControl()
{
}


void ImpulseConvolverControls::openAudioFile()
{
	QString af = m_impulseBuffer.openAudioFile();
	if( af != "" )
	{
		setAudioFile( af );
		engine::getSong()->setModified();
	}
}


void ImpulseConvolverControls::setAudioFile( const QString & impulse_file )
{
	m_impulseBuffer.setAudioFile( impulse_file );
	changeImpulse();
}


void ImpulseConvolverControls::changeImpulse()
{
	m_effect->updateImpulse();
}


#include "moc_ImpulseConvolverControls.cxx"
