/***********************************************************************
**
**   settingspageunits.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by André Somers
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef SETTINGS_PAGEUNITS_H
#define SETTINGS_PAGEUNITS_H

#include <QWidget>
#include <QComboBox>

/**
 * This class represents the Units settings page
 * @author André Somers
 */
class SettingsPageUnits : public QWidget
{
    Q_OBJECT

public:

    SettingsPageUnits(QWidget *parent=0);
    ~SettingsPageUnits();

public slots: // Public slots
    /**
     * Called to initiate loading of the configurationfile
     */
    void slot_load();

    /**
     * called to initiate saving to the configurationfile.
     */
    void slot_save();

private:
    /**
     * This function returns the location of the value in the array.
     */
    int searchItem(int * p, int value, int max);

    QComboBox *UnitAlt;
    QComboBox *UnitSpeed;
    QComboBox *UnitDistance;
    QComboBox *UnitVario;
    QComboBox *UnitWind;
    QComboBox *UnitPosition;
    QComboBox *UnitTime;

    // we use seven arrays to store mappings of item locations in the combo boxes
    // to the enumeration values of the units they do represent
    int altitudes[2];
    int speeds[4];
    int distances[3];
    int varios[3];
    int winds[5];
    int positions[2];
    int times[2];

private slots:

    void  slotUnitChanged();
};

#endif
