/***********************************************************************
**
**   gpsclient.cpp
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2012 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>

#include <cerrno>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <ctime>

#include <QtCore>

#include "gpsclient.h"
#include "gpscon.h"
#include "protocol.h"
#include "ipc.h"

#ifdef FLARM
#include "flarmbase.h"
#include "flarmbincomlinux.h"
#endif

#ifdef BLUEZ
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#endif

#ifdef DEBUG
#undef DEBUG
#endif

// Switch this on for permanent error logging. That will display
// all open and reconnect failures. Because open and reconnect
// is done periodically you will get a lot of such message in error
// case.
#ifdef ERROR_LOG
#undef ERROR_LOG
#endif

// Define connection lost timeout in milli seconds
#define TO_CONLOST  10000


GpsClient::GpsClient( const ushort portIn )
{
  device           = "";
  ioSpeedTerminal  = B0;
  ioSpeedDevice    = 0;
  ipcPort          = portIn;
  fd               = -1;
  forwardGpsData   = true;
  connectionLost   = true;
  shutdown         = false;
  datapointer      = databuffer;
  dbsize           = 0;
  badSentences     = 0;
  initFlarm        = false;

  // establish a connection to the server
  if( ipcPort )
    {
      if( clientData.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          qCritical() << "Command channel Connection to Cumulus Server failed!"
                      << "Fatal error, terminate process!" << endl;

          setShutdownFlag(true);
          return;
        }

      if( clientForward.connect2Server( IPC_IP, ipcPort ) == -1 )
        {
          qCritical() << "Forward channel Connection to Cumulus Server failed!"
                      << "Fatal error, terminate process!";

          setShutdownFlag(true);
          return;
        }

      // Set forward channel to non blocking IO
      int fc = clientForward.getSock();
      fcntl( fc, F_SETFL, O_NONBLOCK );
    }
}

GpsClient::~GpsClient()
{
  closeGps();
  clientData.closeSock();
  clientForward.closeSock();
}

/**
 * Return all currently used read file descriptors as mask, usable by the
 * select call
 */
fd_set *GpsClient::getReadFdMask()
{
  // Reset file descriptor mask bits
  FD_ZERO( &fdMask );

  if( fd != -1 ) // serial device
    {
      FD_SET( fd, &fdMask );
    }

  if( ipcPort ) // command data channel to server
    {
      int sfd = clientData.getSock();

      if( sfd != -1 )
        {
          FD_SET( sfd, &fdMask );
        }
    }

  return &fdMask;
}

// Processes incoming read events. They can come from the server or
// from the GGS device.
void GpsClient::processEvent( fd_set *fdMask )
{
  if( ipcPort )
    {
      int sfd = clientData.getSock();

      if( sfd != -1 && FD_ISSET( sfd, fdMask ) )
        {
          int loops = 0;

          // Try to process several messages from the client in order. That
          // is more effective as to wait for a new select call.
          while( loops++ < 32 )
            {
              readServerMsg();

              if( shutdown == true )
                {
                  break;
                }

              // Check, if more bytes are available in the receiver buffer because we
              // use blocking IO.
              int bytes = 0;

              // Number of bytes currently in the socket receiver buffer.
              if( ioctl( sfd, FIONREAD, &bytes) == -1 )
                {
                  qWarning() << "GpsClient::processEvent():"
                              << "ioctl() returns with Errno="
                              << errno
                              << "," << strerror(errno);
                  break;
                }

              if( bytes <= 0 )
                {
                  break;
                }
            }
        }
    }

  if( fd != -1 && FD_ISSET( fd, fdMask ) )
    {
      if( readGpsData() == false )
        {
          // problem occurred, likely buffer overrun. we do restart the GPS
          // receiving.
          int error = errno; // Save errno
          closeGps();

          if( error == ECONNREFUSED )
            {
              // BT devices can reject a connection try. If we don't return here
              // we run in an endless loop.
              setShutdownFlag(true);
            }
          else
            {
              sleep(3);
              // reopen connection
              openGps( device.data(), ioSpeedDevice );
            }
        }
    }
}

