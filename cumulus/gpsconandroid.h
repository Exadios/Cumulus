/***********************************************************************
 **
 **   gpsconandroid.h
 **
 **   This file is part of Cumulus
 **
 ************************************************************************
 **
 **   Copyright (c):  2012 by Axel Pauli (axel@kflog.org)
 **
 **   This program is free software; you can redistribute it and/or modify
 **   it under the terms of the GNU General Public License as published by
 **   the Free Software Foundation; either version 2 of the License, or
 **   (at your option) any later version.
 **
 **   $Id$
 **
 ***********************************************************************/

/**
 * \class GpsConAndroid
 *
 * \author Axel Pauli
 *
 * \brief GPS connection interface from and to Android's Java part.
 *
 * This module manages the GPS data transfer to and from the Android Java
 * part.
 *
 * \date 2012
 */

#ifndef GPS_CON_ANDROID_H
#define GPS_CON_ANDROID_H

#include <QObject>

class QByteArray;
class QMutex;
class QString;

class GpsConAndroid : public QObject
{
  Q_OBJECT

 private:

  Q_DISABLE_COPY( GpsConAndroid )

 public:

  GpsConAndroid( QObject* parent=0 );

  virtual ~GpsConAndroid();

  static void rcvByte( const char byte );

  static bool sndBytes( QByteArray& bytes );

  static bool getByte( unsigned char* b );

  static bool verifyCheckSum( const char *sentence );

  static void forwardNmea( QString& qnmea );

#ifdef FLARM

 private:

  /** Gets the flight list from the Flarm device. */
  void getFlarmFlightList();

  /** Reports an error to the calling application. */
  void flarmFlightListError();

  /**
   * Downloads the requested IGC flights. The args string contains the destination
   * directory and one or more flight numbers. The single elements are separated
   * by vertical tabs.
   */
  void getFlarmIgcFiles( QString& args );

  /** Reports an info to the calling application. */
  void flarmFlightDowloadInfo( QString info );

  /** Reports the download progress to the calling application. */
  void flarmFlightDowloadProgress( const int idx, const int progress );

  /**
   * Switches the Flarm device into the binary mode.
   *
   * \return True on success otherwise false.
   */
  bool flarmBinMode();

  /**
   * Resets the Farm device. Should be called only if Flarm is in binary mode.
   */
  bool flarmReset();

#endif

  signals:

  /**
   * This signal is emitted, when a new byte was received from the Java part.
   */
  void newByte();

 private:

  /** Receive buffer for GPS data from java part. */
  static QByteArray rcvBuffer;

  /** Thread synchronizer. */
  static QMutex mutex;
};

#endif
