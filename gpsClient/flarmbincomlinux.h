/***********************************************************************
**
**   flarmbincomlinux.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

#ifndef FLARM_BIN_COM_LINUX_H_
#define FLARM_BIN_COM_LINUX_H_

#include "flarmbincom.h"

/**
 * \class FlarmBinComLinux
 *
 * \author Axel Pauli
 *
 * \date 2012
 *
 * \brief Flarm binary low level communication routines for Linux.
 *
 * \version $Id$
 *
 */

class FlarmBinComLinux : public FlarmBinCom
{
 public:

  /**
   * Contructor of class.
   *
   * \param socket Opened socket to Flarm device.
   */
  FlarmBinComLinux(int socket);

  virtual ~FlarmBinComLinux();

 protected:

  /** Low level write character port method. */
  virtual int writeChar(const unsigned char c);

  /** Low level read character port method. */
  virtual int readChar(unsigned char* b);

 private:

  /** Socket to Flarm device. */
  int m_Socket;
};

#endif /* FLARM_BIN_COM_LINUX_H_ */
