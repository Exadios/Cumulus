/***********************************************************************
**
**   flarmdislay.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \author Axel Pauli
 *
 * \brief Flarm display view.
 *
 * This widget shows the Flarm display view.
 *
 */

#ifndef FLARM_DISPLAY_H
#define FLARM_DISPLAY_H

#include <QWidget>
#include <QPixmap>
#include <QResizeEvent>
#include <QPaintEvent>
#include <QShowEvent>

class FlarmDisplay : public QWidget
{
  Q_OBJECT

private:

  Q_DISABLE_COPY ( FlarmDisplay )

public:

  /** The three supported Zoom levels (5km, 1km, 0,5km). */
  enum Zoom { Low=0, Middle=1, High=2 };

  /**
   * Constructor
   */
  FlarmDisplay( QWidget *parent=0 );

  /**
   * Destructor
   */
  virtual ~FlarmDisplay();

  /**
   * Returns the current active zoom level.
   */
  enum Zoom getZoomLevel() const
  {
    return zoomLevel;
  };

protected:

  void resizeEvent( QResizeEvent *event );

  void paintEvent( QPaintEvent *event );

  void showEvent( QShowEvent *event );

public slots:

  /** Called to destroy the background pixmap. */
  void slotResetBackground()
  {
    background = QPixmap();
  };

  /** Switch to a new zoom level. */
  void slotSwitchZoom( enum Zoom value );

private:

  /** Creates the background picture with the radar screen. */
  void createBackground();

private:

  /** Background picture according to zoom level as radar screen */
  QPixmap background;

  /** current zoom level */
  enum Zoom zoomLevel;

  /** Current used center point */
  int centerX;
  int centerY;

  /** Current used width */
  int width;

  /** Current used height */
  int height;

  /** Current used scale distance to pixels */
  double scale;

  /** Current outer circle radius. */
  int radius;

};
#endif /* FLARM_DISPLAY_H */
