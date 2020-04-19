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
#include <pearl/RmaPutEnd_rep.h>

#include <iostream>

#include <pearl/Buffer.h>

using namespace std;
using namespace pearl;


//---------------------------------------------------------------------------
//
//  class RmaPutEnd_rep
//
//---------------------------------------------------------------------------

//--- Constructors & destructor ---------------------------------------------

RmaPutEnd_rep::RmaPutEnd_rep(timestamp_t timestamp,
                             uint32_t    rma_id)
  : Event_rep(timestamp),
    m_rma_id(rma_id)
{
}


RmaPutEnd_rep::RmaPutEnd_rep(const GlobalDefs& defs, Buffer& buffer)
  : Event_rep(defs, buffer)
{
    m_rma_id = buffer.get_uint32();    
}


//--- Event type information ------------------------------------------------

event_t RmaPutEnd_rep::getType() const
{
  return RMA_PUT_END;
}


bool RmaPutEnd_rep::isOfType(event_t type) const
{
  return ((RMA_PUT_END == type)
          || (GROUP_ALL == type));
}


//--- Access event data -----------------------------------------------------

uint32_t RmaPutEnd_rep::get_rma_id() const
{
  return m_rma_id;
}


//--- Serialize event data (protected) --------------------------------------

void RmaPutEnd_rep::pack(Buffer& buffer) const
{
  Event_rep::pack(buffer);

  buffer.put_uint32(m_rma_id);
}


//--- Generate human-readable output of event data (protected) --------------

ostream& RmaPutEnd_rep::output(ostream& stream) const
{
  Event_rep::output(stream);

  return stream << "  rma id   = " << m_rma_id << endl;
}
