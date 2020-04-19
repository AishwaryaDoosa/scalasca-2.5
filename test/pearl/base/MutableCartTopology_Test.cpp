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


#include <config.h>

#include "MutableCartTopology.h"

#include <algorithm>
#include <vector>

#include <gtest/gtest.h>

#include <pearl/CartDimension.h>
#include <pearl/Communicator.h>
#include <pearl/CommLocationSet.h>
#include <pearl/CommSet.h>
#include <pearl/Error.h>
#include <pearl/Location.h>
#include <pearl/ScopedPtr.h>
#include <pearl/String.h>
#include <pearl/SystemNode.h>

#include "Functors.h"
#include "Process.h"

using namespace std;
using namespace testing;
using namespace pearl;
using namespace pearl::detail;


// --- Helper ---------------------------------------------------------------

namespace
{
// Test fixture for mutable Cartesian topology tests
class MutableCartTopologyT
    : public Test
{
    public:
        ~MutableCartTopologyT();

        void
        createCommunicator(uint32_t size);


    protected:
        CartTopology::DimensionContainer mDimensions;
        ScopedPtr< Communicator >        mCommunicator;
        ScopedPtr< CommSet >             mCommSet;
        ScopedPtr< CommLocationSet >     mCommLocationSet;
        vector< const Location* >        mLocations;
        ScopedPtr< LocationGroup >       mLocationGroup;
        ScopedPtr< SystemNode >          mSystemNode;
};
}    // unnamed namespace


// --- MutableCartTopology tests --------------------------------------------

TEST_F(MutableCartTopologyT, setCommunicator_item_updatesCommunicator)
{
    MutableCartTopology item(0, String::UNDEFINED, Communicator::UNDEFINED,
                             mDimensions);
    EXPECT_EQ(&Communicator::UNDEFINED, &item.getCommunicator());

    createCommunicator(8);
    item.setCommunicator(*mCommunicator);
    EXPECT_EQ(mCommunicator.get(), &item.getCommunicator());

    createCommunicator(16);
    item.setCommunicator(*mCommunicator);
    EXPECT_EQ(mCommunicator.get(), &item.getCommunicator());
}


TEST_F(MutableCartTopologyT, addRankCoordinate_item_addsCoordinateToList)
{
    createCommunicator(8);

    mDimensions.reserve(2);
    CartDimension dim0(0, String::UNDEFINED, 2, CartDimension::PERIODIC);
    mDimensions.push_back(&dim0);
    CartDimension dim1(1, String::UNDEFINED, 2, CartDimension::PERIODIC);
    mDimensions.push_back(&dim1);

    MutableCartTopology item(0, String::UNDEFINED, *mCommunicator, mDimensions);
    ASSERT_EQ(0, item.getRankCoordinates(0).size());

    CartCoordinate coordinate1;
    coordinate1.reserve(2);
    coordinate1.push_back(0);
    coordinate1.push_back(0);
    item.addRankCoordinate(0, coordinate1);
    const CartCoordList coords1 = item.getRankCoordinates(0);
    ASSERT_EQ(1, coords1.size());
    EXPECT_EQ(coordinate1, coords1[0]);

    CartCoordinate coordinate2;
    coordinate2.reserve(2);
    coordinate2.push_back(0);
    coordinate2.push_back(0);
    item.addRankCoordinate(0, coordinate2);
    const CartCoordList coords2 = item.getRankCoordinates(0);
    ASSERT_EQ(2, coords2.size());
    EXPECT_EQ(coordinate1, coords2[0]);
    EXPECT_EQ(coordinate2, coords2[1]);
}


TEST_F(MutableCartTopologyT, addRankCoordinate_itemAndInvalidRank_throws)
{
    createCommunicator(8);

    MutableCartTopology item(0, String::UNDEFINED, *mCommunicator, mDimensions);

    CartCoordinate coordinate;
    EXPECT_THROW(item.addRankCoordinate(8, coordinate), RuntimeError);
}


TEST_F(MutableCartTopologyT, addRankCoordinate_itemAndInvalidDimensions_throws)
{
    createCommunicator(8);

    mDimensions.reserve(1);
    CartDimension dim(0, String::UNDEFINED, 2, CartDimension::PERIODIC);
    mDimensions.push_back(&dim);

    MutableCartTopology item(0, String::UNDEFINED, *mCommunicator, mDimensions);

    CartCoordinate coordinate(2, 0);
    EXPECT_THROW(item.addRankCoordinate(0, coordinate), RuntimeError);
}


TEST_F(MutableCartTopologyT, addRankCoordinate_itemAndInvalidCoordinate_throws)
{
    createCommunicator(8);

    mDimensions.reserve(2);
    CartDimension dim0(0, String::UNDEFINED, 2, CartDimension::PERIODIC);
    mDimensions.push_back(&dim0);
    CartDimension dim1(1, String::UNDEFINED, 2, CartDimension::PERIODIC);
    mDimensions.push_back(&dim1);

    MutableCartTopology item(0, String::UNDEFINED, *mCommunicator, mDimensions);

    CartCoordinate coordinate;
    coordinate.reserve(2);
    coordinate.push_back(0);
    coordinate.push_back(2);
    EXPECT_THROW(item.addRankCoordinate(0, coordinate), RuntimeError);
}


