/***********************************************************************
 **
 **   isohypse.cpp
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2008 by Axel Pauli, Josua Dietze
 **                   2009 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include <QRegion>
#include <QString>
#include <QSize>

#include "isohypse.h"
#include "generalconfig.h"
#include "mapmatrix.h"

extern MapMatrix* _globalMapMatrix;
extern MapConfig* _globalMapConfig;

Isohypse::Isohypse( QPolygon elevationCoordinates,
                    const short elevation,
                    const uchar  elevationIndex,
                    const ushort secID,
                    const char typeID ) :
    LineElement( "Isoline", BaseMapElement::Isohypse, elevationCoordinates, false, secID ),
    _elevation(elevation),
    _elevationIndex(elevationIndex),
    _typeID(typeID)
{}


Isohypse::~Isohypse()
{}

QRegion* Isohypse::drawRegion( QPainter* targetP, const QRect &viewRect,
                               bool really_draw, bool isolines )
{
  if( !glMapMatrix->isVisible(bBox, getTypeID()) )
    {
     return static_cast<QRegion *> (0);
    }

  QPolygon tP = glMapMatrix->map(projPolygon);

  if (really_draw)
    {
      if (tP.boundingRect().isNull())
        {
          // ignore null values and return also no region
          return static_cast<QRegion *> (0);
        }

      targetP->setClipRegion(viewRect);

      targetP->drawPolygon(tP);

      if (isolines)
        {
          targetP->drawPolyline(tP);
        }
    }

  // The region is returned for the elevation finding in every
  // case, also when drawing was skipped.
  if( glMapMatrix->isInProjCenterArea(bBox) )
    {
      return new QRegion( tP );
    }

  return static_cast<QRegion *> (0);
}
