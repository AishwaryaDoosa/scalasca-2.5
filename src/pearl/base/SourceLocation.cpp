/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2015                                                     **
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
 *  @brief   Implementation of the class SourceLocation.
 *
 *  This file provides the implementation of the class SourceLocation and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>
#include <pearl/SourceLocation.h>

#include <iostream>

#include <pearl/String.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


//--- Constructors & destructor ---------------------------------------------

SourceLocation::SourceLocation(const IdType   id,
                               const String&  filename,
                               const uint32_t line)
    : mFilename(filename),
      mIdentifier(id),
      mLine(line)
{
}


//--- Access definition data ------------------------------------------------

SourceLocation::IdType
SourceLocation::getId() const
{
    return mIdentifier;
}


const String&
SourceLocation::getFilename() const
{
    return mFilename;
}


uint32_t
SourceLocation::getLine() const
{
    return mLine;
}


//--- Related functions -----------------------------------------------------

namespace pearl
{
//--- Stream I/O operators ---------------------------------

ostream&
operator<<(ostream&              stream,
           const SourceLocation& item)
{
    // Special case: undefined source-code location
    if (item == SourceLocation::UNDEFINED) {
        return stream << "<undefined>";
    }

    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 13);

    // Print data
    stream << "SourceLocation {" << iendl(indent)
           << "  id       = " << item.getId() << iendl(indent)
           << "  filename = " << item.getFilename() << iendl(indent)
           << "  line     = " << item.getLine() << iendl(indent)
           << "}";

    // Reset indentation
    return setIndent(stream, indent);
}


//--- Comparison operators ---------------------------------

bool
operator==(const SourceLocation& lhs,
           const SourceLocation& rhs)
{
    // Short-cut: self-comparison
    if (&lhs == &rhs) {
        return true;
    }

    return ((lhs.getId() == rhs.getId())
            && (lhs.getFilename() == rhs.getFilename())
            && (lhs.getLine() == rhs.getLine()));
}


bool
operator!=(const SourceLocation& lhs,
           const SourceLocation& rhs)
{
    return !(lhs == rhs);
}
}   // namespace pearl