TEST_F(MutableCartTopologyT, operatorLeftShift_item_printsData)
{
    String        name0(24, "Periodic");
    CartDimension dim0(42, name0, 2, CartDimension::PERIODIC);
    mDimensions.push_back(&dim0);

    String        name1(25, "Non-periodic");
    CartDimension dim1(43, name1, 3, CartDimension::NON_PERIODIC);
    mDimensions.push_back(&dim1);

    createCommunicator(2);
    String              name2(4, "My topology");
    MutableCartTopology item(17, name2, *mCommunicator, mDimensions);

    CartCoordinate coordinate;
    coordinate.reserve(2);
    coordinate.push_back(0);
    coordinate.push_back(2);
    item.addRankCoordinate(0, coordinate);

    coordinate.clear();
    coordinate.push_back(1);
    coordinate.push_back(2);
    item.addRankCoordinate(0, coordinate);

    coordinate.clear();
    coordinate.push_back(0);
    coordinate.push_back(1);
    item.addRankCoordinate(1, coordinate);

    string expected("CartTopology {\n"
                    "  id     = 17\n"
                    "  name   = String(4, \"My topology\")\n"
                    "  comm   = Communicator {\n"
                    "             id       = 0\n"
                    "             name     = <undefined>\n"
                    "             parent   = <none>\n"
                    "             comm set = CommSet {\n"
                    "                          id         = 0\n"
                    "                          name       = <undefined>\n"
                    "                          properties = 0 (NONE)\n"
                    "                          paradigm   = user\n"
                    "                          size       = 2\n"
                    "                          members    = [\n"
                    "                            Location {\n"
                    "                              id        = 0\n"
                    "                              name      = <undefined>\n"
                    "                              type      = CPU thread\n"
                    "                              numEvents = 0\n"
                    "                              parent    = Process {\n"
                    "                                            id     = 0\n"
                    "                                            name   = <undefined>\n"
                    "                                            rank   = -1\n"
                    "                                            parent = SystemNode(0)\n"
                    "                                          }\n"
                    "                            }\n"
                    "                            Location {\n"
                    "                              id        = 1\n"
                    "                              name      = <undefined>\n"
                    "                              type      = CPU thread\n"
                    "                              numEvents = 0\n"
                    "                              parent    = Process {\n"
                    "                                            id     = 0\n"
                    "                                            name   = <undefined>\n"
                    "                                            rank   = -1\n"
                    "                                            parent = SystemNode(0)\n"
                    "                                          }\n"
                    "                            }\n"
                    "                          ]\n"
                    "                        }\n"
                    "           }\n"
                    "  dims   = [\n"
                    "    CartDimension {\n"
                    "      id       = 42\n"
                    "      name     = String(24, \"Periodic\")\n"
                    "      size     = 2\n"
                    "      periodic = true\n"
                    "    }\n"
                    "    CartDimension {\n"
                    "      id       = 43\n"
                    "      name     = String(25, \"Non-periodic\")\n"
                    "      size     = 3\n"
                    "      periodic = false\n"
                    "    }\n"
                    "  ]\n"
                    "  coords = [\n"
                    "    0 -> (0, 2)\n"
                    "    0 -> (1, 2)\n"
                    "    1 -> (0, 1)\n"
                    "  ]\n"
                    "}");

    ostringstream result;
    result << item;
    EXPECT_EQ(expected, result.str());
}


// --- Helper ---------------------------------------------------------------

MutableCartTopologyT::~MutableCartTopologyT()
{
    for_each(mLocations.begin(), mLocations.end(), delete_ptr< Location >());
}


/// @todo [C++11] Pass `nullptr` to comm constructor for `parent`
void
MutableCartTopologyT::createCommunicator(const uint32_t size)
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
    vector< uint32_t > ranks;
    ranks.reserve(size);
    mLocations.reserve(size);
    for (uint32_t rank = 0; rank < size; ++rank)
    {
        ranks.push_back(rank);

        ScopedPtr< Location > location(new Location(rank,
                                                    String::UNDEFINED,
                                                    Location::TYPE_CPU_THREAD,
                                                    0,
                                                    mLocationGroup.get()));
        mLocations.push_back(location.get());
        location.release();
    }

    // Create CommLocationSet
    mCommLocationSet.reset(new CommLocationSet(0,
                                               String::UNDEFINED,
                                               Paradigm::USER,
                                               mLocations));

    // Create CommSet
    mCommSet.reset(new CommSet(0,
                               String::UNDEFINED,
                               GroupingSet::PROPERTY_NONE,
                               *mCommLocationSet,
                               ranks));

    // Create Communicator
    mCommunicator.reset(new Communicator(0,
                                         String::UNDEFINED, *mCommSet, 0));
}
