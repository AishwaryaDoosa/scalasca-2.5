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
#include <pearl/MpiCancelled_rep.h>

#include <iostream>

using namespace std;
using namespace pearl;


//---------------------------------------------------------------------------
//
//  class MpiCancelled_rep
//
//---------------------------------------------------------------------------

//--- Constructors & destructor ---------------------------------------------

MpiCancelled_rep::MpiCancelled_rep(timestamp_t timestamp,
                                   uint64_t    requestId)
  : Event_rep(timestamp),
    mRequestId(requestId),
    m_prev_reqoffs(0)
{
}


MpiCancelled_rep::MpiCancelled_rep(const GlobalDefs& defs, Buffer& buffer)
  : Event_rep(defs, buffer),
    mRequestId(PEARL_NO_REQUEST),   // request ID is only local
    m_prev_reqoffs(0)
{
}


//--- Event type information ------------------------------------------------

event_t MpiCancelled_rep::getType() const
{
  return MPI_CANCELLED;
}


bool MpiCancelled_rep::isOfType(event_t type) const
{
  return ((MPI_CANCELLED == type)
          || (GROUP_NONBLOCK == type)
          || (GROUP_ALL == type));
}


//--- Access event data -----------------------------------------------------

uint64_t MpiCancelled_rep::getRequestId() const
{
  return mRequestId;
}


//--- Modify event data -----------------------------------------------------

void MpiCancelled_rep::setRequestId(uint64_t requestId)
{
  mRequestId = requestId;
}


//--- Generate human-readable output of event data (protected) --------------

ostream& MpiCancelled_rep::output(ostream& stream) const
{
  Event_rep::output(stream);

  return stream << "  reqid  = " << mRequestId << endl;
}


//--- Find next/prev request entries (protected) ----------------------------

uint32_t MpiCancelled_rep::get_prev_reqoffs() const
{
  return m_prev_reqoffs;
}


uint32_t MpiCancelled_rep::get_next_reqoffs() const
{
  return 0;
}


void MpiCancelled_rep::set_prev_reqoffs(uint32_t offs)
{
  m_prev_reqoffs = offs;
}


void MpiCancelled_rep::set_next_reqoffs(uint32_t ptr)
{
}
