/***********************************************************************
**
**   wpeditdialog.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Andr� Somers, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef WPEDIT_DIALOG_H
#define WPEDIT_DIALOG_H

#include <QWidget>
#include <QDialog>
#include <QTextEdit>
#include "waypoint.h"

/**
 * The WpEditDialog allows the creation of a new waypoint or the modification
 * of an existing waypoint.
 * @author Andr� Somers
 */

class WpEditDialog : public QDialog
{
    Q_OBJECT
public:
    /**
     * Constructor
     * @param parent The parent widget
     * @param name A name for this QObject
     * @param wp A waypoint to show the dialog for
     */
    WpEditDialog(QWidget *parent=0, wayPoint * wp=0);

    virtual ~WpEditDialog();

    /**
     * Called if OK button is pressed
     */
    void accept();

private slots: // Private slots
    /**
     * This slot is called just before showing the dialog,
     * and loads the current settings.
     */
    void slot_LoadCurrent();

signals: // Signals
    /**
     * Signal emitted to indicate the settings should be saved
     * to the configuration file
     */
    void save(wayPoint *);

    /**
     * Emitted to indicate that the settings should be
     * re-) loaded from the configuration file.
     */
    void load(wayPoint *);

    /**
     * this signal is emitted after a save procedure has
     * occurred. It gives connected objects the change to
     * adjust to new settings.
     */
    void settingsChanged();

    /**
     * This signal is emitted after a waypoint has been saved.
     * It is used to re-fill the waypoint list.
     */
    void wpListChanged(wayPoint *);

private:

    wayPoint * _wp;
    QTextEdit * comment;
};

#endif
