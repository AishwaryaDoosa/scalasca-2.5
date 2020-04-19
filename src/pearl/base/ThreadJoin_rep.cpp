/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2014                                                **
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
#include <pearl/ThreadJoin_rep.h>

#include <iostream>

#include <pearl/Buffer.h>

using namespace std;
using namespace pearl;


//---------------------------------------------------------------------------
//
//  class ThreadJoin_rep
//
//---------------------------------------------------------------------------

//--- Constructors & destructor ---------------------------------------------

ThreadJoin_rep::ThreadJoin_rep(timestamp_t timestamp, Paradigm paradigm)
  : Event_rep(timestamp),
    mParadigm(paradigm)
{
}


ThreadJoin_rep::ThreadJoin_rep(const GlobalDefs& defs, Buffer& buffer)
  : Event_rep(defs, buffer)
{
  mParadigm = Paradigm(buffer.get_uint32());    
}


//--- Event type information ------------------------------------------------

event_t ThreadJoin_rep::getType() const
{
  return THREAD_JOIN;
}


bool ThreadJoin_rep::isOfType(event_t type) const
{
  return ((THREAD_JOIN == type)
          || (GROUP_ALL == type));
}


//--- Access event data -----------------------------------------------------

Paradigm ThreadJoin_rep::getParadigm() const
{
  return mParadigm;
}


//--- Serialize event data (protected) --------------------------------------

void ThreadJoin_rep::pack(Buffer& buffer) const
{
  Event_rep::pack(buffer);

  buffer.put_uint32(native_value(mParadigm));
}


//--- Generate human-readable output of event data (protected) --------------

ostream& ThreadJoin_rep::output(ostream& stream) const
{
  Event_rep::output(stream);

  return stream << "  paradigm = "<< mParadigm << endl;
}
