/***********************************************************************
**
**   protocol.h
**
**   This file is part of Cumulus
**
************************************************************************
**
**   Copyright (c):  2004-2011 by Axel Pauli (axel@kflog.org)
**
**   This program is free software; you can redistribute it and/or modify
**   it under the terms of the GNU General Public License as published by
**   the Free Software Foundation; either version 2 of the License, or
**   (at your option) any later version.
**
**   $Id$
**
***********************************************************************/

/**
 * This file contains the message keys used during interprocess communication
 * between the Cumulus and the GPS client process.
 */

#ifndef _Protocol_h_
#define _Protocol_h_

// Message key word definitions

#define MSG_PROTOCOL   "Cumulus-GPS_Client_IPC_V1.3_Axel@kflog.org"

#define MSG_MAGIC      "\\Magic\\"

#define MSG_POS		     "\\Positive\\"

#define MSG_NEG		     "\\Negative\\"

// data available notification

#define MSG_DA		     "\\Data_Available\\"

// open connection to the GPS device "Open" <device> <speed>

#define MSG_OPEN	     "\\Open\\"

// close connection to the GPS device

#define MSG_CLOSE	     "\\Close\\"

// get all available messages from the queue

#define MSG_GM		     "\\Get_Msg\\"

// reply the next available message from the queue

#define MSG_RM		     "\\Reply_Msg\\"

// reply the available messages count of the queue

#define MSG_RMC        "\\Reply_Msg_Cnt\\"

// send message to GPS device

#define MSG_SM		     "\\Send_Msg\\"

// notify request

#define MSG_NTY		     "\\Notify\\"

// shutdown request

#define MSG_SHD	       "\\Shutdown\\"

// GPS message keys to be processed.

#define MSG_GPS_KEYS   "\\GPS_Msg_Keys\\"

//------- Other message strings-------//

#define MSG_CON_OFF     "#GPS_Connection_off#"

#define MSG_CON_ON      "#GPS_Connection_on#"

#endif  // #ifndef _Protocol_h_