// returns true=success / false=unsuccess
bool GpsClient::readGpsData()
{
  if( fd == -1 ) // no connection is active
    {
      return false;
    }

  // First check, if enough space is available in the receiver buffer.
  // If we read only trash for a while we can run in a dead lock.
  int freeSpace = sizeof(databuffer) - dbsize;

  if( freeSpace < 10 )
    {
      // Reset buffer pointer because the minimal free buffer space is
      // reached. That will discard all already read data but we never
      // read a end of line. That is our emergency break.
      datapointer = databuffer;
      dbsize = 0;
    }

  // all available GPS data lines are read successive
  int bytes = 0;

  bytes = read( fd, datapointer, sizeof(databuffer) - dbsize -1 );

  if( bytes == 0 ) // Nothing read, should normally not happen
    {
      qWarning() << "GpsClient::readGpsData(): 0 bytes read!";
      return false;
    }

  if( bytes == -1 )
    {
      qWarning() << "GpsClient::readGpsData(): Read error"
                  << errno << "," << strerror(errno);
      return false;
    }

  if( bytes > 0 )
    {
      dbsize      += bytes;
      datapointer += bytes;

      databuffer[dbsize] = '\0'; // terminate buffer with a null

      readSentenceFromBuffer();

      if( ! initFlarm )
        {
          // update supervision timer/variables
          last.start();
        }

      connectionLost = false;
    }

  return true;
}

// Sends a NMEA sentence to the GPS. Check sum will be calculated by
// this routine. Don't add an asterix at the end of the sentence. It
// will be part of the check sum.
int GpsClient::writeGpsData( const char *sentence )
{
  // don't try to send anything if there is no valid connection
  // available
  if( fd < 0 )
    {
      return -1;
    }

  uchar csum = calcCheckSum( sentence );
  QString check;
  check.sprintf ("*%02X\r\n", csum);
  QString cmd (sentence + check);

  // write sentence to gps device
  int result = write (fd, cmd.toLatin1().data(), cmd.length());

  if (result != -1 && result != cmd.length())
    {
      qWarning() << "GpsClient::writeGpsData Only"
                 << result
                 << "characters were written:"
                 << cmd;
    }

  return result;
}

