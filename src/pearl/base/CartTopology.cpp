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
 *  @brief   Implementation of the class CartTopology.
 *
 *  This file provides the implementation of the class CartTopology and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <pearl/CartTopology.h>

#include <iostream>

#include <pearl/CartDimension.h>
#include <pearl/Communicator.h>
#include <pearl/Error.h>
#include <pearl/String.h>
#include <pearl/Utils.h>

#include "pearl_iomanip.h"

using namespace std;
using namespace pearl;


// --- Constructors & destructor --------------------------------------------

CartTopology::CartTopology(const IdType              id,
                           const String&             name,
                           const Communicator&       communicator,
                           const DimensionContainer& dimensions)
    : mCommunicator(&communicator),
      mDimensions(dimensions),
      mName(name),
      mIdentifier(id)
{
}


CartTopology::~CartTopology()
{
}


// --- Access definition data -----------------------------------------------

CartTopology::IdType
CartTopology::getId() const
{
    return mIdentifier;
}


const String&
CartTopology::getName() const
{
    return mName;
}


const Communicator&
CartTopology::getCommunicator() const
{
    return *mCommunicator;
}


uint8_t
CartTopology::numDimensions() const
{
    return mDimensions.size();
}


const CartDimension&
CartTopology::getDimension(const uint8_t index) const
{
    if (index >= mDimensions.size())
    {
        throw RuntimeError("pearl::CartTopology::"
                           "getDimension(uint8_t) -- "
                           "invalid index: " + toStdString(index));
    }

    return *mDimensions[index];
}


const CartCoordList
CartTopology::getRankCoordinates(const uint32_t rank) const
{
    if (rank >= mCommunicator->getSize())
    {
        throw RuntimeError("pearl::CartTopology::"
                           "getRankCoordinates(uint32_t) -- "
                           "invalid rank: " + toStdString(rank));
    }

    CoordinateContainer::const_iterator it = mCoordinates.find(rank);
    if (it == mCoordinates.end())
    {
        return CartCoordList();
    }

    return it->second;
}


// --- Related functions ----------------------------------------------------

namespace pearl
{
ostream&
operator<<(ostream&            stream,
           const CartTopology& item)
{
    // Adjust indentation
    int indent = getIndent(stream);
    setIndent(stream, indent + 11);

    // Print dimension data
    stream << "CartTopology {" << iendl(indent)
           << "  id     = " << item.getId() << iendl(indent)
           << "  name   = " << item.getName() << iendl(indent)
           << "  comm   = " << item.getCommunicator() << iendl(indent)
           << "  dims   = [";
    setIndent(stream, indent + 4);
    const uint8_t numDimensions = item.numDimensions();
    for (size_t i = 0; i < numDimensions; ++i)
    {
        stream << iendl(indent + 4)
               << item.getDimension(i);
    }
    stream << iendl(indent)
           << "  ]" << iendl(indent)
           << "  coords = [" << iendl(indent);
    const uint32_t commSize = item.getCommunicator().getSize();
    for (uint32_t rank = 0; rank < commSize; ++rank)
    {
        const CartCoordList coordList = item.getRankCoordinates(rank);

        CartCoordList::const_iterator it = coordList.begin();
        while (it != coordList.end())
        {
            stream << "    " << rank << " -> (";
            CartCoordinate::const_iterator cit = it->begin();
            while (cit != it->end())
            {
                stream << *cit;
                ++cit;
                if (cit != it->end())
                {
                    stream << ", ";
                }
            }
            stream << ")" << iendl(indent);

            ++it;
        }
    }
    stream << "  ]" << iendl(indent)
           << "}";

    // Reset indentation
    return setIndent(stream, indent);
}
}    // namespace pearl
