/***********************************************************************
**
**   gliderflightdialog.h
**
**   This file is part of QPushButton *cancelCumulus.
**
************************************************************************
**
**   Copyright (c):  2002      by Eggert Ehmke
**                   2008-2009 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   Licence. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#ifndef GLIDER_FLIGHT_DIALOG_H
#define GLIDER_FLIGHT_DIALOG_H

#include <QDialog>
#include <QPushButton>
#include <QTimer>
#include <QSpinBox>
#include <QDoubleSpinBox>

#include "speed.h"

/**
 * @author Eggert Ehmke
 */

class GliderFlightDialog : public QDialog
{
    Q_OBJECT

public:

    GliderFlightDialog(QWidget *parent);
    ~GliderFlightDialog();
    void load ();

protected:

    void accept ();
    void showEvent(QShowEvent *);

private:

    void save ();

    QDoubleSpinBox* spinMcCready;
    QSpinBox* spinWater;
    QSpinBox* spinBugs;
    QPushButton* buttonDump;
    QTimer* timeout;
    int _time;

    QPushButton* mcPlus;
    QPushButton* mcMinus;

    QPushButton* waterPlus;
    QPushButton* waterMinus;

    QPushButton* bugsPlus;
    QPushButton* bugsMinus;

    QPushButton *ok;
    QPushButton *cancel;

private slots:

    void slotDump();
    void setTimer();

    void slotMcPlus();
    void slotMcMinus();

    void slotWaterPlus();
    void slotWaterMinus();

    void slotBugsPlus();
    void slotBugsMinus();

signals:
    void settingsChanged();
};

#endif
