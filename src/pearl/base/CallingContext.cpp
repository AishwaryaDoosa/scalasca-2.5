/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2015-2017                                                **
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
 *  @brief   Implementation of the class CallingContext.
 *
 *  This file provides the implementation of the class CallingContext and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <pearl/CallingContext.h>

#include <iostream>

#include <pearl/Region.h>
#include <pearl/SourceLocation.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


//--- Constructors & destructor ---------------------------------------------

CallingContext::CallingContext(const IdType          id,
                               const Region&         region,
                               const SourceLocation& scl,
                               CallingContext* const parent)
    : mRegion(region),
      mSourceLocation(scl),
      mParent(parent),
      mIdentifier(id)
{
    if (parent) {
        parent->mChildren.push_back(this);
    }
}


//--- Access definition data ------------------------------------------------

CallingContext::IdType
CallingContext::getId() const
{
    return mIdentifier;
}


const Region&
CallingContext::getRegion() const
{
    return mRegion;
}


const SourceLocation&
CallingContext::getSourceLocation() const
{
    return mSourceLocation;
}


CallingContext*
CallingContext::getParent() const
{
    return mParent;
}


uint32_t
CallingContext::numChildren() const
{
    return mChildren.size();
}


const CallingContext&
CallingContext::getChild(const uint32_t index) const
{
    return *(mChildren.at(index));
}


//--- Related functions -----------------------------------------------------

namespace pearl
{
//--- Stream I/O operators ---------------------------------

ostream&
operator<<(ostream&              stream,
           const CallingContext& item)
{
    // Special case: undefined calling context
    if (item == CallingContext::UNDEFINED) {
        return stream << "<undefined>";
    }

    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 11);

    // Print data
    stream << "CallingContext {" << iendl(indent)
           << "  id     = " << item.getId() << iendl(indent)
           << "  region = " << item.getRegion() << iendl(indent)
           << "  scl    = " << item.getSourceLocation() << iendl(indent)
           << "  parent = ";
    CallingContext* parent = item.getParent();
    if (parent == 0) {
        stream << "<undefined>";
    } else {
        stream << parent->getId();
    }
    stream << iendl(indent)
           << "}";

    // Reset indentation
    return setIndent(stream, indent);
}


//--- Comparison operators ---------------------------------

bool
operator==(const CallingContext& lhs,
           const CallingContext& rhs)
{
    // Short-cut: self-comparison
    if (&lhs == &rhs) {
        return true;
    }

    return ((lhs.getId() == rhs.getId())
            && (lhs.getRegion() == rhs.getRegion())
            && (lhs.getSourceLocation() == rhs.getSourceLocation())
            && (lhs.getParent() == rhs.getParent()));
}


bool
operator!=(const CallingContext& lhs,
           const CallingContext& rhs)
{
    return !(lhs == rhs);
}
}   // namespace pearl
