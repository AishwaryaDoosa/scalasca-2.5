/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2015                                                **
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
#include <pearl/MpiReceiveComplete_rep.h>

#include <iostream>

using namespace std;
using namespace pearl;


//---------------------------------------------------------------------------
//
//  class MpiReceiveComplete_rep
//
//---------------------------------------------------------------------------

//--- Constructors & destructor ---------------------------------------------

MpiReceiveComplete_rep::MpiReceiveComplete_rep(timestamp_t timestamp,
                                               MpiComm*    communicator,
                                               uint32_t    source,
                                               uint32_t    tag,
                                               uint64_t    bytesReceived,
                                               uint64_t    requestId)
  : MpiReceive_rep(timestamp, communicator, source, tag, bytesReceived),
    mRequestId(requestId),
    m_prev_reqoffs(0)
{
}


MpiReceiveComplete_rep::MpiReceiveComplete_rep(const GlobalDefs& defs,
                                               Buffer&           buffer)
  : MpiReceive_rep(defs, buffer),
    mRequestId(PEARL_NO_REQUEST),   // request ID is only local
    m_prev_reqoffs(0)
{

}


//--- Event type information ------------------------------------------------

event_t MpiReceiveComplete_rep::getType() const
{
  return MPI_RECV_COMPLETE;
}


bool MpiReceiveComplete_rep::isOfType(event_t type) const
{
  return ((MPI_RECV_COMPLETE == type)
          || (GROUP_RECV == type)
          || (GROUP_NONBLOCK == type)
          || (GROUP_ALL == type));
}


//--- Access event data -----------------------------------------------------

uint64_t MpiReceiveComplete_rep::getRequestId() const
{
  return mRequestId;
}


//--- Modify event data -----------------------------------------------------

void MpiReceiveComplete_rep::setRequestId(uint64_t requestId)
{
  mRequestId = requestId;
}


//--- Generate human-readable output of event data (protected) --------------

ostream& MpiReceiveComplete_rep::output(ostream& stream) const
{
  MpiReceive_rep::output(stream);

  return stream << "  reqid  = " << mRequestId << endl;
}


//--- Find next/prev request entries (protected) ----------------------------

uint32_t MpiReceiveComplete_rep::get_prev_reqoffs() const
{
  return m_prev_reqoffs;
}


void MpiReceiveComplete_rep::set_prev_reqoffs(uint32_t offs)
{
  m_prev_reqoffs = offs;
}
