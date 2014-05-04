/*
 * Denominator.cpp - implementation for DenominatorSpinBox for time signatures
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

#include <QtGui/QInputDialog>
#include <QtGui/QMouseEvent>
#include <QtGui/QApplication>
 
#include "DenominatorSpinBox.h"
#include "engine.h"
#include "MainWindow.h" 
 
DenominatorSpinBox::DenominatorSpinBox( QWidget* parent, const QString& name ) :
	LcdSpinBox( 2, parent, name )
{
}


void DenominatorSpinBox::mouseMoveEvent( QMouseEvent * event )
{
	if( m_mouseMoving )
	{
		int dy = event->globalY() - m_origMousePos.y();
		if( engine::mainWindow()->isShiftPressed() )
			dy = qBound( -4, dy/4, 4 );
		if( dy > 1 || dy < -1 )
		{
			int val = model()->value();
			val = DenominatorModel::checkValidValue( dy > 0 ? val * 2 : val / 2 );
			model()->setInitValue( val );
			emit manualChange();
			QCursor::setPos( m_origMousePos );
		}
	}	
}


void DenominatorSpinBox::wheelEvent( QWheelEvent * _we )
{
	_we->accept();
	int val = model()->value();
	val = DenominatorModel::checkValidValue( _we->delta() > 0 ? val * 2 : val / 2 );
	model()->setInitValue( val );
	emit manualChange();
}


void DenominatorSpinBox::mouseDoubleClickEvent( QMouseEvent * )
{
	enterDenominatorValue();
}


void DenominatorSpinBox::mousePressEvent( QMouseEvent* event )
{
	if( event->button() == Qt::LeftButton &&
		! ( event->modifiers() & Qt::ControlModifier ) &&
						event->y() < cellHeight() + 2  )
	{
		m_mouseMoving = true;
		m_origMousePos = event->globalPos();
		QApplication::setOverrideCursor( Qt::BlankCursor );

		AutomatableModel *thisModel = model();
		if( thisModel )
		{
			thisModel->addJournalCheckPoint();
			thisModel->saveJournallingState( false );
		}
	}
	else
	{
		IntModelView::mousePressEvent( event );
	}
}


void DenominatorSpinBox::mouseReleaseEvent( QMouseEvent* )
{
	if( m_mouseMoving )
	{
		model()->restoreJournallingState();

		QCursor::setPos( m_origMousePos );
		QApplication::restoreOverrideCursor();

		m_mouseMoving = false;
	}
}


void DenominatorSpinBox::enterDenominatorValue()
{
	bool ok;
	int new_val;

	new_val = QInputDialog::getInt(
			this, windowTitle(),
			tr( "Please enter a new value between %1 and %2:" ).
			arg( 1 ).
			arg( 32 ),
			model()->value(),
			1,
			32, 4, &ok );

	if( ok )
	{
		model()->setValue( DenominatorModel::checkValidValue( new_val ) );
	}	
}


#include "moc_DenominatorSpinBox.cxx"
