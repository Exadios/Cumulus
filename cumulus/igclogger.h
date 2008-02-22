/***************************************************************************
                          igclogger.h  -  creates an igc logfile
                             -------------------
    begin                : Sat Jul 20 2002
    copyright            : (C) 2002 by André Somers, 2008 Axel Pauli
    email                : andre@kflog.org, axel@kflog.org

    This file is part of Cumulus

    $Id$

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef IGCLOGGER_H
#define IGCLOGGER_H

/* info about flightlogger */
#define FL_CODE X
#define FL_LCODE XXX
#define FL_NAME Cumulus
#define FL_ID ATS
/*maybe give some info on the Zaurus in the future?*/
#define FL_HWV UNKNOWN
/*the versionnumber of Cumulus is used as firmwareversion */

/* info about GPS, may in the future be used from GPS */
#define GPS_MAN UNKNOWN
#define GPS_MODEL UNKNOWN
#define GPS_CHAN UNKNOWN
#define GPS_MAXALT UNKNOWN


#include <QObject>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QTimer>
#include <QDateTime>

#include "altitude.h"
#include "cucalc.h"
#include "limitedlist.h"

/** @short IGC logger
 * This object provides the igc logging facilities, using the
 * parsed data from the GPSNMEA object.
 * @author André Somers
 */
class IgcLogger : public QObject
{
    Q_OBJECT

public: //types
    /**
      * Used to describe the current logging mode:
      * -off: no logging going on
      * -on: logging enabled
      * -standby: logging will be turned on when the flightstatus changes
      */
    enum LogMode{off=0, standby=1, on=2};

public: //methods
    /**
     * Destructor
     */
    virtual ~IgcLogger();

    /**
     * Stop logging
     */
    void Stop();

    /**
     * Starts logging of data
     */
    void Start();

    /**
     * Switches to standby mode. If we are currently logging, the logfile will
     * be closed.
     */
    void Standby();

    /**
     * Creates a log file
     */
    void CreateLogfile();

    /**
     * Closes the logfile.
     */
    void CloseFile();

    /**
     * @returns true if we are currently logging
     */
    virtual const bool getisLogging();

    /**
     * @returns true if we are currently standby
     */
    virtual const bool getisStandby();

    /**
     * @returns the instance of the IgcLogger, and creates an instance
     * if there was none
     */
    static IgcLogger * instance();

public slots: // Public slots
    /**
     * This slot is used internaly by the timer to make a log entry on
     * an interval, but can also be used from outside the class to
     * make sure a specific point is being logged (ie., to respond to
     * a usercommand to log).
     */
    void slotMakeFixEntry();

    /** Call this slot, if a task sector has been touched to increase
     *  logger interval for a certain time.
     */
    void slotTaskSectorTouched();

    /**
     * This slot is called to indicate that a new satellite constellation is
     * now in use.
     */
    void slotConstellation();

    /**
     * This slot is called to start or end a log.
     */
    void slotToggleLogging();

    /**
     * This slot is used to signal that a new flightmode has been detected.
     * This is needed for auto-starting the logger if the logger is in
     * standby mode.
     */
    void slotFlightMode(CuCalc::flightmode);

signals: // Signals
    /**
     * This signal is send to indicate that logging has started
     * (isLogging is true) or has stopped (isLogging is false).
     */
    void logging(bool isLogging);

    /**
     * This signal is emitted for every fix entry that has been
     * entered in the log. It can be used to display a visual
     * indication that an entry has been made.
     */
    void madeEntry();


private: // Private attributes
    /** Provides a timer for the periodic recording of logentries */
    QTimer *timer;
    /** Store time interval for faster logging */
    QTime fasterLoggingTime;
    /** The textstream object to write our data to. */
    QTextStream _stream;
    /** This is our logfile. It is being used via the _stream object. */
    QFile _logfile;
    /** Indicates if logging is currently active. */
    LogMode _logMode;
    /** Timestamp of last logged fix */
    QTime lastLoggedFix;
    /** List of last would-be logentries.
      * This list is filled when in standby mode with strings that would be
      * in the log were logging enabled. When a change in flightmode is detected
      * and logging is triggered, the list is used to write out some older events
      * to the log. This way, we can be sure that the complete start sequence is
      * available in the log. */
    LimitedList<QString> _backtrack;
    /** Holds the flightnumber for this day */
    int flightnumber;
    /** A pointer to the instance */
    static IgcLogger * _theInstance;


private: // Private methods
    /**
     * Constructor
     * Private, because this is a singleton class
     */
    IgcLogger(QObject*);

    /**
     * This function formats a date in the correct igc format DDMMYY
     */
    QString formatDate(const QDate& date);

    /**
     * This function writes the headers for the logfile to the logfile.
     */
    void writeHeaders();

    /**
     * Makes a fix entry in the logfile.
     */
    void makeSatConstEntry();

    /**
     * This function formats an Altitude to the correct format
     * for IGC files (XXXXX)
     */
    QString formatAltitude(Altitude altitude);

    /**
     * This function formats a QTime to the correct format for igc
     * files (HHMMSS)
     */
    QString formatTime(const QTime& time);

    /**
     * This function formats the position to the correct format for
     * igc files. Latitude and Longitude are encoded as DDMMmmmADDDMMmmmO,
     * with A=N or S and O=E or W.
     */
    QString formatPosition(const QPoint& position);

    /**
     * Creates a new filename for the IGC file according to the IGC
     * standards (IGC GNSS FR Specification, may 2002, Section 2.5)
     */
    QString createFileName(const QString& path);
};

#endif


