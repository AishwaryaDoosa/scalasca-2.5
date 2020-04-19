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


#include <config.h>
#include <pearl/Callpath.h>

#include <algorithm>
#include <cstddef>
#include <iostream>

#include <pearl/Callsite.h>
#include <pearl/Region.h>
#include <pearl/String.h>

#include "CallpathMatcher.h"
#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


/*-------------------------------------------------------------------------*/
/**
 *  @file    Callpath.cpp
 *  @ingroup PEARL_base
 *  @brief   Implementation of the class Callpath.
 *
 *  This file provides the implementation of the class Callpath and related
 *  functions.
 **/
/*-------------------------------------------------------------------------*/


//--- Constructors & destructor ---------------------------------------------

Callpath::Callpath(const IdType    id,
                   const Region&   region,
                   const Callsite& callsite,
                   Callpath* const parent)
    : mIdentifier(id),
      mRegion(region),
      mCallsite(callsite),
      mParent(parent)
{
    if (parent) {
        parent->mChildren.push_back(this);
    }
}


//--- Access definition data ------------------------------------------------

Callpath::IdType
Callpath::getId() const
{
    return mIdentifier;
}


const Region&
Callpath::getRegion() const
{
    return mRegion;
}


const Callsite&
Callpath::getCallsite() const
{
    return mCallsite;
}


Callpath*
Callpath::getParent() const
{
    return mParent;
}


uint32_t
Callpath::numChildren() const
{
    return mChildren.size();
}


const Callpath&
Callpath::getChild(const uint32_t index) const
{
    return *(mChildren.at(index));
}


//--- Private methods -------------------------------------------------------

/// @brief Stream output helper function.
///
/// Prints the referenced region name and the callpath's identifier to the
/// given @a stream before recursively processing all child callpaths. This
/// member function is triggered by CallTree::print() to print a nicely
/// formatted calltree.
///
/// @param  stream  Output stream
/// @param  prefix  Prefix printed before the callpath data
/// @param  isLast  True if the callpath is the last entry in the child
///                 list of its parent, false otherwise.
///
/// @return Output stream
///
ostream&
Callpath::print(ostream&      stream,
                const string& prefix,
                const bool    isLast) const
{
    // Print current callpath
    stream << prefix
           << " + " << mRegion.getDisplayName().getString()
           << "  <" << mIdentifier << ">"
           << endl;

    // Process children
    size_t count = mChildren.size();
    for (size_t index = 0; index < count; ++index) {
        mChildren[index]->print(stream,
                                prefix + (isLast ? "   " : " | "),
                                (count == (index + 1)));
    }
    return stream;
}


/// @brief Search a particular child callpath.
///
/// Returns a pointer to the child callpath referencing the given @a region
/// and @a callsite, or NULL if no such child exists.
///
/// @param  region    Called region
/// @param  callsite  Specific callsite (Callsite::UNDEFINED if not
///                   applicable)
///
/// @return Pointer to child callpath if existent, NULL otherwise
///
Callpath*
Callpath::findChild(const Region&   region,
                    const Callsite& callsite) const
{
    ChildContainer::const_iterator it = find_if(mChildren.begin(),
                                                mChildren.end(),
                                                CallpathMatcher(region,
                                                                callsite));
    if (mChildren.end() == it) {
        return NULL;
    }
    return *it;
}


//--- Related functions -----------------------------------------------------

namespace pearl
{
//--- Stream I/O operators ---------------------------------

ostream&
operator<<(ostream&        stream,
           const Callpath& item)
{
    // Special case: undefined callpath
    if (Callpath::UNDEFINED == item) {
        return stream << "<undefined>";
    }

    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 13);

    // Print data
    stream << "Callpath {" << iendl(indent)
           << "  id       = " << item.getId() << iendl(indent)
           << "  region   = " << item.getRegion() << iendl(indent)
           << "  callsite = " << item.getCallsite() << iendl(indent)
           << "  parent   = ";
    Callpath* parent = item.getParent();
    if (NULL == parent) {
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
operator==(const Callpath& lhs,
           const Callpath& rhs)
{
    return ((lhs.getId() == rhs.getId())
            && (lhs.getRegion() == rhs.getRegion())
            && (lhs.getCallsite() == rhs.getCallsite())
            && (lhs.getParent() == rhs.getParent()));
}


bool
operator!=(const Callpath& lhs,
           const Callpath& rhs)
{
    return !(lhs == rhs);
}
}   // namespace pearl