// Opens the connection to the GPS. All old messages in the queue are removed.
bool GpsClient::openGps( const char *deviceIn, const uint ioSpeedIn )
{
  qDebug() << "GpsClient::openGps:" << deviceIn << "," << ioSpeedIn;

  device          = deviceIn;
  ioSpeedDevice   = ioSpeedIn;
  ioSpeedTerminal = getBaudrate(ioSpeedIn);
  badSentences    = 0;
  unknownsReported.clear();

  if( deviceIn == (const char *) 0 || strlen(deviceIn) == 0 )
    {
      // no valid device has been passed
      return false;
    }

  // reset buffer pointer
  datapointer = databuffer;
  dbsize      = 0;
  memset( databuffer, 0, sizeof(databuffer) );

  if( fd != -1 )
    {
      // closes an existing connection before opening a new one
      closeGps();
      sleep(2);
    }

#ifdef BLUEZ

  // Define a reg. expression for a Bluetooth address like "XX:XX:XX:XX:XX:XX"
  QRegExp regExp("([0-9A-Fa-f]{2,2}:){5,5}[0-9A-Fa-f]{2,2}");

  if( QString(deviceIn).contains(QRegExp( regExp )) )
    {
      fd = socket( AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM );

      // NON blocking io is requested!
      fcntl( fd, F_SETFL, O_NONBLOCK );

      struct sockaddr_rc addr;

      memset( &addr, 0, sizeof (addr) );

      addr.rc_family = AF_BLUETOOTH;

      // 1 is the default channel for a connection to the BT daemon
      addr.rc_channel = (uint8_t) 1;
      str2ba( deviceIn, &addr.rc_bdaddr );

      if( connect( fd, (struct sockaddr *) &addr, sizeof (addr)) == -1 &&
          errno != EINPROGRESS )
        {
          qCritical() << "GpsClient::openGps(): BT connect error"
                      << errno << "," << strerror(errno);

          close( fd );
          fd = -1;

          last = QTime();
          setShutdownFlag(true);
          return false;
        }

      // Stop supervision control to give the BT daemon time for connection.
      // The first data read will activate it again.
      last = QTime();
      return true;
    }

#endif

  // create a fifo for the nmea simulator, if device starts not with /dev/
  if( strncmp( "/dev/", deviceIn, strlen("/dev/") ) != 0 )
    {
      int ret = mkfifo( device.data(), S_IRUSR | S_IWUSR );

      if( ret && errno != EEXIST )
        {
          perror("mkfifo");
        }
    }

  fd = open( device.data(), O_RDWR | O_NOCTTY | O_NDELAY | O_NONBLOCK );

  if( fd == -1 )
    {
      perror( "Open GPS device:" );

      // Could not open the serial device.
#ifdef ERROR_LOG
      qWarning() << "openGps: Unable to open GPS device"
                 << device << "at transfer rate"
                 << ioSpeedIn;
#endif

      last.start(); // store time point for restart control
      return false;
    }

  if( ! isatty(fd) )
    {
      // Fifo needs no serial initialization.
      // Write a notice for the user about that fact
      if( device.startsWith("/dev/") )
        {
          qDebug() << "GpsClient::openGps: Device '"
                    << deviceIn
                    << "' is not a TTY!";
        }
    }
  else
    {
      tcgetattr(fd, &oldtio); // get current options from port

      // copy current values into new structure for changes
      memcpy( &newtio, &oldtio, sizeof(newtio) );

      // http://www.mkssoftware.com/docs/man5/struct_termios.5.asp
      //
      // Prepare serial port settings for raw mode. That is important
      // otherwise Flarm binary communication do not work!
      //
      // - no canonical input (no line oriented input)
      // - 8 data bits
      // - no parity
      // - no interpretation of special characters
      // - no hardware control

      // Port control modes
      // CS8    8 bits per byte
      // CLOCAL Ignore modem status lines
      // CREAD  Enable receiver
      newtio.c_cflag = CS8 | CLOCAL | CREAD;

      // Port input modes
      // raw input without any special handling
      newtio.c_iflag = 0;

      // Port output modes
      // raw output without any special handling
      newtio.c_oflag = 0;

      // Port local modes
      // raw input/output without any special handling
      newtio.c_lflag = 0;

      // The values of the MIN and TIME members of the c_cc array of the termios
      // structure are used to determine how to process the bytes received.
      //
      // MIN represents the minimum number of bytes that should be received when
      // the read() function returns successfully.
      //
      // TIME is a timer of 0.1 second granularity (or as close to that value as
      // can be accommodated) that is used to time out bursty and short-term data
      // transmissions.
      newtio.c_cc[VMIN]  = 1;
      newtio.c_cc[VTIME] = 0;

      // AP: Note, the setting of the speed must be done at last
      // because the manipulation of the c_iflag and c_oflag can
      // destroy the already assigned values! Needed me several hours
      // to find out that. Setting the baud rate under c_cflag seems
      // also to work.
      cfsetispeed( &newtio, ioSpeedTerminal ); // set baud rate for input
      cfsetospeed( &newtio, ioSpeedTerminal ); // set baud rate for output

      tcflush(fd, TCIOFLUSH);
      tcsetattr(fd, TCSANOW, &newtio);

      fcntl(fd, F_SETFL, FNDELAY); // NON blocking io is requested
    }

  last.start(); // store time point for supervision control
  return true;
}

