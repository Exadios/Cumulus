/***********************************************************************
**
**   layout.cpp
**
**   This file is part of Cumulus.
**
************************************************************************
**
**   Copyright (c):  2012 by Axel Pauli
**
**   This file is distributed under the terms of the General Public
**   License. See the file COPYING for more information.
**
**   $Id$
**
***********************************************************************/

#include "layout.h"

void Layout::adaptFont( QFont& fontRef,
                        const int pxHeight,
                        const int startPointSize,
                        const int minPointSize )
{
  fontRef.setPointSize( startPointSize );

  int start = startPointSize;

  // Adapt the font point size to the given pixel height.
  while( start >= minPointSize )
    {
      QFontMetrics qfm(fontRef);

      if( (qfm.boundingRect("XM").height() ) > pxHeight )
        {
          fontRef.setPointSize( --start );
          continue;
        }
      else
        {
          return;
        }
     }
}