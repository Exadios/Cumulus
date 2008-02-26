/***********************************************************************
**
**   waypointlistview.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2004 by Andr� Somers, Eckhard V�llm, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef REACHPOINTLISTVIEW_H
#define REACHPOINTLISTVIEW_H

#include <QWidget>
#include <Q3ListView>
#include <QPixmap>
#include <QBoxLayout>
#include <QPushButton>

#include "waypoint.h"

class CumulusApp;

/**
 * This widget provides a list of reachable waypoints and a means to
 * select one.  @author Eckhard V�llm
 */

class ReachpointListView : public QWidget
{
    Q_OBJECT
public:
    ReachpointListView(CumulusApp *parent=0);
    ~ReachpointListView();

    /** Returns a pointer to the currently highlighted waypoint. */
    wayPoint * getSelectedWaypoint();

    
public slots: // Public slots
    /**
     * This slot is called to indicate that a selection has been made.
     */
    void slot_Select();

    /**
     * This slot is called if the info button has been clicked,
     * or the user pressed 'i'
     */
    void slot_Info();

    /**
     * Called when the listview should be closed without selection
     */
    void slot_Close ();

    /**
     * This slot is called if the Show Outland button has been clicked
     */
    void slot_ShowOl ();

    /**
     * This slot is called if the Hide Outland button has been clicked
     */
    void slot_HideOl ();

    /**
     * Signalled if the list of reachable points has changed
     */
    void slot_newList ();

    /**
     * Retreives the waypoints from the mapcontents, and fills the list.
     */
    void fillRpList();


signals: // Signals
    /**
     * This signal is emitted if a new waypoint is selected.
     */
    void newWaypoint(wayPoint*, bool);

    /**
     * This signal is send if the selection is done, and the
     * screen can be closed.
     */
    void done();

    /**
     * Emitted if the user clicks the Info button.
     */
    void info(wayPoint *);


private:
    Q3ListView* list;
    CumulusApp * par;
    QBoxLayout * buttonrow;
    wayPoint    selectedWp;
    bool       _outlandShow;
    QPushButton *cmdShowOl;
    QPushButton *cmdHideOl;
    QPixmap     _arrows;
    bool        _newList;
    QPushButton *cmdSelect;

private slots:
    void slot_Selected();

protected:
    void showEvent(QShowEvent *);

private: // Private methods

};

#endif
