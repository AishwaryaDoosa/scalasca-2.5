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
#include "pearl_iomanip.h"

#include <iostream>
#include <string>

using namespace std;
using namespace pearl;


/*-------------------------------------------------------------------------*/
/**
 *  @file    pearl_iomanip.cpp
 *  @ingroup PEARL_base
 *  @brief   Implementation of PEARL-specific stream I/O functionality.
 *
 *  This header file provides the implementation of some PEARL-specific
 *  stream I/O manipulators and format flag handlers.
 **/
/*-------------------------------------------------------------------------*/


//--- Local variables -------------------------------------------------------

namespace
{
    /// Format flag index of the indentation level
    static const int indentIndex = ios_base::xalloc();
}   // unnamed namespace


//--- Constructors & destructor ---------------------------------------------

iendl::iendl(const int indent)
    : mIndent(indent)
{
}


//--- Related functions -----------------------------------------------------

namespace pearl
{
ostream&
operator<<(ostream&     stream,
           const iendl& item)
{
    string indent(item.mIndent, ' ');

    return stream << endl
                  << indent;
}


int
getIndent(std::ostream& stream)
{
    return stream.iword(indentIndex);
}


ostream&
setIndent(std::ostream& stream,
          int           level)
{
    stream.iword(indentIndex) = level;

    return stream;
}
}   // namespace pearl