/**
 * This method tries to read all lines contained in the receive buffer. A line
 * is always terminated by a newline and is taken over in the receiver queue,
 * if the checksum is valid and the GPS identifier is requested.
 */
void GpsClient::readSentenceFromBuffer()
{
  char *start = databuffer;
  char *end   = 0;

  while(strlen(start))
    {
      // Search for a newline in the receiver buffer.
      // That is the normal end of a GPS sentence.
      if( ! (end = strchr( start, '\n' )) )
        {
          // No newline in the receiver buffer, wait for more characters
          return;
        }

      if( start == end )
        {
          // skip newline and start at next position with a new search
          start++;
          continue;
        }

      // found a complete record in the buffer, it will be extracted now
      char *record = (char *) malloc( end-start + 2 );

      memset( record, 0, end-start + 2 );

      strncpy( record, start, end-start + 1);

      if( verifyCheckSum( record ) == true )
        {
          // Forward sentence to the server, if checksum is ok and
          // processing is desired.
          if( checkGpsMessageFilter( record ) == true && forwardGpsData == true )
            {
              QByteArray ba;
              ba.append( MSG_GPS_DATA );
              ba.append( ' ' );
              ba.append( record );
              writeForwardMsg( ba.data() );
            }
        }

#ifdef DEBUG
      qDebug() << "GpsClient(): Extracted NMEA Record:" << record;
#endif

      free(record);
      record = 0;

      // remove queued record from receive buffer
      memmove( databuffer, end+1,
               databuffer + sizeof(databuffer)-1 - end );

      datapointer -= (end+1 - databuffer);

      dbsize -= ( end+1 - databuffer);

      start = databuffer;

      end = 0;
    }
}

/**
 * closes the connection to the GPS device
 */
void GpsClient::closeGps()
{
  if( fd != -1 )
    {
      if( isatty(fd) )
        {
          tcflush(fd, TCIOFLUSH);

          //cfsetispeed( &newtio, B0 ); // set baud rate for input
          //cfsetospeed( &newtio, B0 ); // set baud rate for output

          //tcsetattr( fd, TCSANOW, &newtio ); // stop tty
          //tcsetattr( fd, TCSANOW, &oldtio );
        }

      close(fd);
      fd = -1;
    }

  connectionLost = true;
  badSentences   = 0;
  last = QTime();
}

/**
 * Check GPS message key, if it shall be processed or discarded.
 *
 * @returns true if processing desired otherwise false
 */
bool GpsClient::checkGpsMessageFilter( const char *sentence )
{
  QString msgKey( sentence );

  int idx = msgKey.indexOf(",");

  if( idx == -1 )
    {
      return false;
    }

  // Extract message key.
  msgKey = msgKey.left(idx);

  if( gpsMessageFilter.contains( msgKey ) || gpsMessageFilter.isEmpty() )
    {
      // message shall be processed.
      return true;
    }

  if( unknownsReported.contains( msgKey ) == false )
    {
      // Message shall be discarded. We do report that only once.
      unknownsReported.insert( msgKey );
      qWarning() << "GPS sentence discarded!" << sentence;
    }

  return false;
}

/**
 * Verify the checksum of the passed sentences.
 *
 * @returns true (success) or false (error occurred)
 */
bool GpsClient::verifyCheckSum( const char *sentence )
{
  // Filter out wrong data messages read in from the GPS port. Known messages
  // do start with a dollar sign or an exclamation mark.
  if( sentence[0] != '$' && sentence[0] != '!' )
    {
      qWarning() << "GpsClient::CheckSumError:" << sentence;

      badSentences++;

      if( badSentences >= 3 )
        {
          // Close the receiver after 3 bad sentences back-to-back.
          closeGps();
          last.start(); // activate restart control
        }

      return false;
    }

  badSentences = 0;

  for( int i = strlen(sentence) - 1; i >= 0; i-- )
    {
      if( sentence[i] == '*' )
        {
          if( (strlen(sentence) - 1 - i) < 2 )
            {
              // too less characters
              return false;
            }

          char checkBytes[3];
          checkBytes[0] = sentence[i+1];
          checkBytes[1] = sentence[i+2];
          checkBytes[2] = '\0';

          uchar checkSum = (uchar) QString( checkBytes ).toUShort( 0, 16 );

          if( checkSum == calcCheckSum( sentence ) )
            {
              return true;
            }
          else
            {
              return false;
            }
        }
    }

  return false;
}

