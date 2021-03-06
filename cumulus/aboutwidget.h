/***********************************************************************
**
**   aboutwidget.h
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c): 2010-2011 Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * \class AboutWidget
 *
 * \author Axel Pauli
 *
 * \brief A widget to display the about application data.
 *
 * This widget displays the about application data in a tabbed window
 * decorated this a headline and an icon on top.
 *
 * \date 2010-2013
 *
 * \version $Id$
 *
*/

#ifndef ABOUT_WIDGET_H
#define ABOUT_WIDGET_H

#include <QWidget>
#include <QMetaObject>
#include <QLabel>
#include <QTextBrowser>
#include <QString>

class QPixmap;

class AboutWidget : public QWidget
{
  Q_OBJECT

  private:

  Q_DISABLE_COPY ( AboutWidget )

 public:

  AboutWidget( QWidget *parent = 0 );

  virtual ~AboutWidget() {};

  /**
  * Sets the passed pixmap at the left side of the headline.
  *
  * \param pixmap The pixmap to be set in the headline.
  */
  void setHeaderIcon( const QPixmap pixmap )
  {
    headerIcon->setPixmap( pixmap );
  };

  /**
  * Sets the passed text in the headline. The text can be HTML formatted.
  *
  * \param text The text to be set in the headline.
  */
  void setHeaderText( const QString& text )
  {
    headerText->setText( text );
  };

  /**
  * Sets the passed text on the about page. The text can be HTML formatted.
  *
  * \param text The text to be set on the about page.
  */
  void setAboutText( const QString& text )
  {
    about->setHtml( text );
  };

  /**
  * Sets the passed text on the team page. The text can be HTML formatted.
  *
  * \param text The text to be set on the team page.
  */
  void setTeamText( const QString& text )
  {
    team->setHtml( text );
  };

  /**
  * Sets the passed text on the disclaimer page. The text can be HTML formatted.
  *
  * \param text The text to be set on the disclaimer page.
  */
  void setDisclaimerText( const QString& text )
  {
    disclaimer->setHtml( text );
  };

 private slots:

  /** Called, if the cursor position is changed to clear the text selection. */
  void slotAboutCursorChanged();

  /** Called, if the cursor position is changed to clear the text selection. */
  void slotTeamCursorChanged();

  /** Called, if the cursor position is changed to clear the text selection. */
  void slotDisclaimerCursorChanged();

 private:

  /** The header icon widget. */
  QLabel       *headerIcon;

  /** The header text widget. */
  QLabel       *headerText;

  /** The about text browser page widget. */
  QTextBrowser *about;

  /** The team text browser page widget. */
  QTextBrowser *team;

  /** The disclaimer text browser page widget. */
  QTextBrowser *disclaimer;

};

#endif /* ABOUT_WIDGET_H */
