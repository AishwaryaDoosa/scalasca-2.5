/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2017-2018                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


#include <config.h>

#include <pearl/CommLocationSet.h>

#include <algorithm>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include <pearl/Error.h>
#include <pearl/Location.h>
#include <pearl/ScopedPtr.h>
#include <pearl/String.h>
#include <pearl/SystemNode.h>
#include <pearl/Utils.h>

#include "DefinitionContainer.h"
#include "Functors.h"
#include "Process.h"

using namespace std;
using namespace testing;
using namespace pearl;
using namespace pearl::detail;


// --- Helper ---------------------------------------------------------------

namespace
{
// Test fixture for communication location set tests
class CommLocationSetT
    : public Test
{
    public:
        virtual
        ~CommLocationSetT()
        {
            for_each(mLocations.begin(), mLocations.end(),
                     delete_ptr< Location >());
        }

        void
        createLocations(const Location::IdType maxId);


    protected:
        vector< const Location* >     mLocations;
        ScopedPtr< Process >          mLocationGroup;
        ScopedPtr< SystemNode >       mSystemNode;
        DefinitionContainer< String > mStrings;
};
}    // unnamed namespace


// --- CommLocationSet tests ------------------------------------------------


TEST_F(CommLocationSetT, getId_undefined_returnsMaxId)
{
    EXPECT_EQ(numeric_limits< CommLocationSet::IdType >::max(),
              CommLocationSet::UNDEFINED.getId());
}


TEST_F(CommLocationSetT, getId_item_returnsIdentifier)
{
    CommLocationSet item0(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(0, item0.getId());

    CommLocationSet item1(1, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(1, item1.getId());

    CommLocationSet item42(42, String::UNDEFINED, Paradigm::UNKNOWN,
                           mLocations);
    EXPECT_EQ(42, item42.getId());
}


TEST_F(CommLocationSetT, getName_undefined_returnsUndefined)
{
    EXPECT_EQ(String::UNDEFINED, CommLocationSet::UNDEFINED.getName());
}


TEST_F(CommLocationSetT, getName_item_returnsName)
{
    CommLocationSet item0(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(&String::UNDEFINED, &item0.getName());

    String          name1(0, "Foo");
    CommLocationSet item1(1, name1, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(&name1, &item1.getName());
}


TEST_F(CommLocationSetT, getType_undefined_returnsType)
{
    EXPECT_EQ(GroupingSet::COMM_LOCATION_SET,
              CommLocationSet::UNDEFINED.getType());
}


TEST_F(CommLocationSetT, getType_item_returnsType)
{
    CommLocationSet item(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(GroupingSet::COMM_LOCATION_SET, item.getType());
}


TEST_F(CommLocationSetT, numLocations_undefined_returnsZero)
{
    EXPECT_EQ(0, CommLocationSet::UNDEFINED.numLocations());
}


TEST_F(CommLocationSetT, numLocations_emptyItem_returnsZero)
{
    CommLocationSet item(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(0, item.numLocations());
}


TEST_F(CommLocationSetT, numLocations_item_returnsMemberCount)
{
    createLocations(5);

    CommLocationSet item(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(5, item.numLocations());
}


TEST_F(CommLocationSetT, getLocation_itemAndValidRank_returnsMember)
{
    createLocations(3);

    CommLocationSet item(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(mLocations[0], &item.getLocation(0));
    EXPECT_EQ(mLocations[1], &item.getLocation(1));
    EXPECT_EQ(mLocations[2], &item.getLocation(2));
}


TEST_F(CommLocationSetT, getLocation_itemAndInvalidRank_throws)
{
    createLocations(3);

    CommLocationSet item(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_THROW(item.getLocation(3), pearl::RuntimeError);
}


TEST_F(CommLocationSetT, getParadigm_undefined_returnsUnknown)
{
    EXPECT_EQ(Paradigm::UNKNOWN, CommLocationSet::UNDEFINED.getParadigm());
}


TEST_F(CommLocationSetT, getParadigm_item_returnsParadigm)
{
    CommLocationSet item0(0, String::UNDEFINED, Paradigm::UNKNOWN, mLocations);
    EXPECT_EQ(Paradigm::UNKNOWN, item0.getParadigm());

    CommLocationSet item1(1, String::UNDEFINED, Paradigm::MPI, mLocations);
    EXPECT_EQ(Paradigm::MPI, item1.getParadigm());
}


TEST_F(CommLocationSetT, operatorLeftShift_undefined_printsUndefined)
{
    string expected("<undefined>");

    ostringstream result;
    result << CommLocationSet::UNDEFINED;
    EXPECT_EQ(expected, result.str());
}


TEST_F(CommLocationSetT, operatorLeftShift_item_printsData)
{
    createLocations(2);

    String          name(24, "Foo");
    CommLocationSet item(42, name, Paradigm::MPI, mLocations);

    string expected("CommLocationSet {\n"
                    "  id       = 42\n"
                    "  name     = String(24, \"Foo\")\n"
                    "  paradigm = mpi\n"
                    "  size     = 2\n"
                    "  members  = [\n"
                    "    Location {\n"
                    "      id        = 0\n"
                    "      name      = String(0, \"Thread 0\")\n"
                    "      type      = CPU thread\n"
                    "      numEvents = 0\n"
                    "      parent    = Process {\n"
                    "                    id     = 0\n"
                    "                    name   = <undefined>\n"
                    "                    rank   = -1\n"
                    "                    parent = SystemNode(0)\n"
                    "                  }\n"
                    "    }\n"
                    "    Location {\n"
                    "      id        = 1\n"
                    "      name      = String(1, \"Thread 1\")\n"
                    "      type      = CPU thread\n"
                    "      numEvents = 0\n"
                    "      parent    = Process {\n"
                    "                    id     = 0\n"
                    "                    name   = <undefined>\n"
                    "                    rank   = -1\n"
                    "                    parent = SystemNode(0)\n"
                    "                  }\n"
                    "    }\n"
                    "  ]\n"
                    "}");

    ostringstream result;
    result << item;
    EXPECT_EQ(expected, result.str());
}


// --- Helper ---------------------------------------------------------------

void
CommLocationSetT::createLocations(const Location::IdType maxId)
{
    // Create dummy SystemNode (used as parent)
    mSystemNode.reset(new SystemNode(0,
                                     String::UNDEFINED,
                                     String::UNDEFINED,
                                     0));

    // Create dummy LocationGroup (used as parent)
    mLocationGroup.reset(new Process(0,
                                     String::UNDEFINED,
                                     mSystemNode.get()));

    // Create locations
    for (Location::IdType id = 0; id < maxId; ++id)
    {
        ScopedPtr< String > name(new String(id, "Thread " + toStdString(id)));
        mStrings.addItem(name.get());
        name.release();

        ScopedPtr< Location > location(new Location(id,
                                                    *mStrings.getItemById(id),
                                                    Location::TYPE_CPU_THREAD,
                                                    0,
                                                    mLocationGroup.get()));
        mLocations.push_back(location.get());
        location.release();
    }
}