/** Calculate check sum over NMEA record. */
uchar GpsClient::calcCheckSum( const char *sentence )
{
  uchar sum = 0;

  for( uint i = 1; i < strlen( sentence ); i++ )
    {
      uchar c = (uchar) sentence[i];

      if( c == '$' || c == '!' ) // Start sign will not to be considered
        {
          continue;
        }

      if( c == '*' ) // End of sentence reached
        {
          break;
        }

      sum ^= c;
    }

  return sum;
}

// Timeout controller
void GpsClient::toController()
{
  // Null time is used to switch off the timeout control.
  if( last.isNull() )
    {
      return;
    }

#ifdef FLARM

  if( initFlarm )
    {
      if( last.elapsed() >= 60000  )
        {
          // Enable NMEA output of Flarm after 60s after reset.
          initFlarm = false;
          writeGpsData("$PFLAC,S,NMEAOUT,1");

          // set next retry time point
          last.start();
        }

      return;
    }

#endif

  if( last.elapsed() > TO_CONLOST )
    {
      if( connectionLost == false )
        {
          // connection is lost, send only one message to the server
          connectionLost = true;
          writeForwardMsg( MSG_CON_OFF );
        }

#ifdef ERROR_LOG
      qWarning() << "GpsClient::toController():"
                 << "Connection to GPS seems to be dead, trying restart.";
#endif

      // A timeout occurs from GPS side, when the receiver is
      // switched off or the adapter cable is disconnected.
      // In such a case we close the device, that the OS can
      // release the temporary allocated resource. Otherwise
      // we will block the resource and a reconnect is never
      // possible.
      if( fd != -1 )
        {
          // closes an existing connection before opening a new one
          closeGps();
          sleep(3); // wait before a restart is tried
        }

      // try to reconnect to the GPS receiver
      if( openGps( device.data(), ioSpeedDevice ) == false )
        {
          last.start(); // set next retry time point
        }
    }
}

