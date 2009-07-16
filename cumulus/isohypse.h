/***********************************************************************
 **
 **   isohypse.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **                   2007-2009 Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef ISOHYPSE_H
#define ISOHYPSE_H

#include <QRect>
#include <QRegion>

#include "lineelement.h"

/**
 * This class is used for isohypses.
 *
 * @author Heiner Lamprecht, Florian Ehinger
 */
class Isohypse : public LineElement
  {
  public:
        /**
     * Creates a new isohypse.
     *
     * @param  elevationCoordinates  The polygon containing the projected points of the isoline.
     * @param  elevation  The elevation in meters
     * @param  elevationIndex The elevation as index
     * @param  secID The tile section identifier
     * @param  typeID The type of isohypse, ground or terrain
     */
    Isohypse( QPolygon elevationCoordinates,
              const short elevation,
              const uchar  elevationIndex,
              const ushort secID,
              const char typeID );
    /**
     * Destructor
     */
    virtual ~Isohypse();

    /**
     * Draws the iso region into the given painter.
     *
     * @param  targetP  The painter to draw the element into.
     */
    QRegion* drawRegion( QPainter* targetP, const QRect &viewRect,
                         bool really_draw, bool isolines=false );

    /**
     * @return the elevation of the line
     */
    short getElevation() const
      {
        return _elevation;
      };

    /**
     * @return the elevation index of the line
     */
    ushort getElevationIndex() const
      {
        return _elevationIndex;
      };

    /**
     * @return the type of isohypse, ground 'G' or terrain 'T'
     */
    ushort getTypeId() const
      {
        return _typeID;
      };

  private:

    /**
     * The elevation in meters
     */
    short _elevation;

    /**
     * The elevation index
     */
    uchar _elevationIndex;

    /**
     * The type of isohypse, ground or terrain.
     */
     char _typeID;
  };

#endif
