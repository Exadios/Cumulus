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

/** This widget is used to define the GPS interface parameters.
 *  The user can select different source devices and some special
 *  GPS parameters.
 *
 *  There is a difference in the provided options between normal Desktop
 *  and Maemo. Under Maemo RS232 devices are supported only via USB.
 */

#include <QLabel>
#include <QLineEdit>
#include <QGridLayout>

#include "generalconfig.h"
#include "settingspagegps.h"
#include "gpsnmea.h"
#include "gpscon.h"
#include "hwinfo.h"

SettingsPageGPS::SettingsPageGPS(QWidget *parent) : QWidget(parent)
{
  setObjectName("SettingsPageGPS");
  QGridLayout * topLayout = new QGridLayout(this);

  int row=0;

  topLayout->addWidget(new QLabel(tr("GPS Device:"), this), row, 0);
  GpsDev = new QComboBox(this);
  GpsDev->setObjectName ("GPSDevice");
  topLayout->addWidget(GpsDev, row++, 1);
  GpsDev->setEditable(true);
  topLayout->setColumnStretch(2, 10);

#ifndef MAEMO
  GpsDev->addItem("/dev/ttyS0");
  GpsDev->addItem("/dev/ttyS1");
  GpsDev->addItem("/dev/ttyS2");
  GpsDev->addItem("/dev/ttyS3");
  GpsDev->addItem("/dev/ttyUSB0"); // external USB device
  // Bluetooth default devices
  GpsDev->addItem("/dev/rfcomm0");
  GpsDev->addItem("/dev/rfcomm1");
  // add entry for NMEA simulator choice
  GpsDev->addItem(NMEASIM_DEVICE);
#else
  GpsDev->setEditable(false);  // forbid edit for the user

  // Under Maemo there are only three predefined sources.
  GpsDev->addItem(MAEMO_LOCATION_SERVICE); // Maemo GPS Location Service
  GpsDev->addItem("/dev/ttyUSB0"); // external USB device
  GpsDev->addItem(NMEASIM_DEVICE); // Cumulus NMEA simulator
#endif

  // catch selection changes of the GPS device combo box
  connect( GpsDev, SIGNAL(activated(const QString &)),
           this, SLOT(slot_gpsDeviceChanged(const QString&)) );

  topLayout->addWidget(new QLabel(tr("Transfer rate (bps):"), this), row, 0);
  GpsSpeed = new QComboBox(this);
  GpsSpeed->setObjectName("GPSSpeed");
  GpsSpeed->setEditable(false);
  topLayout->addWidget(GpsSpeed, row++, 1);
  GpsSpeed->addItem("115200");
  GpsSpeed->addItem("57600");
  GpsSpeed->addItem("38400");
  GpsSpeed->addItem("19200");
  GpsSpeed->addItem("9600");
  GpsSpeed->addItem("4800");
  GpsSpeed->addItem("2400");
  GpsSpeed->addItem("1200");
  GpsSpeed->addItem("600");

  // @AP: Some GPS CF Cards (e.g. BC-307) deliver only height above the WGS 84
  // ellipsoid in GGA record. This is not derivable from the received
  // record. Therefore we need an additional configuration entry :(
  topLayout->addWidget(new QLabel(tr("Altitude Reference:"), this),row,0);
  GpsAltitude = new QComboBox(this);
  GpsAltitude->setObjectName("GPSAltitude");
  GpsAltitude->setEditable(false);
  topLayout->addWidget(GpsAltitude,row++,1);
  GpsAltitude->addItem(tr("MSL"));
  GpsAltitude->addItem(tr("HAE"));
  GpsAltitude->addItem(tr("User"));
  GpsAltitude->addItem(tr("Pressure"));

  connect( GpsAltitude, SIGNAL(activated(int )),
           this, SLOT(slot_altitude_mode(int )) );
  topLayout->setColumnStretch(2,10);

  //AS: Some GPS units (like the Pretec) don't include any form of HAE correction.
  //For these, the user can manually enter the correction.
  topLayout->addWidget(new QLabel(tr("Altitude Correction:"), this),row,0);
  spinUserCorrection = new QSpinBox(this);
  spinUserCorrection->setObjectName("GPSAltitudeCorrection");
  spinUserCorrection->setMinimum(-1000);
  spinUserCorrection->setMaximum(1000);
  spinUserCorrection->setButtonSymbols(QSpinBox::PlusMinus);
  topLayout->addWidget(spinUserCorrection,row++,1);

  connect( spinUserCorrection, SIGNAL(valueChanged(int)),
           this, SLOT(slot_spinUserCorrectionChanged(int)) );

#ifndef MAEMO
  topLayout->setRowMinimumHeight( row++, 10);

  checkSoftStart = new QCheckBox (tr("Soft start"), this);
  topLayout->addWidget(checkSoftStart, row, 0 );
  row++;

  checkHardStart = new QCheckBox (tr("Hard start"), this);
  topLayout->addWidget(checkHardStart, row, 0 );
  row++;

  checkSyncSystemClock = new QCheckBox (tr("Update system clock"), this);
  topLayout->addWidget(checkSyncSystemClock, row, 0 );
  row++;

  topLayout->setRowStretch(row++, 10);

  buttonReset = new QPushButton (tr("Reset to factory settings"), this);
  topLayout->addWidget(buttonReset, row, 2, Qt::AlignRight);
  row++;

  connect( buttonReset, SIGNAL(clicked()),
           GpsNmea::gps, SLOT(sendFactoryReset()) );

#else
  topLayout->setRowStretch(row++,10);
#endif

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

  spinUserCorrection->setValue( (int) conf->getGpsUserAltitudeCorrection().getMeters() );

  slot_altitude_mode( conf->getGpsAltitude() );

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

  if( GpsDev->currentText() == NMEASIM_DEVICE )
    {
      // switch off access to speed box, when NMEA Simulator is selected
      GpsSpeed->setEnabled( false );
    }

  checkSoftStart->setChecked( conf->getGpsSoftStart() );
  checkHardStart->setChecked( conf->getGpsHardStart() );
  checkSyncSystemClock->setChecked( conf->getGpsSyncSystemClock() );
}