// Reads a server message from the socket. The protocol consists of
// two parts. First the message length is read as unsigned
// integer, after that the actual message as 8 bit character string.
void GpsClient::readServerMsg()
{
  static const char* method = "GpsClient::readServerMsg():";

  uint msgLen = 0;

  uint done = clientData.readMsg( &msgLen, sizeof(msgLen) );

  if( done < sizeof(msgLen) )
    {
      qWarning() << method << "MSG length" << done << "too short";
      setShutdownFlag(true);
      return; // Error occurred
    }

  if( msgLen > 512 )
    {
      // such messages length are not defined. we will ignore that.
      qWarning() << method
                  << "message" << msgLen << "too large, ignoring it!";

      setShutdownFlag(true);
      return; // Error occurred
    }

  char *buf = new char[msgLen+1];

  memset( buf, 0, msgLen+1 );

  done = clientData.readMsg( buf, msgLen );

  if( done <= 0 )
    {
      qWarning() << method << "MSG data" << done << "too short";
      delete [] buf;
      setShutdownFlag(true);
      return; // Error occurred
    }

#ifdef DEBUG
  qDebug() << method << "Received Message:" << buf;
#endif

  // Split the received message into its two parts. Space is used as separator
  // between the command word and the optional content of the message.
  QString qbuf( buf );

  delete[] buf;
  buf = 0;

  int spaceIdx = qbuf.indexOf( QChar(' ') );

  QStringList args;

  if( spaceIdx == -1 || qbuf.size() == spaceIdx )
    {
      args.append(qbuf);
      args.append("");
    }
  else
    {
      args.append(qbuf.left(spaceIdx));
      args.append(qbuf.mid(spaceIdx+1));
    }

  // look, what the server is requesting
 if( MSG_MAGIC == args[0] )
    {
      // check protocol versions, reply with pos or neg
      if( MSG_PROTOCOL != args[1] )
        {
          qCritical() << method
                      << "Client-Server protocol mismatch!"
                      << "Client:" << MSG_PROTOCOL
                      << "Server:" << args[1];

          writeServerMsg( MSG_NEG );
          setShutdownFlag(true);
          return;
        }

      writeServerMsg( MSG_POS );
    }
  else if( MSG_OPEN == args[0] )
    {
      QStringList devArgs = args[1].split(QChar(' '));

      if( devArgs.size() == 2 )
        {
          // Initialization of GPS device is requested. The message
          // consists of two parts separated by spaces.
          // 1) device name
          // 2) io speed
          bool res = openGps( devArgs[0].toLatin1().data(), devArgs[1].toUInt() );

          if( res )
            {
              writeServerMsg( MSG_POS );
            }
          else
            {
              writeServerMsg( MSG_NEG );
            }
        }
    }
  else if( MSG_CLOSE == args[0] )
    {
      // Close GPS device is requested
      closeGps();
      writeServerMsg( MSG_POS );
    }
  else if( MSG_FGPS_ON == args[0] )
    {
      // Switches on GPS data forwarding to the server.
      forwardGpsData = true;
      writeServerMsg( MSG_POS );
    }
  else if( MSG_FGPS_OFF == args[0] )
    {
      // Switches off GPS data forwarding to the server.
      forwardGpsData = false;
      writeServerMsg( MSG_POS );
    }
  else if( MSG_SM == args[0] && args.count() == 2 )
    {
      // Sent message to the GPS device
      int res = writeGpsData( args[1].toLatin1().data() );

      if( res == -1 )
        {
          writeServerMsg( MSG_NEG );
        }
      else
        {
          writeServerMsg( MSG_POS );
        }
    }
  else if( MSG_GPS_KEYS == args[0] && args.count() == 2 )
    {
      // Well known GPS message keys are received.
      QStringList keys = args[1].split( ",", QString::SkipEmptyParts );

      // Clear old content.
      gpsMessageFilter.clear();

      for( int i = 0; i < keys.size(); i++ )
        {
          gpsMessageFilter.insert( keys.at(i) );
        }

      // qDebug() << "GPS-Keys:" << gpsMessageFilter;
      writeServerMsg( MSG_POS );
    }
  else if( MSG_SHD == args[0] )
    {
      // Shutdown is requested by the server. This message will not be
      // acknowledged!
      setShutdownFlag(true);
    }

#ifdef FLARM

  else if( MSG_FLARM_FLIGHT_LIST_REQ == args[0] )
    {
      // Flarm flight list is requested
      writeServerMsg( MSG_POS );
      getFlarmFlightList();
    }
 else if( MSG_FLARM_FLIGHT_DOWNLOAD == args[0] && args.count() == 2 )
   {
     // Flarm flight download is requested
     writeServerMsg( MSG_POS );
     getFlarmIgcFiles(args[1]);
   }
 else if( MSG_FLARM_RESET == args[0] )
   {
     // Flarm reset is requested
     writeServerMsg( MSG_POS );
     flarmReset();
   }

#endif

  else
    {
      qWarning() << method << "Unknown message received:" << qbuf;
      writeServerMsg( MSG_NEG );
    }

  return;
}

/**
 * Send a message via data socket to the server. The protocol consists of two
 * parts. First the message length is transmitted as unsigned integer, after
 * that the actual message as 8 bit character string.
 */
void GpsClient::writeServerMsg( const char *msg )
{
  uint msgLen = strlen( msg );

  int done = clientData.writeMsg( (char *) &msgLen, sizeof(msgLen) );

  done = clientData.writeMsg( (char *) msg, msgLen );

  if( done < 0 )
    {
      // Error occurred, make shutdown of process
      setShutdownFlag(true);
    }

  return;
}

