/***********************************************************************
 **
 **   airspace.cpp
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2000 by Heiner Lamprecht, Florian Ehinger
 **   Modified:       2008 by Axel Pauli, Josua Dietze
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#include "airspace.h"
#include "airregion.h"
#include "generalconfig.h"
#include "cucalc.h"

Airspace::Airspace(QString n, BaseMapElement::objectType t, QPolygon pG,
                   int u, BaseMapElement::elevationType uType,
                   int l, BaseMapElement::elevationType lType)
  : LineElement(n, t, pG), lLimitType(lType), uLimitType(uType),
    m_airRegion(0)
{
  type = t;

  // All Airspaces are closed regions ...
  closed = true;

  // Normalize values
  double lLim=0.0;
  switch( lLimitType ) {
  case GND:
  case MSL:
  case STD:
    lLim = Distance::mFromFeet*l;
    break;
  case FL:
    lLim = Distance::mFromFeet*100.0*l;
    break;
  case UNLTD:
    lLim=99999.0;
    break;
  case NotSet:
    lLim=0.0;
    break;
  default:
    lLim=0.0;
  };

  lLimit.setMeters( lLim );
  double uLim=0.0;
  switch( uLimitType ) {
  case GND:
  case MSL:
  case STD:
    uLim = Distance::mFromFeet*u;
    break;
  case FL:
    uLim = Distance::mFromFeet*100.0*u;
    break;
  case UNLTD:
    lLim=99999.0;
    break;
  case NotSet:
    lLim=0.0;
    break;
  default:
    uLim=0.0;
  };

  uLimit.setMeters( uLim );
  _lastVConflict=none;
}


Airspace::~Airspace()
{
  // @AP: Remove the class pointer in related AirRegion object. We
  // have a cross reference here. First deleted object will reset its
  // pointer in the other object. Check is necessary to avoid usage of
  // null pointer.

  if( m_airRegion ) {
    m_airRegion->airspace = 0;
  }
}

void Airspace::drawRegion( QPainter* targetP, const QRect &viewRect,
                           qreal opacity )
{
  // qDebug("Airspace::drawRegion(): TypeId=%d, opacity=%f, Name=%s",
  //        typeID, opacity, getInfoString().latin1() );


  if(!glConfig->isBorder(typeID) || !__isVisible())
    {
      return;
    }

  //@JD: replaced clipping and filling with polygon drawing, regions not needed anymore

  QPolygon tP = glMapMatrix->map(projPolygon);

  QBrush drawB = glConfig->getDrawBrush(typeID);

  if( opacity < 100.0 )
    {
      // use transparent filled air regions
      drawB.setStyle( Qt::SolidPattern );
    }

  QPen drawP = glConfig->getDrawPen(typeID);
  drawP.setJoinStyle(Qt::RoundJoin);
  // increase drawPen, it is to small under X11
  //@JD: polygon drawing blew it up further for some reason, thus reduced increasing
  drawP.setWidth(drawP.width() + 1);

  targetP->setPen(drawP);
  targetP->setBrush(drawB);

  targetP->setClipRegion( viewRect );

  if( opacity < 100.0 && opacity > 0.0 )
    {
      // Draw airspace filled with opacity factor
      targetP->setOpacity( opacity/100.0 );
      targetP->drawPolygon(tP);
      targetP->setBrush(Qt::NoBrush);
      targetP->setOpacity( 1.0 );
    }
  else if( opacity == 0.0 )
    {
      // draw only airspace borders without any filling inside
      targetP->setBrush(Qt::NoBrush);
      targetP->setOpacity( 1.0 );
    }

  // Draw the outline of the airspace with the selected brush
  targetP->drawPolygon(tP);

  /*#warning: FIXME: we should change to use the plain pointarray
    (projPointArray), so our precision isn't depending on the current
    scale */

  return;
}

/**
 * Return a pointer to the mapped airspace region data. The caller takes
 * the ownership about the returned object.
 */
QRegion* Airspace::createRegion()
{
  QPolygon tP = glMapMatrix->map(projPolygon);

  return (new QRegion(tP));
}

/**
 * Returns a text representing the type of the airspace
 */
QString Airspace::getTypeName (objectType type)
{
  switch(type) {
  case BaseMapElement::AirA:
    return QObject::tr("AS-A");
  case BaseMapElement::AirB:
    return QObject::tr("AS-B");
  case BaseMapElement::AirC:
    return QObject::tr("AS-C");
  case BaseMapElement::AirD:
    return QObject::tr("AS-D");
  case BaseMapElement::AirElow:
    return QObject::tr("AS-E low");
  case BaseMapElement::AirEhigh:
    return QObject::tr("AS-E high");
  case BaseMapElement::AirF:
    return QObject::tr("AS-F");
  case BaseMapElement::Restricted:
    return QObject::tr("Restricted");
  case BaseMapElement::Danger:
    return QObject::tr("Danger");
  case BaseMapElement::Prohibited:
    return QObject::tr("Prohibited");
  case BaseMapElement::ControlC:
    return QObject::tr("CTR-C");
  case BaseMapElement::ControlD:
    return QObject::tr("CTR-D");
  case BaseMapElement::LowFlight:
    return QObject::tr("Low Flight");
  case BaseMapElement::Tmz:
    return QObject::tr("TMZ");
  case BaseMapElement::SuSector:
    return QObject::tr("Sector");
  default:
    return "<B><EM>" + QObject::tr("unknown") + "</EM></B>";
  }
}


