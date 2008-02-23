/***********************************************************************
**
**   degreespinbox.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2002 by Andr� Somers, 2008 Axel pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "degreespinbox.h"

DegreeSpinBox::DegreeSpinBox(QWidget *parent) : QSpinBox(parent)
{
  this->setMinValue(-1);
  this->setMaxValue(36);
  this->setWrapping(true);
  this->setLineStep(1);
  this->setValue(-1); //default=Unknown
}


DegreeSpinBox::~DegreeSpinBox()
{}


QString DegreeSpinBox::textFromValue(int value) const
{
  if( value == -1 )
    {
      return QString(tr("Unknown"));
    }

  return QString("%1").arg(value*10);
}


int DegreeSpinBox::valueFromText( const QString &text ) const
{
  if( text == QString(tr("Unknown")) )
    {
      return -1;
    }

  return int(text.toInt()/10);
}

