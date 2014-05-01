/*
 * ImpulseConvolverControls.h - A native impulse convolver effect plugin for LMMS
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

#ifndef IMPULSE_CONVOLVER_CONTROLS_H
#define IMPULSE_CONVOLVER_CONTROLS_H


#include "lmms_math.h"
#include "EffectControls.h"
#include "ImpulseConvolverControlDialog.h"
#include "knob.h"
#include "SampleBuffer.h"
#include "pixmap_button.h"

const int CONTROL_COUNT = 2;

class ImpulseConvolverEffect;

class ImpulseConvolverControls : public EffectControls
{
	Q_OBJECT
public:
	ImpulseConvolverControls( ImpulseConvolverEffect* effect );
	virtual ~ImpulseConvolverControls() {};

	virtual void saveSettings( QDomDocument & doc, QDomElement & parent );
	virtual void loadSettings( const QDomElement & me );
	virtual void loadFile( const QString & file );
	inline virtual QString nodeName() const
	{
		return "ImpulseConvolverControls";
	}
	virtual int controlCount()
	{
		return CONTROL_COUNT;
	}

	virtual EffectControlDialog* createView()
	{
		return new ImpulseConvolverControlDialog( this );
	}

private slots:
	void changeControl();
	void changeSampleRate();
	void openAudioFile();
	void setAudioFile( const QString & impulse_file );
	void changeImpulse();

private:
	ImpulseConvolverEffect* m_effect;

	typedef SampleBuffer::handleState handleState;

	SampleBuffer m_impulseBuffer;

	FloatModel m_gainModel;
	FloatModel m_freqModel;

	sample_rate_t m_sampleRate;

	friend class ImpulseConvolverEffect;
	friend class ImpulseConvolverControlDialog;
};


#endif
