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
#include <pearl/Callsite.h>

#include <iostream>

#include <pearl/Region.h>
#include <pearl/String.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


/*-------------------------------------------------------------------------*/
/**
 *  @file    Callsite.cpp
 *  @ingroup PEARL_base
 *  @brief   Implementation of the class Callsite.
 *
 *  This file provides the implementation of the class Callsite and related
 *  functions.
 **/
/*-------------------------------------------------------------------------*/


//--- Constructors & destructor ---------------------------------------------

Callsite::Callsite(const IdType   id,
                   const String&  filename,
                   const uint32_t line,
                   const Region&  callee)
    : mIdentifier(id),
      mFilename(filename),
      mLine(line),
      mCallee(callee)
{
}


//--- Access definition data ------------------------------------------------

Callsite::IdType
Callsite::getId() const
{
    return mIdentifier;
}


const String&
Callsite::getFilename() const
{
    return mFilename;
}


uint32_t
Callsite::getLine() const
{
    return mLine;
}


const Region&
Callsite::getCallee() const
{
    return mCallee;
}


//--- Related functions -----------------------------------------------------

namespace pearl
{
//--- Stream I/O operators ---------------------------------

ostream&
operator<<(ostream&        stream,
           const Callsite& item)
{
    // Special case: undefined callsite
    if (Callsite::UNDEFINED == item) {
        return stream << "<undefined>";
    }

    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 13);

    // Print data
    stream << "Callsite {" << iendl(indent)
           << "  id       = " << item.getId() << iendl(indent)
           << "  filename = " << item.getFilename() << iendl(indent)
           << "  line     = " << item.getLine() << iendl(indent)
           << "  callee   = " << item.getCallee() << iendl(indent)
           << "}";

    // Reset indentation
    return setIndent(stream, indent);
}


//--- Comparison operators ---------------------------------

bool
operator==(const Callsite& lhs,
           const Callsite& rhs)
{
    return ((lhs.getId() == rhs.getId())
            && (lhs.getFilename() == rhs.getFilename())
            && (lhs.getLine() == rhs.getLine())
            && (lhs.getCallee() == rhs.getCallee()));
}


bool
operator!=(const Callsite& lhs,
           const Callsite& rhs)
{
    return !(lhs == rhs);
}
}   // namespace pearl
