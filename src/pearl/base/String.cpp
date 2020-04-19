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
#include <pearl/String.h>

#include <iostream>

using namespace std;
using namespace pearl;


/*-------------------------------------------------------------------------*/
/**
 *  @file    String.cpp
 *  @ingroup PEARL_base
 *  @brief   Implementation of the class String.
 *
 *  This file provides the implementation of the class String and related
 *  functions.
 **/
/*-------------------------------------------------------------------------*/


//--- Constructors & destructor ---------------------------------------------

String::String(const IdType  id,
               const string& str)
    : mIdentifier(id),
      mString(str)
{
}


//--- Access definition data ------------------------------------------------

String::IdType
String::getId() const
{
    return mIdentifier;
}


const string
String::getString() const
{
    return mString;
}


const char*
String::getCString() const
{
    return mString.c_str();
}


//--- Related functions -----------------------------------------------------

namespace pearl
{
//--- Stream I/O operators ---------------------------------

ostream&
operator<<(ostream&      stream,
           const String& item)
{
    // Special case: undefined string
    if (String::UNDEFINED == item) {
        return stream << "<undefined>";
    }

    // Print data
    return stream << "String("
                  << item.getId()
                  << ", \""
                  << item.getString()
                  << "\")";
}


//--- Comparison operators ---------------------------------

bool
operator==(const String& lhs,
           const String& rhs)
{
    return ((lhs.getId() == rhs.getId())
            && (lhs.getString() == rhs.getString()));
}


bool
operator!=(const String& lhs,
           const String& rhs)
{
    return !(lhs == rhs);
}
}   // namespace pearl