/**
 * Sends a data message via the forward channel to the server. The protocol consists
 * of two parts. First the message length is transmitted as unsigned integer,
 * after that the actual message as 8 bit character string.
 */
void GpsClient::writeForwardMsg( const char *msg )
{
  static QString method = "GpsClient::writeForwardMsg():";

  // The message to be transfered starts with the message length.
  uint msgLen = strlen( msg );

  QByteArray ba = QByteArray::fromRawData( (const char *) &msgLen, sizeof(msgLen) );
  ba.append( msg );

  // We use non blocking IO for the transfer. Therefore we have to consider some
  // special return codes.
  int done = clientForward.writeMsg( (char *) ba.data(), ba.size() );

  if( done < 0 )
    {
      if( errno == EWOULDBLOCK )
        {
          // The write call would block because the transfer queue is full.
          // In this case we discard the message.
          qWarning() << method
                      << "Write would block, drop Message!";
        }
      else
        {
          // Fatal error occurred, make shutdown of process.
          setShutdownFlag(true);
        }
    }

#ifdef DEBUG
  qDebug() << method << msg;
#endif

  return;
}

/**
 * Translates the baud rate to a terminal speed definition.
 */
uint GpsClient::getBaudrate(int rate)
{
  switch (rate)
    {
    case 600:
      return B600;
    case 1200:
      return B1200;
    case 2400:
      return B2400;
    case 4800:
      return B4800;
    case 9600:
      return B9600;
    case 19200:
      return B19200;
    case 38400:
      return B38400;
    case 57600:
      return B57600;
    case 115200:
      return B115200;
    case 230400:
      return B230400;
    default:
      return B4800;
    }
}

#ifdef FLARM

bool GpsClient::flarmBinMode()
{
  // Binary switch command for Flarm interface
  const char* pflax = "$PFLAX\n";

  FlarmBinComLinux fbc( fd );

  // Precondition is that the NMEA output of the Flarm device was disabled by
  // the calling method before!

  // qDebug() << "Switch Flarm to binary mode";

  // I made the experience, that the Flarm device did not answer to the first
  // binary transfer switch. Therefore I make several tries. Flarm tool makes
  // the same, as I could observe with a RS232 port sniffer.
  bool pingOk = false;
  int loop = 5;

  while( loop-- )
    {
      // Switch connection to binary mode.
      if( write( fd, pflax, strlen(pflax) ) <= 0 )
        {
          // write failed
          break;
        }

      // Check connection with a ping command.
      if( fbc.ping() == true )
        {
          FlarmBase::setProtocolMode( FlarmBase::binary );
          pingOk = true;
          break;
        }
    }

  if( pingOk == false )
    {
      // Switch to binary mode failed
      qWarning() << "GpsClient::flarmBinMode(): Switch failed!";
    }

  return pingOk;
}

void GpsClient::getFlarmFlightList()
{
  // Switch off timeout control
  last = QTime();

  FlarmBinComLinux fbc( fd );

  if( flarmBinMode() == false )
    {
      flarmFlightListError();
      return;
    }

  // read out flight header records
  int recNo = 0;
  char buffer[MAXSIZE];
  QStringList flights;

  while( true )
    {
      if( fbc.selectRecord( recNo ) == true )
        {
          recNo++;

          if( fbc.getRecordInfo( buffer ) )
            {
              flights << QString( buffer );
            }
          else
            {
              qWarning() << "GpsClient::getFlarmFlightList(): GetRecordInfo("
                          << (recNo - 1)
                          << ") failed!";
              break;
            }
        }
      else
        {
          // No more records available
          break;
        }
    }

  // Send back flight headers to application
  QByteArray ba;
  ba.append( MSG_FLARM_FLIGHT_LIST_RES );
  ba.append( " " );

  if( flights.size() )
    {
      ba.append( flights.join("\n") );
    }
  else
    {
      ba.append( " Empty" );
    }

  writeForwardMsg( ba.data() );
}

