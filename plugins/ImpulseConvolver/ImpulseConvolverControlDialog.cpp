/*
 * ImpulseConvolverControlDialog.cpp - A native impulse convolver effect plugin for LMMS
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

#include <QtGui/QLayout>
#include <QPainter>

#include "ImpulseConvolverControlDialog.h"
#include "ImpulseConvolverControls.h"
#include "embed.h"
#include "tooltip.h"
#include "gui_templates.h"

QPixmap * ImpulseConvolverControlDialog::s_artwork = NULL;

ImpulseConvolverControlDialog::ImpulseConvolverControlDialog( ImpulseConvolverControls* controls ):
	EffectControlDialog( controls )
{	
	if( s_artwork == NULL ) 
	{
		s_artwork = new QPixmap( PLUGIN_NAME::getIconPixmap( "artwork" ) );
	}
	
	setFixedSize( 230, 50 );

	knob * gainKnob = new knob( knobBright_26, this );
	gainKnob -> move( 2, 8 );
	gainKnob -> setVolumeKnob( true );
	gainKnob->setModel( &controls->m_gainModel );
	gainKnob->setLabel( tr( "GAIN" ) );
	gainKnob->setHintText( tr( "Gain:" ) + " ", "%" );

	knob * freqKnob = new knob( knobBright_26, this );
	freqKnob -> move( 32, 8 );
	freqKnob->setModel( &controls->m_freqModel );
	freqKnob->setLabel( tr( "FREQ" ) );
	freqKnob->setHintText( tr( "Impulse frequency adjustment:" ) + " ", " oct." );

	pixmapButton * openFileButton = new pixmapButton( this );
	openFileButton->setCursor( QCursor( Qt::PointingHandCursor ) );
	openFileButton->move( 207, 16 );
	openFileButton->setActiveGraphic( PLUGIN_NAME::getIconPixmap(
							"openfile_active" ) );
	openFileButton->setInactiveGraphic( PLUGIN_NAME::getIconPixmap(
							"openfile_inactive" ) );
	connect( openFileButton, SIGNAL( clicked() ), controls, SLOT( openAudioFile() ) );
	connect( &controls->m_impulseBuffer, SIGNAL( sampleUpdated() ), this,  SLOT( impulseUpdated() ) );
	toolTip::add( openFileButton, tr( "Load a sample from disk to use as impulse" ) );

}



void ImpulseConvolverControlDialog::paintEvent( QPaintEvent * )
{
	QPainter p( this );

	p.drawPixmap( 0, 0, *s_artwork );

	ImpulseConvolverControls * i = castModel<ImpulseConvolverControls>();

 	QString file_name = "";
	int idx = i->m_impulseBuffer.audioFile().length();

	p.setFont( pointSize<8>( font() ) );

	QFontMetrics fm( p.font() );

	// simple algorithm for creating a text from the filename that
	// matches in the white rectangle
	while( idx > 0 &&
		fm.size( Qt::TextSingleLine, file_name + "..." ).width() < 136 )
	{
		file_name = i->m_impulseBuffer.audioFile()[--idx] + file_name;
	}

	if( idx > 0 )
	{
		file_name = "..." + file_name;
	}

	p.setPen( QColor( 255, 255, 255 ) );
	p.drawText( 69, 31, file_name );
}


void ImpulseConvolverControlDialog::impulseUpdated()
{
	update();
}



#include "moc_ImpulseConvolverControlDialog.cxx"
