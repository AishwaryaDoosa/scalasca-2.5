/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2017                                                     **
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
 *  @brief   Implementation of the class GroupingSet.
 *
 *  This file provides the implementation of the class GroupingSet and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <pearl/GroupingSet.h>

#include <pearl/Utils.h>

using namespace std;
using namespace pearl;


// --- Constructors & destructor --------------------------------------------

GroupingSet::GroupingSet(const IdType  id,
                         const String& name)
    : mName(name),
      mIdentifier(id)
{
}


GroupingSet::~GroupingSet()
{
}


// --- Query functions ------------------------------------------------------

GroupingSet::IdType
GroupingSet::getId() const
{
    return mIdentifier;
}


const String&
GroupingSet::getName() const
{
    return mName;
}


// --- Related functions ----------------------------------------------------

namespace pearl
{
ostream&
operator<<(ostream&           stream,
           const GroupingSet& item)
{
    return item.output(stream);
}


string
toStdString(const GroupingSet::SetType type)
{
    string result;
    switch (type)
    {
        #define TYPE_TO_STRING(type, stringRep) \
            case GroupingSet::type:             \
                result = stringRep;             \
                break;

        TYPE_TO_STRING(LOCATION_SET, "Location set")
        TYPE_TO_STRING(REGION_SET, "Region set")
        TYPE_TO_STRING(METRIC_SET, "Metric set")
        TYPE_TO_STRING(COMM_LOCATION_SET, "Comm location set")
        TYPE_TO_STRING(COMM_SET, "Comm set")

        #undef TYPE_TO_STRING

        default:
            result = "UNKNOWN";
            break;
    }

    return result;
}


string
toStdString(const GroupingSet::Properties& properties)
{
    bool isFirst = true;

    #define PROPERTY_TO_STRING(property, stringRep) \
        if (properties.test(GroupingSet::property)) \
        {                                           \
            result += isFirst ? " (" : ",";         \
            result += stringRep;                    \
            isFirst = false;                        \
        }


    string result(toStdString(properties.getValue()));

    PROPERTY_TO_STRING(PROPERTY_NONE, "NONE")
    PROPERTY_TO_STRING(PROPERTY_SELF, "self")
    PROPERTY_TO_STRING(PROPERTY_WORLD, "world")
    PROPERTY_TO_STRING(PROPERTY_GLOBAL_RANKS, "global ranks")

    result += ")";

    #undef PROPERTY_TO_STRING

    return result;
}
}    // namespace pearl
