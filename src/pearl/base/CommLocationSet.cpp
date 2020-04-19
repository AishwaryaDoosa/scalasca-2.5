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
 *  @brief   Implementation of the class CommLocationSet.
 *
 *  This file provides the implementation of the class CommLocationSet.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <pearl/CommLocationSet.h>

#include <iostream>

#include <pearl/Location.h>
#include <pearl/String.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


// --- Constructors & destructor --------------------------------------------

CommLocationSet::CommLocationSet(const IdType                     id,
                                 const String&                    name,
                                 const Paradigm                   paradigm,
                                 const vector< const Location* >& members)
    : LocationSet(id, name, members),
      mParadigm(paradigm)
{
}


// --- Query functions ------------------------------------------------------

GroupingSet::SetType
CommLocationSet::getType() const
{
    return GroupingSet::COMM_LOCATION_SET;
}


Paradigm
CommLocationSet::getParadigm() const
{
    return mParadigm;
}


// --- Stream I/O functions (private) ---------------------------------------

ostream&
CommLocationSet::output(ostream& stream) const
{
    // Special case: undefined comm location set
    if (this == &CommLocationSet::UNDEFINED)
    {
        return stream << "<undefined>";
    }

    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 13);

    // Print comm location set data
    const size_t numLocations = this->numLocations();
    stream << "CommLocationSet {" << iendl(indent)
           << "  id       = " << getId() << iendl(indent)
           << "  name     = " << getName() << iendl(indent)
           << "  paradigm = " << getParadigm() << iendl(indent)
           << "  size     = " << numLocations << iendl(indent)
           << "  members  = [";
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
