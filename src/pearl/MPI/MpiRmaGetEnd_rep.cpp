/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2013                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2009-2013                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


#include <config.h>
#include <pearl/MpiRmaGetEnd_rep.h>

using namespace pearl;


//---------------------------------------------------------------------------
//
//  class MpiRmaGetEnd_rep
//
//---------------------------------------------------------------------------

//--- Constructors & destructor ---------------------------------------------

MpiRmaGetEnd_rep::MpiRmaGetEnd_rep(timestamp_t timestamp,
                         uint32_t    rma_id)
  : RmaGetEnd_rep(timestamp, rma_id)
{
}


MpiRmaGetEnd_rep::MpiRmaGetEnd_rep(const GlobalDefs& defs, Buffer& buffer)
  : RmaGetEnd_rep(defs, buffer)
{
}


//--- Event type information ------------------------------------------------

event_t MpiRmaGetEnd_rep::getType() const
{
  return RMA_GET_END;
}


bool MpiRmaGetEnd_rep::isOfType(event_t type) const
{
  return ((MPI_RMA_GET_END == type)
          || (RMA_GET_END == type)
          || (GROUP_ALL == type));
}