void GpsClient::flarmFlightListError()
{
  QByteArray ba;
  ba.append( MSG_FLARM_FLIGHT_LIST_RES );
  ba.append( " Error" );
  writeForwardMsg( ba.data() );

  // qDebug() << ba;
}

void GpsClient::getFlarmIgcFiles(QString& args)
{
  // The argument string contains at the first position the destination directory
  // for the files and then the indexes of the flights separated by vertical tabs.
  QStringList idxList = args.split("\v");

  if( idxList.size() < 2 )
    {
      return;
    }

  // Switch off timeout control
  last = QTime();

  FlarmBinComLinux fbc( fd );

  if( flarmBinMode() == false )
    {
      flarmFlightDowloadInfo( "Error" );
      return;
    }

  // read out flights
  char buffer[MAXSIZE];
  uint progress = 0;

  // Check, if the download directory exists. Here we take the directory element
  // from the list.
  QDir igcDir( idxList.takeFirst() );

  if( ! igcDir.exists() )
    {
      if( ! igcDir.mkpath( igcDir.absolutePath() ) )
        {
          flarmFlightDowloadInfo( "Error create directory" );
          return;
        }
    }

  for( int idx = 0; idx < idxList.size(); idx++ )
    {
      // Select the flight to be downloaded
      int recNo = idxList.at(idx).toInt();
      QStringList flightData;

      if( fbc.selectRecord(recNo ) == true )
        {
          // read flight header data
          if( fbc.getRecordInfo( buffer ) )
            {
              flightData = QString( buffer ).split("|");
            }
          else
            {
              // Entry not available, although select answered positive!
              // Not conform to the specification.
              flarmFlightDowloadInfo( "Error" );
              return;
            }

          // Open an IGC file for writing download data.
          QFile f( igcDir.absolutePath() + "/" + flightData.at(0) );

          if( ! f.open( QIODevice::WriteOnly ) )
            {
              // could not open file ...
              qWarning() << "Cannot open file: " << f.fileName();
              flarmFlightDowloadInfo( "Error open file" );
              return;
            }

          uint lastProgress = -1;
          bool eof = false;

          while( fbc.getIGCData(buffer, &progress) )
            {
              if( lastProgress != progress )
                {
                  // That eliminates a lot of intermediate steps
                  flarmFlightDowloadProgress(recNo, progress);
                  lastProgress = progress;
                }

              if( buffer[strlen(buffer) - 1] == 0x1A )
                {
                  // EOF was send by the Flarm, remove it from the data stream.
                   buffer[strlen(buffer) - 1] = '\0';
                   eof = true;
                 }

              f.write(buffer);

              if( eof )
                {
                  break;
                }
            }

          f.close();
        }
     }

  flarmFlightDowloadInfo( "Finished" );
}

void GpsClient::flarmFlightDowloadInfo( QString info )
{
  QString msg = QString("%1 %2").arg(MSG_FLARM_FLIGHT_DOWNLOAD_INFO).arg(info);

  writeForwardMsg( msg.toAscii().data() );
}

/** Reports the flight download progress to the calling application. */
void GpsClient::flarmFlightDowloadProgress( const int idx, const int progress )
{
  QString msg = QString("%1 %2,%3").arg(MSG_FLARM_FLIGHT_DOWNLOAD_PROGRESS)
                                   .arg(idx)
                                   .arg(progress);

  writeForwardMsg( msg.toAscii().data() );
}

bool GpsClient::flarmReset()
{
  // Swich off timeout control
  last = QTime();

  if( ! flarmBinMode() )
    {
      last.start();
      return false;
    }

  FlarmBinComLinux fbc( fd );
  bool res = fbc.exit();
  initFlarm = true;
  last.start();
  return res;
}

#endif
