/*
 * DenominatorSpinBox.h - classes DenominatorSpinBox, DenominatorModel for time signatures
 *
 * Copyright (c) 2014 Vesa Kivimäki <contact/dot/diizy/at/nbl/dot/fi>
 * Copyright (c) 2005-2014 Tobias Doerffel <tobydox/at/users.sourceforge.net>
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
 
#ifndef DENOMINATOR_H
#define DENOMINATOR_H
 
#include "AutomatableModel.h"
#include "LcdSpinBox.h"

const int VALID_VALUES[6] = { 1, 2, 4, 8, 16, 32 };

class DenominatorModel: public IntModel
{
	Q_OBJECT
public:
	DenominatorModel( Model * parent = NULL, int val = 4, const QString& displayName = QString(),
				bool defaultConstructed = false ) : 
	IntModel( val, 1, 32, parent, displayName, defaultConstructed )
	{
	}
	virtual ~DenominatorModel()
	{
	}
	
	static int checkValidValue( int val )
	{
		for( int i = 0; i < 6; i ++ )
		{
			if( val == VALID_VALUES[i] ) { return val; }
		}
		for( int i = 5; i >= 0; i -- )
		{
			if( val > VALID_VALUES[i] ) { return VALID_VALUES[ i ]; }
		}
		return VALID_VALUES[0];
	}


	int value( int frameOffset = 0 ) const
	{
		return checkValidValue( IntModel::value( frameOffset ) );
	}
	
	int initValue() const
	{
		return checkValidValue( IntModel::initValue() );
	}
	
	int minValue() const
	{
		return 1;
	}
	
	int maxValue() const
	{
		return 32;
	}
};


class DenominatorSpinBox : public LcdSpinBox
{
	Q_OBJECT
public:
	DenominatorSpinBox( QWidget* parent, const QString& name = QString::null );
	virtual ~DenominatorSpinBox() {}
	
protected:
	virtual void mouseMoveEvent( QMouseEvent * _me );
	virtual void wheelEvent( QWheelEvent * _we );
	virtual void mouseDoubleClickEvent( QMouseEvent * _me );
	virtual void mousePressEvent( QMouseEvent * _me );
	virtual void mouseReleaseEvent( QMouseEvent * _me );
			
	void enterDenominatorValue();

private:
	bool m_mouseMoving;
	QPoint m_origMousePos;
	int m_displayOffset;

signals:
	void manualChange();	
}; 
 
#endif
 
