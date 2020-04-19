/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2019                                                     **
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
 *  @brief   Implementation of the class CartDimension.
 *
 *  This file provides the implementation of the class CartDimension and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <pearl/CartDimension.h>

#include <iostream>

#include <pearl/String.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


// --- Constructors & destructor --------------------------------------------

CartDimension::CartDimension(const IdType      id,
                             const String&     name,
                             const uint32_t    size,
                             const Periodicity periodicity)
    : mName(name),
      mIdentifier(id),
      mSize(size),
      mPeriodicity(periodicity)
{
}


// --- Access definition data -----------------------------------------------


CartDimension::IdType
CartDimension::getId() const
{
    return mIdentifier;
}


const String&
CartDimension::getName() const
{
    return mName;
}


uint32_t
CartDimension::getSize() const
{
    return mSize;
}


bool
CartDimension::isPeriodic() const
{
    return mPeriodicity == CartDimension::PERIODIC;
}


// --- Related functions ----------------------------------------------------

namespace pearl
{
ostream&
operator<<(ostream&             stream,
           const CartDimension& item)
{
    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 13);

    // Print dimension data
    ios_base::fmtflags fmtflags = stream.setf(ios_base::boolalpha);
    stream << "CartDimension {" << iendl(indent)
           << "  id       = " << item.getId() << iendl(indent)
           << "  name     = " << item.getName() << iendl(indent)
           << "  size     = " << item.getSize() << iendl(indent)
           << "  periodic = " << item.isPeriodic() << iendl(indent)
           << "}";
    stream.flags(fmtflags);

    // Reset indentation
    return setIndent(stream, indent);
}
}    // namespace pearl
