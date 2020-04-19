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
#include "UnknownLocationGroup.h"

#include <iostream>

#include <pearl/String.h>
#include <pearl/SystemNode.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


/*-------------------------------------------------------------------------*/
/**
 *  @file
 *  @ingroup PEARL_base
 *  @brief   Implementation of the class UnknownLocationGroup.
 *
 *  This file provides the implementation of the class UnknownLocationGroup
 *  and related functions.
 **/
/*-------------------------------------------------------------------------*/


//--- Constructors & destructor ---------------------------------------------

UnknownLocationGroup::UnknownLocationGroup(const IdType      id,
                                           const String&     name,
                                           SystemNode* const parent)
    : LocationGroup(id, name, parent)
{
}


//--- Access definition data ------------------------------------------------

LocationGroup::Type
UnknownLocationGroup::getType() const
{
    return LocationGroup::TYPE_UNKNOWN;
}


//--- Private methods -------------------------------------------------------

/// @brief Stream output helper function.
///
/// Prints the location group's attributes to the given @a stream. This
/// member function is triggered by the stream output operator for objects
/// of type LocationGroup.
///
/// @param  stream  Output stream
///
/// @return Output stream
///
ostream&
UnknownLocationGroup::output(ostream& stream) const
{
    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 11);

    // Print data
    stream << "UnknownLocationGroup {" << iendl(indent)
           << "  id     = " << getId() << iendl(indent)
           << "  name   = " << getName() << iendl(indent)
           << "  parent = ";
    SystemNode* parent = getParent();
    if (NULL == parent) {
        stream << "<undefined>";
    } else {
        stream << "SystemNode(" << parent->getId() << ")";
    }
    stream << iendl(indent)
           << "}";

    // Reset indentation
    return setIndent(stream, indent);
}
