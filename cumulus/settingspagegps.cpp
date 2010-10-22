/***********************************************************************
**
**   settingspagegps.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright(c): 2002      by Andrè Somers,
**                 2007-2010 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

/**
 * This widget is used to define the GPS interface parameters.
 * The user can select different source devices and some special
 * GPS parameters.
 *
 * There is a difference in the provided options between normal Desktop
 * and Maemo. Under Maemo RS232 devices are supported only via USB.
 */

#include <QtGui>

#include "generalconfig.h"
#include "settingspagegps.h"
#include "gpsnmea.h"
#include "gpscon.h"
#include "hwinfo.h"

SettingsPageGPS::SettingsPageGPS(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGPS");
  QGridLayout* topLayout = new QGridLayout(this);

  int row=0;

  topLayout->addWidget(new QLabel(tr("GPS Device:"), this), row, 0);
  GpsDev = new QComboBox(this);
  topLayout->addWidget(GpsDev, row++, 1);
  GpsDev->setEditable(true);
  topLayout->setColumnStretch(2, 10);

#ifdef TOMTOM
  GpsDev->addItem(TOMTOM_DEVICE);
#endif

#ifndef MAEMO
  GpsDev->addItem("/dev/ttyS0");
  GpsDev->addItem("/dev/ttyS1");
  GpsDev->addItem("/dev/ttyS2");
  GpsDev->addItem("/dev/ttyS3");
  GpsDev->addItem("/dev/ttyUSB0"); // external USB device
  // Bluetooth default devices
  GpsDev->addItem("/dev/rfcomm0");
  GpsDev->addItem("/dev/rfcomm1");
#ifdef BLUEZ
  // Bluetooth default adapter
  GpsDev->addItem(BT_ADAPTER);
#endif
  // add entry for NMEA simulator choice
  GpsDev->addItem(NMEASIM_DEVICE);
#else
  // Under Maemo there are only three predefined sources.
  GpsDev->addItem(MAEMO_LOCATION_SERVICE); // Maemo GPS Location Service
  // Bluetooth default adapter
  GpsDev->addItem(BT_ADAPTER);
  GpsDev->addItem("/dev/ttyUSB0"); // external USB device
  GpsDev->addItem(NMEASIM_DEVICE); // Cumulus NMEA simulator
#endif

  // catch selection changes of the GPS device combo box
  connect( GpsDev, SIGNAL(activated(const QString &)),
           this, SLOT(slot_gpsDeviceChanged(const QString&)) );

  topLayout->addWidget(new QLabel(tr("Speed (bps):"), this), row, 0);
  GpsSpeed = new QComboBox(this);
  GpsSpeed->setEditable(false);
  GpsSpeed->addItem("230400");
  GpsSpeed->addItem("115200");
  GpsSpeed->addItem("57600");
  GpsSpeed->addItem("38400");
  GpsSpeed->addItem("19200");
  GpsSpeed->addItem("9600");
  GpsSpeed->addItem("4800");
  GpsSpeed->addItem("2400");
  GpsSpeed->addItem("1200");
  GpsSpeed->addItem("600");
  topLayout->addWidget(GpsSpeed, row++, 1);

  // Defines from which device the altitude data shall be taken. Possible
  // devices are the GPS or a pressure sonde.
  topLayout->addWidget(new QLabel(tr("Altitude Reference:"), this),row,0);
  GpsAltitude = new QComboBox(this);
  GpsAltitude->setEditable(false);
  topLayout->addWidget(GpsAltitude,row++,1);
  GpsAltitude->addItem(tr("GPS"));
  GpsAltitude->addItem(tr("Pressure"));

  topLayout->setColumnStretch(2,10);

#ifndef MAEMO
  topLayout->setRowMinimumHeight( row++, 10);

  checkSyncSystemClock = new QCheckBox (tr("Sync Clock"), this);
  topLayout->addWidget(checkSyncSystemClock, row, 0 );
  row++;
#endif

  topLayout->setRowStretch(row++, 10);
  topLayout->setColumnStretch(2,10);

  // search for GPS device to be selected
  bool found = false;

  QString devText = GeneralConfig::instance()->getGpsDevice();

  // select last saved device, if possible
  for (int i=0; i < GpsDev->count(); i++)
    {
      if (GpsDev->itemText(i) == devText)
        {
          GpsDev->setCurrentIndex(i);
          found = true;
          break;
        }
    }

  // Stored device not found, we assume, it was added by hand.
  // Therefore we do add it to the list too.
  if ( found == false )
    {
#ifndef MAEMO
      GpsDev->addItem( devText );
#else
      // On Maemo we select the first entry, the Maemo GPS daemon as default
      GpsDev->setCurrentIndex(0);
#endif
    }
}

SettingsPageGPS::~SettingsPageGPS()
{
  return;
}

/** Called to initiate loading of the configuration file */
void SettingsPageGPS::slot_load()
{
  GeneralConfig *conf = GeneralConfig::instance();

  GpsAltitude->setCurrentIndex( conf->getGpsAltitude() );

  QString rate = QString::number( conf->getGpsSpeed() );

  for (int i=0; i < GpsSpeed->count(); i++)
    {
      if (GpsSpeed->itemText(i) == rate)
        {
          GpsSpeed->setCurrentIndex(i);
          break;
        }
    }

#ifdef MAEMO
  if( GpsDev->currentText() != "/dev/ttyUSB0" )
    {
      // switch off access to speed box, when USB is not selected
      GpsSpeed->setEnabled( false );
    }
#endif

  if( GpsDev->currentText() == NMEASIM_DEVICE ||
      GpsDev->currentText() == TOMTOM_DEVICE ||
      GpsDev->currentText() == BT_ADAPTER )
    {
      // switch off access to speed box, when NMEA Simulator, TomTom or
      // BT adapter are selected
      GpsSpeed->setEnabled( false );
    }

#ifndef MAEMO
  checkSyncSystemClock->setChecked( conf->getGpsSyncSystemClock() );
#endif
}

/** Called to initiate saving to the configuration file. */
void SettingsPageGPS::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsDevice( GpsDev->currentText() );
  conf->setGpsAltitude( GpsNmea::DeliveredAltitude(GpsAltitude->currentIndex()) );
  conf->setGpsSpeed( GpsSpeed->currentText().toInt() );

#ifndef MAEMO
  conf->setGpsSyncSystemClock( checkSyncSystemClock->isChecked() );
#endif
}

/**
 * Called when the GPS device selection is changed to toggle the access
 * to the GPS speed box in dependency of the necessity.
 */
void SettingsPageGPS::slot_gpsDeviceChanged( const QString& text )
{
  // qDebug("text=%s", text.toLatin1().data());

  if( text == NMEASIM_DEVICE || text == TOMTOM_DEVICE || text == BT_ADAPTER )
    {
      // Switch off access to speed box, when a named pipe is used.
      GpsSpeed->setEnabled( false );
      return;
    }

#ifdef MAEMO
  if( ! text.startsWith("/dev/ttyUSB") )
    {
      // Switch off access to speed box, when USB is not selected.
      // That is done only for Maemo because in those cases a speed
      // entry is not necessary.
      GpsSpeed->setEnabled( false );
      return;
    }
#endif

  GpsSpeed->setEnabled( true );
}