QString Airspace::getInfoString() const
{
  QString text, tempL, tempU;

  QString type;

  switch(lLimitType) {
  case MSL:
    tempL.sprintf("%s MSL", lLimit.getText(true,0).toLatin1().data());
    break;
  case GND:
    if(lLimit.getMeters())
      tempL.sprintf("%s GND", lLimit.getText(true,0).toLatin1().data());
    else
      tempL = "GND";
    break;
  case FL:
    tempL.sprintf("FL %d (%s)", (int) rint(lLimit.getFeet()/100.), lLimit.getText(true,0).toLatin1().data());
    break;  
  case STD:
    tempL.sprintf("%s STD", lLimit.getText(true,0).toLatin1().data());
    break;
  case UNLTD:
    tempL = QObject::tr("Unlimited");
  default:
    ;
  }

  switch(uLimitType) {
  case MSL:
    if(uLimit.getMeters() >= 99999)
      tempU = QObject::tr("Unlimited");
    else
      tempU.sprintf("%s MSL", uLimit.getText(true,0).toLatin1().data());
    break;
  case GND:
    tempU.sprintf("%s GND", uLimit.getText(true,0).toLatin1().data());
    break;
  case FL:
    tempU.sprintf("FL %d (%s)", (int) rint(uLimit.getFeet()/100.), uLimit.getText(true,0).toLatin1().data());
    break;
  case STD:
    tempU.sprintf("%s STD", uLimit.getText(true,0).toLatin1().data());
    break;
  case UNLTD:
    tempU = QObject::tr("Unlimited");
  default:
    ;
  }

  text = getTypeName(typeID);

  text += " " + name + "<BR>" +
    "<FONT SIZE=-1>" + tempL + " / " + tempU + "</FONT>";

  return text;
}


/**
 * Returns true if the given altitude conflicts with the airspace
 * properties. Only the altitude is considered not the current
 * position.
 */
//bool Airspace::conflicts (const Altitude& alt) const
Airspace::ConflictType Airspace::conflicts (const AltitudeCollection& alt,
                                            const AirspaceWarningDistance& dist) const
{
  Altitude lowerAlt(0);
  Altitude upperAlt(0);

  //set which altitude to use from our range of available altitudes,
  //and apply uncertainty margins
    
  //#warning FIXME: we should take our GPS error into account
  switch (lLimitType) {
  case NotSet:
    break;
  case MSL:
    lowerAlt=alt.gpsAltitude;
    break;
  case GND:
    lowerAlt=alt.gndAltitude + alt.gndAltitudeError;  // we need to use a conservative estimate
    if (lLimit==0)
      lowerAlt.setMeters(1); // we're always above ground
    break;
  case FL:
  case STD:
    lowerAlt=alt.stdAltitude; // flight levels are always at pressure altitude!
    break;
  case UNLTD:
    _lastVConflict=none;
    return none;
  }

  switch (uLimitType) {
  case NotSet:
    upperAlt.setMeters(100000);
    break;
  case MSL:
    upperAlt=alt.gpsAltitude;
    break;
  case GND:
    upperAlt=alt.gndAltitude - alt.gndAltitudeError;  //we need to use a conservative estimate
    break;
  case FL:
  case STD:
    upperAlt=alt.stdAltitude;
    break;
  case UNLTD:
    upperAlt=uLimit-1; //we are always below the upper border of an Unlimited airspace
  }

  //check to see if we're inside the airspace
  if ((lowerAlt.getMeters() >= lLimit.getMeters()) &&
      (upperAlt.getMeters() <= uLimit.getMeters())) {
    _lastVConflict=inside;
    // qDebug("vertical conflict: %d, airspace: %s", _lastVConflict, getName().latin1());
    return inside;
  }

  // @AP: very near and near will not work, if you use the defined
  // operators. Changed it to the getMeters method, that will work
  // fine.

  //not inside. Check to see if we're very near to the airspace
  if ((lowerAlt.getMeters() >= (lLimit.getMeters() - dist.verBelowVeryClose.getMeters())) &&
      (upperAlt.getMeters() <= (uLimit.getMeters() + dist.verAboveVeryClose.getMeters()))) {
    _lastVConflict=veryNear;
    return veryNear;
  }

  //not very near. Just near then?
  if ((lowerAlt.getMeters() >= (lLimit.getMeters() - dist.verBelowClose.getMeters())) &&
      (upperAlt.getMeters() <= (uLimit.getMeters() + dist.verAboveClose.getMeters()))) {
    _lastVConflict=near;
    return near;
  }

  //nope, we're not even near.
  _lastVConflict=none;
  return none;
}

bool Airspace::operator < (const Airspace& other) const
{
  int a1C = getUpperL(), a2C = other.getUpperL();

  if (a1C > a2C) {
    return false;
  } else if (a1C < a2C) {
    return true;
  } else { //equal
    int a1F = getLowerL(), a2F = other.getLowerL();
    return (a1F < a2F);
  }
}


