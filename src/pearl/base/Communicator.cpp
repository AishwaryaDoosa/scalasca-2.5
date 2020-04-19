/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2017                                                **
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


/*-------------------------------------------------------------------------*/
/**
 *  @file
 *  @ingroup PEARL_base
 *  @brief   Implementation of the class Communicator.
 *
 *  This file provides the implementation of the class Communicator and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <pearl/Communicator.h>

#include <cstddef>
#include <iostream>

#include <pearl/CommSet.h>
#include <pearl/String.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


// --- Constructors & destructor --------------------------------------------

Communicator::Communicator(const IdType              id,
                           const String&             name,
                           const CommSet&            commSet,
                           const Communicator* const parent)
    : mIdentifier(id),
      mName(name),
      mCommSet(commSet),
      mParent(parent)
{
}


Communicator::~Communicator()
{
}


// --- Query functions ------------------------------------------------------

Communicator::IdType
Communicator::getId() const
{
    return mIdentifier;
}


const String&
Communicator::getName() const
{
    return mName;
}


const CommSet&
Communicator::getCommSet() const
{
    return mCommSet;
}


const Communicator*
Communicator::getParent() const
{
    return mParent;
}


uint32_t
Communicator::getSize() const
{
    return mCommSet.numRanks();
}


Paradigm
Communicator::getParadigm() const
{
    return mCommSet.getParadigm();
}


// --- Related functions ----------------------------------------------------

namespace pearl
{
// --- Stream I/O operators --------------------------------

ostream&
operator<<(ostream&            stream,
           const Communicator& item)
{
    // Special case: undefined communicator
    if (item == Communicator::UNDEFINED)
    {
        return stream << "<undefined>";
    }

    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 13);

    // Print data
    stream << "Communicator {" << iendl(indent)
           << "  id       = " << item.getId() << iendl(indent)
           << "  name     = " << item.getName() << iendl(indent)
           << "  parent   = ";
    const Communicator* const parent = item.getParent();
    if (parent == 0)
    {
        stream << "<none>";
    }
    else
    {
        stream << parent->getId();
    }
    stream << iendl(indent)
           << "  comm set = " << item.getCommSet() << iendl(indent)
           << "}";

    // Reset indentation
    return setIndent(stream, indent);
}


// --- Comparison operators --------------------------------

bool
operator==(const Communicator& lhs,
           const Communicator& rhs)
{
    // Shortcut: compare addresses
    if (&lhs == &rhs)
    {
        return true;
    }

    return (  (lhs.getId() == rhs.getId())
           && (lhs.getName() == rhs.getName())
           && (lhs.getCommSet() == rhs.getCommSet())
           && (lhs.getParent() == rhs.getParent()));
}


bool
operator!=(const Communicator& lhs,
           const Communicator& rhs)
{
    return !(lhs == rhs);
}
}    // namespace pearl
