/************************************************************************
 **
 **   Copyright (c): 2007 by Axel Pauli, axel@kflog.org
 **
 **   This file is distributed under the terms of the General Public
 **   Licence. See the file COPYING for more information.
 **
 **   $Id$
 **
 *************************************************************************
 **
 **   sonne.h
 **
 **   This class is part of Cumulus. It provides calculation of sun
 **   rise and sun set times. Algorithmus was taken from a Swiss
 **   german webpage. Thanks to the author Roland Brodbeck for his
 **   publication. Explanations are all in german.
 **
 ***********************************************************************/
//
// Sonnenaufgangs- und untergangsberechnung nach
// http://lexikon.astronomie.info/zeitgleichung/neu.html
// Version Januar 2005
// von Dr. sc. nat. Roland Brodbeck, Diplom Physiker ETH Z�rich
//
// Ergebnisse wurden hier gepr�ft: http://www.generalaviation.de/sunrise/index.shtml
//
// Achtung! Der Algorithmus weisst nur eine bedingte Genauigkeit im
// Minutenbereich auf. Besonders am Polarkreis kann es zu gr��eren
// Differenzen gegen�ber einer besseren Berechnungsmethode
// kommen. Diese Zeiten sind nur Anhaltswerte und keine amtlich
// anerkannten! F�r die Richtigkeit wird keinerlei Haftung �bernommen.
//
//***********************************************************************

#ifndef _sonne_h
#define _sonne_h

#include <QString>
#include <QPoint>
#include <QDateTime>

class Sonne
{
 public:

  // Gregorianischer Kalender
  static double JulianischesDatum ( int Jahr, int Monat, int Tag, 
                                    int Stunde=12, int Minuten=0, double Sekunden=0.0 );

  static double InPi( double x );

  // Neigung der Erdachse
  static double eps( const double T );

  static double BerechneZeitgleichung( double &DK, double T );

  // Auf:	Aufgangszeit in hh:mm
  // Unter:	Untergangszeit in hh:mm
  // Datum:     Tagesdatum
  // Position:  x=Breite, y=L�nge WGS84 Koordinaten im KFlog Format
  // Zeitzone:	0=Weltzeit (UTC)
  //            1=Winterzeit (MEZ)
  //            2=Sommerzeit (MESZ)
  //
  //  R�ckgabe true=OK, false=Fehler, im Fehlerfall sind Auf und Unter
  //  Variablen nicht gesetzt
  static bool sonneAufUnter( QString& Auf, QString& Unter,
                             QDate& Datum,
                             QPoint& Position,
                             double Zeitzone=0 );

};

#endif