/** Called to initiate saving to the configuration file. */
void SettingsPageGPS::slot_save()
{
  GeneralConfig *conf = GeneralConfig::instance();

  conf->setGpsDevice( GpsDev->currentText() );
  conf->setGpsAltitude( GpsNmea::DeliveredAltitude(GpsAltitude->currentIndex()) );

  if ( GpsAltitude->currentIndex() == GpsNmea::USER )
    {
      conf->setGpsUserAltitudeCorrection( Altitude(spinUserCorrection->value()) );
    }
  else
    {
      conf->setGpsUserAltitudeCorrection( 0 );
    }

  conf->setGpsSpeed( GpsSpeed->currentText().toInt() );
  conf->setGpsHardStart( checkHardStart->isChecked() );
  conf->setGpsSoftStart( checkSoftStart->isChecked() );
  conf->setGpsSyncSystemClock( checkSyncSystemClock->isChecked() );
}

void SettingsPageGPS::slot_altitude_mode(int mode)
{
  spinUserCorrection->setEnabled( mode == GpsNmea::USER );
}

/**
 * Called when the GPS device selection is changed to toggle the access
 * to the GPS speed box in dependency of the necessity.
 */
void SettingsPageGPS::slot_gpsDeviceChanged( const QString& text )
{
  // qDebug("text=%s", text.toLatin1().data());

  if( text == NMEASIM_DEVICE )
    {
      // Switch off access to speed box, when NMEA Simulator is selected.
      GpsSpeed->setEnabled( false );
      return;
    }

#ifdef MAEMO
  if( text != "/dev/ttyUSB0" )
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

/**
 * Called when the user correction value is changed.
 */
void SettingsPageGPS::slot_spinUserCorrectionChanged( int newValue )
{
  if( newValue <= 0 )
    {
      spinUserCorrection->setPrefix("");
    }
  else
    {
      spinUserCorrection->setPrefix("+");
    }
}

