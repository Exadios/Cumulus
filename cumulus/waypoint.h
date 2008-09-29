/***********************************************************************
 **
 **   waypoint.h
 **
 **   This file is part of Cumulus.
 **
 ************************************************************************
 **
 **   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
 **                         2002 adjusted by Andr� Somers for Cumulus
 **                         2007 ported to X11 by Axel Pauli
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 ***********************************************************************/

#ifndef __wayPoint__
#define __wayPoint__

#include <QPoint>
#include <QString>

#include "mapmatrix.h"
#include "wgspoint.h"

/**
 * This struct contains one flightpoint.
 */
struct flightPoint
{
  /**
   * The original position of the point. Given in
   * the internal format.
   */
  WGSPoint origP;

  /**
   * The projected position of the point.
   */
  QPoint projP;

  /**
   * The barometrical height, registered by the logger.
   */
  int height;

  /**
   * The gps-height, registered by the logger.
   */
  int gpsHeight;

  /**
   * The time, the point was registered by the logger.
   */
  unsigned int time;

  /**
   * The elevation difference to the previous Point
   */
  int dH;

  /**
   * The time difference to the previous Point
   */
  int dT;

  /**
   * The distance between the Points
   */
  int dS;

  /**
   * The Bearing to the previous Point
   */
  double bearing;

  /**
   * Kreisflug 0 oder Streckenflug 1
   */
  unsigned short f_state;
};

/**
 * This class contains the data of one waypoint.
 */

class wayPoint
{

 public:
  /**
   * The task point types.
   */
  enum TaskPointType {NotSet = 0, TakeOff = 1, Begin = 2, RouteP = 4,
                      End = 8, FreeP = 16, Landing = 32};

  /** contains an importance indidation for the waypoint */

  enum Importance { Low=0, Normal=1, High=2 };

  wayPoint();
  wayPoint(const wayPoint& inst);
  ~wayPoint();
  /** */
  QString getTaskPointTypeString() const;
  /** Compare current instance with another */
  bool equals( const wayPoint *second ) const;
  bool operator==( const wayPoint& second ) const;

  /** The name of the waypoint. */
  QString name;
  /** The original lat/lon-position of the waypoint. */
  WGSPoint origP;
  /** The projected position of the waypoint. */
  QPoint projP;
  /** The time, sector 1 has been reached. */
  unsigned int sector1;
  /** The time, sector 2 has been reached. */
  unsigned int sector2;
  /** The time, the fai-sector has been reached. */
  unsigned int sectorFAI;
  /** The angle of the sector in radian */
  double angle;
  /** The minimum angle of the sector in radian */
  double minAngle;
  /** The maximum angle of the sector in radian */
  double maxAngle;
  /** The type of the waypoint */
  short type;
  /** The task point type of the waypoint */
  enum  TaskPointType taskPointType;
  /** The waypoint index in the task list */
  short taskPointIndex;
  /** The bearing from the previous waypoint in radian */
  double bearing;
  /** The distance to the previous waypoint in km */
  double distance;
  /** The time distance to the previous waypoint in seconds */
  int distTime;

  /** Improvements for planning */
  /** long name or description (internal only) */
  QString description;
  /** ICAO name */
  QString icao;
  /** */
  QString comment;
  /** internal surface id */
  short surface;
  /** direction of runway. Range 0-36 inclusive */
  short runway;
  /** length of runway, in meters */
  short length;
  /** elevation of runway, in meters */
  int elevation;
  /** frequency of contact for waypoint, in MHz */
  double frequency;
  /** flag for landable*/
  bool isLandable;
  /** contains an importance indidation for the waypoint
   * 0=low
   * 1=normal
   * 2=high  */
  enum Importance importance;
};

#endif
