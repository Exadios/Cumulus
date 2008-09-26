/***********************************************************************
**
**   mapcalc.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  1999, 2000 by Heiner Lamprecht, Florian Ehinger
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef MAPCALC_H
#define MAPCALC_H

#include <QRect>

#include "waypoint.h"

#define PI2 M_PI*2

/**
 * The earth's radius used for calculation, given in Meters
 * NOTE: We use the earth as a sphere, not as a spheroid!
 */
#define RADIUS 6371000 // FAI Radius, this was the previous radius ->6370290
#define RADIUS_kfl (RADIUS / (360.0 * 600000.0))

// Define nautical mile in meters according to earth radius of KFL
#define MILE_kfl  (PI2 * RADIUS / (360.0 * 60.0))

/**
 * Calculates the distance between two given points (in km).
 */
double dist(double lat1, double lon1, double lat2, double lon2);

double distC(double lat1, double lon1, double lat2, double lon2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(QPoint* p1, QPoint* p2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(wayPoint* wp1, wayPoint* wp2);

/**
 * Calculates the distance between two given points (in km).
 */
double dist(wayPoint* wp, flightPoint* fp);

/**
 * Calculates the distance between two given points (in km).
 */
double dist( flightPoint* fp1, flightPoint* fp2);

/**
 * Converts the given time (in sec.) into a readable string.
 * ( hh:mm:ss )
 */
QString printTime(int time, bool isZero = false, bool isSecond = true);

/**
 * Calculates the vario of a given point
 */
float getVario(flightPoint p);

/**
 * Calculates the speed of a given point
 */
float getSpeed(flightPoint p);

/**
 * Calculates the bearing to the previous point
 */
float getBearing(flightPoint p1, flightPoint p2);

/**
 * Calculates the bearing to the next point
 */
float getBearing(QPoint p1, QPoint p2);

/**
 * Calculates the bearing to the next point with coordinates mapped to
 * the current projection
 */
double getBearing2(QPoint p1, QPoint p2);

/**
 * Calculates the bearing to the next point with wgs84 coordinates
 */
double getBearingWgs(QPoint p1, QPoint p2);

/**
 * Converts a x/y position into a polar-coordinate.
 */
double polar(double y, double x);

/**
 * Calculates the direction of the vector pointing to the outside
 * of the area spanned by the two vectors.
 */
double outsideVector(QPoint center, QPoint p1, QPoint p2);

double normalize(double angle);

int normalize(int angle);

/**
  * Calculates the difference between two angles, returning the smallest
  * angle. It returns an angle between -180 and 180 in degrees. Positive
  * in clockwise direction.
  */
int angleDiff(int ang1, int ang2);

/**
  * Calculates the difference between two angles, returning the smallest
  * angle. It returns an angle between -Pi and Pi in rad.
  */
double angleDiff(double ang1, double ang2);

/**
 * Calculates a (crude) bounding box that contains the circle of radius @arg r
 * around point @arg center. @arg r is given in kilometers.
 */
QRect areaBox(QPoint center, double r);

#endif
