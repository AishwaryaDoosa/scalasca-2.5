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

#include <pearl/CommSet.h>

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <pearl/CommLocationSet.h>
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
// Test fixture for communication set tests; always creates a communication
// location set with 16 locations
class CommSetT
    : public Test
{
    public:
        CommSetT();

        virtual
        ~CommSetT();

        void
        createOddRanks();

        void
        createReverseRanks();


    protected:
        vector< uint32_t >            mRanks;
        ScopedPtr< CommLocationSet >  mCommLocationSet;
        vector< const Location* >     mLocations;
        DefinitionContainer< String > mStrings;
        ScopedPtr< LocationGroup >    mLocationGroup;
        ScopedPtr< SystemNode >       mSystemNode;
};
}    // unnamed namespace


// --- CommSet tests --------------------------------------------------------

TEST_F(CommSetT, getId_undefined_returnsMaxId)
{
    EXPECT_EQ(numeric_limits< CommSet::IdType >::max(),
              CommSet::UNDEFINED.getId());
}


TEST_F(CommSetT, getId_item_returnsIdentifier)
{
    CommSet item0(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(0, item0.getId());

    CommSet item1(1, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(1, item1.getId());

    CommSet item42(42, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                   CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(42, item42.getId());
}


TEST_F(CommSetT, getName_undefined_returnsUndefined)
{
    EXPECT_EQ(String::UNDEFINED, CommSet::UNDEFINED.getName());
}


TEST_F(CommSetT, getName_item_returnsName)
{
    CommSet item0(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(&String::UNDEFINED, &item0.getName());

    String  name1(0, "Foo");
    CommSet item1(1, name1, GroupingSet::PROPERTY_NONE,
                  CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(&name1, &item1.getName());
}


TEST_F(CommSetT, getType_undefined_returnsType)
{
    EXPECT_EQ(GroupingSet::COMM_SET, CommSet::UNDEFINED.getType());
}


TEST_F(CommSetT, getType_item_returnsType)
{
    CommSet item(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                 CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(GroupingSet::COMM_SET, item.getType());
}


TEST_F(CommSetT, getProperties_undefined_returnsEmptyProperties)
{
    GroupingSet::Properties properties;
    EXPECT_EQ(properties, CommSet::UNDEFINED.getProperties());
}


TEST_F(CommSetT, getProperties_item_returnsProperties)
{
    GroupingSet::Properties properties;
    CommSet item0(0, String::UNDEFINED, properties, CommLocationSet::UNDEFINED,
                  mRanks);
    EXPECT_EQ(properties, item0.getProperties());

    properties |= GroupingSet::PROPERTY_SELF;
    CommSet item1(0, String::UNDEFINED, properties, CommLocationSet::UNDEFINED,
                  mRanks);
    EXPECT_EQ(properties, item1.getProperties());

    properties |= GroupingSet::PROPERTY_GLOBAL_RANKS;
    CommSet item2(0, String::UNDEFINED, properties, CommLocationSet::UNDEFINED,
                  mRanks);
    EXPECT_EQ(properties, item2.getProperties());
}


TEST_F(CommSetT, getCommLocationSet_undefined_returnsUndefined)
{
    EXPECT_EQ(&CommLocationSet::UNDEFINED,
              &CommSet::UNDEFINED.getCommLocationSet());
}


TEST_F(CommSetT, getCommLocationSet_item_returnsCommLocationSet)
{
    CommSet item0(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(&CommLocationSet::UNDEFINED, &item0.getCommLocationSet());

    CommSet item1(1, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  *mCommLocationSet, mRanks);
    EXPECT_EQ(mCommLocationSet.get(), &item1.getCommLocationSet());
}


TEST_F(CommSetT, getParadigm_undefined_returnsUnknown)
{
    EXPECT_EQ(Paradigm::UNKNOWN, CommSet::UNDEFINED.getParadigm());
}


TEST_F(CommSetT, getParadigm_item_returnsParadigmOfCommLocationSet)
{
    CommSet item0(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(Paradigm::UNKNOWN, item0.getParadigm());

    CommSet item1(1, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  *mCommLocationSet, mRanks);
    EXPECT_EQ(Paradigm::MPI, item1.getParadigm());
}


TEST_F(CommSetT, numRanks_undefined_returnsZero)
{
    EXPECT_EQ(0, CommSet::UNDEFINED.numRanks());
}


TEST_F(CommSetT, numRanks_emptySet_returnsZero)
{
    CommSet empty(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                  CommLocationSet::UNDEFINED, mRanks);
    EXPECT_EQ(0, empty.numRanks());
}


TEST_F(CommSetT, numRanks_worldSet_returnsWorldSize)
{
    CommSet world(0, String::UNDEFINED, GroupingSet::PROPERTY_WORLD,
                  *mCommLocationSet, mRanks);
    EXPECT_EQ(16, world.numRanks());
}


TEST_F(CommSetT, numRanks_oddSet_returnsHalfWorldSize)
{
    createOddRanks();
    CommSet odd(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                *mCommLocationSet, mRanks);
    EXPECT_EQ(8, odd.numRanks());
}


TEST_F(CommSetT, getLocalRank_worldSetAndValidRank_returnsGlobalRank)
{
    CommSet world(0, String::UNDEFINED, GroupingSet::PROPERTY_WORLD,
                  *mCommLocationSet, mRanks);

    EXPECT_EQ(0, world.getLocalRank(0));
    EXPECT_EQ(1, world.getLocalRank(1));
    EXPECT_EQ(7, world.getLocalRank(7));
    EXPECT_EQ(8, world.getLocalRank(8));
    EXPECT_EQ(14, world.getLocalRank(14));
    EXPECT_EQ(15, world.getLocalRank(15));
}


TEST_F(CommSetT, getLocalRank_worldSetAndInvalidRank_throws)
{
    CommSet world(0, String::UNDEFINED, GroupingSet::PROPERTY_WORLD,
                  *mCommLocationSet, mRanks);

    EXPECT_THROW(world.getLocalRank(16), pearl::RuntimeError);
}


TEST_F(CommSetT, getLocalRank_oddSetAndValidRank_returnsLocalRank)
{
    createOddRanks();
    CommSet odd(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                *mCommLocationSet, mRanks);

    EXPECT_EQ(0, odd.getLocalRank(1));
    EXPECT_EQ(1, odd.getLocalRank(3));
    EXPECT_EQ(6, odd.getLocalRank(13));
    EXPECT_EQ(7, odd.getLocalRank(15));
}


TEST_F(CommSetT, getLocalRank_oddSetAndInvalidRank_throws)
{
    createOddRanks();
    CommSet odd(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                *mCommLocationSet, mRanks);

    EXPECT_THROW(odd.getLocalRank(0), pearl::RuntimeError);
    EXPECT_THROW(odd.getLocalRank(8), pearl::RuntimeError);
    EXPECT_THROW(odd.getLocalRank(16), pearl::RuntimeError);
}


TEST_F(CommSetT, getLocalRank_reverseSetAndValidRank_returnsLocalRank)
{
    createReverseRanks();
    CommSet reverse(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                    *mCommLocationSet, mRanks);

    EXPECT_EQ(0, reverse.getLocalRank(15));
    EXPECT_EQ(1, reverse.getLocalRank(14));
    EXPECT_EQ(14, reverse.getLocalRank(1));
    EXPECT_EQ(15, reverse.getLocalRank(0));
}


TEST_F(CommSetT, getLocalRank_reverseSetAndInvalidRank_throws)
{
    createReverseRanks();
    CommSet reverse(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                    *mCommLocationSet, mRanks);

    EXPECT_THROW(reverse.getLocalRank(16), pearl::RuntimeError);
}


TEST_F(CommSetT, getGlobalRank_worldSetAndValidRank_returnsGlobalRank)
{
    CommSet world(0, String::UNDEFINED, GroupingSet::PROPERTY_WORLD,
                  *mCommLocationSet, mRanks);

    EXPECT_EQ(0, world.getGlobalRank(0));
    EXPECT_EQ(1, world.getGlobalRank(1));
    EXPECT_EQ(7, world.getGlobalRank(7));
    EXPECT_EQ(8, world.getGlobalRank(8));
    EXPECT_EQ(14, world.getGlobalRank(14));
    EXPECT_EQ(15, world.getGlobalRank(15));
}


TEST_F(CommSetT, getGlobalRank_worldSetAndInalidRank_throws)
{
    CommSet world(0, String::UNDEFINED, GroupingSet::PROPERTY_WORLD,
                  *mCommLocationSet, mRanks);

    EXPECT_THROW(world.getGlobalRank(16), pearl::RuntimeError);
}


TEST_F(CommSetT, getGlobalRank_oddSetAndValidRank_returnsGlobalRank)
{
    createOddRanks();
    CommSet odd(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                *mCommLocationSet, mRanks);

    EXPECT_EQ(1, odd.getGlobalRank(0));
    EXPECT_EQ(3, odd.getGlobalRank(1));
    EXPECT_EQ(13, odd.getGlobalRank(6));
    EXPECT_EQ(15, odd.getGlobalRank(7));
}


TEST_F(CommSetT, getGlobalRank_oddSetAndInvalidRank_throws)
{
    createOddRanks();
    CommSet odd(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                *mCommLocationSet, mRanks);

    EXPECT_THROW(odd.getGlobalRank(8), pearl::RuntimeError);
}


TEST_F(CommSetT, getGlobalRank_reverseSetAndValidRank_returnsGlobalRank)
{
    createReverseRanks();
    CommSet reverse(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                    *mCommLocationSet, mRanks);

    EXPECT_EQ(15, reverse.getGlobalRank(0));
    EXPECT_EQ(14, reverse.getGlobalRank(1));
    EXPECT_EQ(1, reverse.getGlobalRank(14));
    EXPECT_EQ(0, reverse.getGlobalRank(15));
}


TEST_F(CommSetT, getGlobalRank_reverseSetAndInvalidRank_throws)
{
    createReverseRanks();
    CommSet reverse(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                    *mCommLocationSet, mRanks);

    EXPECT_THROW(reverse.getGlobalRank(16), pearl::RuntimeError);
}


TEST_F(CommSetT, getLocation_worldSetAndValidRank_returnsLocation)
{
    CommSet world(0, String::UNDEFINED, GroupingSet::PROPERTY_WORLD,
                  *mCommLocationSet, mRanks);

    EXPECT_EQ(mLocations[0], &world.getLocation(0));
    EXPECT_EQ(mLocations[1], &world.getLocation(1));
    EXPECT_EQ(mLocations[14], &world.getLocation(14));
    EXPECT_EQ(mLocations[15], &world.getLocation(15));
}


TEST_F(CommSetT, getLocation_worldSetAndInvalidRank_throws)
{
    CommSet world(0, String::UNDEFINED, GroupingSet::PROPERTY_WORLD,
                  *mCommLocationSet, mRanks);

    EXPECT_THROW(world.getLocation(16), pearl::RuntimeError);
}


TEST_F(CommSetT, getLocation_oddSetAndValidRank_returnsLocation)
{
    createOddRanks();
    CommSet odd(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                *mCommLocationSet, mRanks);

    EXPECT_EQ(mLocations[1], &odd.getLocation(0));
    EXPECT_EQ(mLocations[3], &odd.getLocation(1));
    EXPECT_EQ(mLocations[13], &odd.getLocation(6));
    EXPECT_EQ(mLocations[15], &odd.getLocation(7));
}


TEST_F(CommSetT, getLocation_oddSetAndInvalidRank_throws)
{
    createOddRanks();
    CommSet odd(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                *mCommLocationSet, mRanks);

    EXPECT_THROW(odd.getLocation(8), pearl::RuntimeError);
}


TEST_F(CommSetT, getLocation_reverseSetAndValidRank_returnsLocation)
{
    createReverseRanks();
    CommSet reverse(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                    *mCommLocationSet, mRanks);

    EXPECT_EQ(mLocations[15], &reverse.getLocation(0));
    EXPECT_EQ(mLocations[14], &reverse.getLocation(1));
    EXPECT_EQ(mLocations[1], &reverse.getLocation(14));
    EXPECT_EQ(mLocations[0], &reverse.getLocation(15));
}


TEST_F(CommSetT, getLocation_reverseSetAndInvalidRank_throws)
{
    createReverseRanks();
    CommSet reverse(0, String::UNDEFINED, GroupingSet::PROPERTY_NONE,
                    *mCommLocationSet, mRanks);

    EXPECT_THROW(reverse.getLocation(16), pearl::RuntimeError);
}


TEST_F(CommSetT, operatorLeftShift_undefined_printsUndefined)
{
    string expected("<undefined>");

    ostringstream result;
    result << CommSet::UNDEFINED;
    EXPECT_EQ(expected, result.str());
}


TEST_F(CommSetT, operatorLeftShift_item_printsData)
{
    createOddRanks();

    String  name(24, "Foo");
    CommSet item(42, name, GroupingSet::PROPERTY_NONE, *mCommLocationSet,
                 mRanks);

    string expected("CommSet {\n"
                    "  id         = 42\n"
                    "  name       = String(24, \"Foo\")\n"
                    "  properties = 0 (NONE)\n"
                    "  paradigm   = mpi\n"
                    "  size       = 8\n"
                    "  members    = [\n"
                    "    Location {\n"
                    "      id        = 1\n"
                    "      name      = String(1, \"Rank 1\")\n"
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
                    "      id        = 3\n"
                    "      name      = String(3, \"Rank 3\")\n"
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
                    "      id        = 5\n"
                    "      name      = String(5, \"Rank 5\")\n"
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
                    "      id        = 7\n"
                    "      name      = String(7, \"Rank 7\")\n"
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
                    "      id        = 9\n"
                    "      name      = String(9, \"Rank 9\")\n"
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
                    "      id        = 11\n"
                    "      name      = String(11, \"Rank 11\")\n"
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
                    "      id        = 13\n"
                    "      name      = String(13, \"Rank 13\")\n"
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
                    "      id        = 15\n"
                    "      name      = String(15, \"Rank 15\")\n"
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

CommSetT::CommSetT()
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
    for (uint32_t id = 0; id < 16; ++id)
    {
        ScopedPtr< String > name(new String(id, "Rank " + toStdString(id)));
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

    // Create communication location set
    mCommLocationSet.reset(new CommLocationSet(0,
                                               String::UNDEFINED,
                                               Paradigm::MPI,
                                               mLocations));
}


CommSetT::~CommSetT()
{
    for_each(mLocations.begin(), mLocations.end(), delete_ptr< Location >());
}


void
CommSetT::createOddRanks()
{
    for (uint32_t rank = 0; rank < 16; ++rank)
    {
        if (rank % 2 != 0)
        {
            mRanks.push_back(rank);
        }
    }
}


void
CommSetT::createReverseRanks()
{
    for (uint32_t rank = 0; rank < 16; ++rank)
    {
        mRanks.push_back(15 - rank);
    }
}
