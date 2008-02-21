/***********************************************************************
**
**   tasklist.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2002 by Heiner Lamprecht, 2008 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef TASKLIST_H
#define TASKLIST_H

#include <QList>
#include <Q3ListView>
#include <QWidget>
#include <QStringList>
#include <QSpinBox>
#include <QCheckBox>

#include "flighttask.h"
#include "tasklistview.h"

class TaskList : public QWidget
{
    Q_OBJECT

public:
    /** */
    TaskList( QWidget* parent, const char* name );

    /** */
    ~TaskList();

    /** Takes out the selected task from the task list. */
    FlightTask* takeSelectedTask();

private:
    /** */
    bool saveTaskList();

private slots:
    /** */
    void slotTaskDetails();

    /**
     * load tasks from file
     */
    bool slotLoadTask();

    /** */
    void slotNewTask();

    /** */
    void slotEditTask();

    /** */
    void slotDeleteTask();

    /**
     * overtake a new task item from the editor
     */
    void slotUpdateTaskList( FlightTask* );

    /**
     * overtake a edited task item from the editor
     */
    void slotEditTaskList( FlightTask* );

    /** new value in cruising spin box set */
    void slotCruisingSpeedChanged( int value );

private:

    /** spin box for crusing speed entry */
    QSpinBox* cruisingSpeed;
    /** */
    Q3ListView* taskListView;
    /** */
    TaskListView* taskContent;
    /** */
    QList<FlightTask*> taskList;
    /** */
    static uint lastSelection;
    /** flight task being edited */
    FlightTask* editTask;
    /** names of flight tasks */
    QStringList taskNames;
};

#endif // TASKLIST_H
