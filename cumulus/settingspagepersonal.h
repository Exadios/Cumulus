/***********************************************************************
**
**   settingspagepersonal.h
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

#ifndef SETTINGS_PAGE_PERSONAL_H
#define SETTINGS_PAGE_PERSONAL_H

#include <QWidget>
#include <QLineEdit>
#include <QComboBox>
#include <QStringList>

#include "coordedit.h"

/**
 * This class represents the personal settings page
 */

class SettingsPagePersonal : public QWidget
{
  Q_OBJECT

  public:

    SettingsPagePersonal(QWidget *parent=0);
    ~SettingsPagePersonal();

    /** Checks if the home position has been changed */
    bool checkIsHomePositionChanged();

    /** Checks if the home latitude has been changed */
    bool checkIsHomeLatitudeChanged();

    /** Checks if the home longitude has been changed */
    bool checkIsHomeLongitudeChanged();

  public slots:

    /** called to initiate saving to the configuration file */
    void slot_save();

    /** Called to initiate loading of the configuration file. */
    void slot_load();

    /** Called to ask is confirmation on the close is needed. */
    void slot_query_close(bool& warn, QStringList& warnings);

  private slots:

    /** called to open the directory selection dialog */
    void slot_openDirectoryDialog();

  private:

    bool loadConfig; // control loading of configuration data

    QLineEdit *edtName;
    QComboBox *langBox;
    LatEdit   *edtHomeLat;
    LongEdit  *edtHomeLong;
    QLineEdit *userDataDir;
};

#endif
