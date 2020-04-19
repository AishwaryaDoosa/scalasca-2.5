/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2017                                                     **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


/*-------------------------------------------------------------------------*/
/**
 *  @file
 *  @ingroup PEARL_base
 *  @brief   Implementation of the class LocationSet.
 *
 *  This file provides the implementation of the class LocationSet.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <pearl/LocationSet.h>

#include <iostream>

#include <pearl/Error.h>
#include <pearl/Location.h>
#include <pearl/String.h>
#include <pearl/Utils.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


// --- Constructors & destructor --------------------------------------------

LocationSet::LocationSet(const IdType                     id,
                         const String&                    name,
                         const vector< const Location* >& members)
    : GroupingSet(id, name),
      mLocations(members)
{
}


// --- Query functions ------------------------------------------------------

GroupingSet::SetType
LocationSet::getType() const
{
    return GroupingSet::LOCATION_SET;
}


size_t
LocationSet::numLocations() const
{
    return mLocations.size();
}


const Location&
LocationSet::getLocation(const size_t rank) const
{
    if (rank >= mLocations.size())
    {
        throw RuntimeError("pearl::LocationSet::"
                           "getLocation(std::size_t) -- "
                           "invalid rank: " + toStdString(rank));
    }

    return *mLocations[rank];
}


// --- Stream I/O functions (private) ---------------------------------------

ostream&
LocationSet::output(ostream& stream) const
{
    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 12);

    // Print location set data
    const size_t numLocations = this->numLocations();
    stream << "LocationSet {" << iendl(indent)
           << "  id      = " << getId() << iendl(indent)
           << "  name    = " << getName() << iendl(indent)
           << "  size    = " << numLocations << iendl(indent)
           << "  members = [";
    setIndent(stream, indent + 4);
    for (size_t i = 0; i < numLocations; ++i)
    {
        stream << iendl(indent + 4)
               << getLocation(i);
    }
    stream << iendl(indent)
           << "  ]" << iendl(indent)
           << "}";

    // Reset indentation
    return setIndent(stream, indent);
}
