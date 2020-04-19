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
#include <pearl/RemoteEvent.h>

#include <iostream>

#include <pearl/Buffer.h>
#include <pearl/Event.h>
#include <pearl/GlobalDefs.h>

#include "EventFactory.h"
#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


//---------------------------------------------------------------------------
//
//  class RemoteEvent
//
//---------------------------------------------------------------------------

//--- Constructors & destructor ---------------------------------------------

RemoteEvent::RemoteEvent(const GlobalDefs& defs,
                         Buffer&           buffer)
{
    // Unpack data
    event_t type = static_cast<event_t>(buffer.get_uint32());
    m_location     = const_cast<Location*>(&defs.getLocation(buffer.get_uint64()));
    m_cnode        = defs.get_cnode(buffer.get_id());
    m_remote_index = buffer.get_uint32();
    m_event        = CountedPtr<Event_rep>(
        EventFactory::instance()->createEvent(type, defs, buffer));
}


//--- Access remote event information ---------------------------------------

Callpath*
RemoteEvent::get_cnode() const
{
    return m_cnode;
}


const Location&
RemoteEvent::get_location() const
{
    return *m_location;
}


uint32_t
RemoteEvent::get_remote_index() const
{
    return m_remote_index;
}


//--- Access event representation -------------------------------------------

Event_rep&
RemoteEvent::operator*() const
{
    return *m_event.get();
}


Event_rep*
RemoteEvent::operator->() const
{
    return m_event.get();
}


//--- Private methods -------------------------------------------------------

ostream&
RemoteEvent::output(ostream& stream) const
{
    // Print data
    int indent = getIndent(stream);
    setIndent(stream, indent + 11);
    stream << "RemoteEvent {" << iendl(indent)
           << "  type   = " << event_typestr(m_event->getType()) << iendl(indent)
           << "  loc    = " << *m_location << iendl(indent)
           << "  index  = " << m_remote_index << iendl(indent);
    m_event->output(stream);
    stream << "}";

    return setIndent(stream, indent);
}


//---------------------------------------------------------------------------
//
//  Related functions
//
//---------------------------------------------------------------------------

ostream&
pearl::operator<<(ostream&           stream,
                  const RemoteEvent& event)
{
    return event.output(stream);
}
