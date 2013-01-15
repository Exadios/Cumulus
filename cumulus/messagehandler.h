/***********************************************************************
**
**   messagehandler.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2005-2013 by Axel Pauli <kflog.cumulus@gmail.com>
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

#ifndef MESSAGE_HANDLER_H
#define MESSAGE_HANDLER_H

/**
 * \author Axel Pauli <kflog.cumulus@gmail.com>
 *
 * \brief A own message handler function for Cumulus.
 *
 * This function is the implementation of an own Qt Message handler and
 * is used to handle the messages generated by Cumulus through using
 * qDebug, qWarning, qFatal. According to the configuration value sysLogMode
 * all messages will forward to stderr or to the system logger.
 *
 * \date 2005-2013
 *
 * \version $Id$
 **/

#include <qglobal.h>

void messageHandler(QtMsgType type, const char* msg);

#endif
