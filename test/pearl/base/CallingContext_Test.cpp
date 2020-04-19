/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2015-2016                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


#include <config.h>
#include <pearl/CallingContext.h>

#include <algorithm>
#include <limits>
#include <vector>

#include <gtest/gtest.h>

#include <pearl/Region.h>
#include <pearl/String.h>
#include <pearl/SourceLocation.h>

#include "Functors.h"

using namespace std;
using namespace testing;
using namespace pearl;
using namespace pearl::detail;


//--- Identifier constants --------------------------------------------------

enum StringIds
{
    STR_EMPTY,
    STR_SOURCE,
    STR_MAIN,
    STR_FOO,
    STR_BAR,
    STR_OOB
};


enum RegionIds
{
    REG_MAIN,
    REG_FOO,
    REG_BAR,
    REG_OOB
};


enum SclIds
{
    SCL_MAIN_0,
    SCL_FOO_0,
    SCL_FOO_1,
    SCL_BAR_0,
    SCL_OOB
};


enum ContextIds
{
    CC_M0,
    CC_M0_F0,
    CC_M0_F0_B0,
    CC_M0_F1,
    CC_M0_F1_B0,
    CC_OOB
};


//--- CallingContext tests --------------------------------------------------

// Test fixture used for calling context definition tests
//
// Build up a calling context tree for the following source code
// (where each "@" source line corresponds to one sample):
//
//      1       void bar( void )
//      2       {
//      3           @fma
//      4       }
//      5
//      6       void foo( void )
//      7       {
//      8           @call bar()
//      9           @call bar()
//     10       }
//     11
//     12       int main( void )
//     13       {
//     14           @call foo()
//     15       }
//
class CallingContextT
    : public Test
{
    public:
        virtual void
        SetUp()
        {
            mStrings.push_back(new String(STR_EMPTY, ""));
            mStrings.push_back(new String(STR_SOURCE, "test.c"));
            mStrings.push_back(new String(STR_MAIN, "main"));
            mStrings.push_back(new String(STR_FOO, "foo"));
            mStrings.push_back(new String(STR_BAR, "bar"));

            mRegions.push_back(new Region(REG_MAIN,
                                          *mStrings[STR_MAIN],
                                          *mStrings[STR_MAIN],
                                          *mStrings[STR_EMPTY],
                                          Region::ROLE_FUNCTION,
                                          Paradigm::SAMPLING,
                                          *mStrings[STR_SOURCE],
                                          13, 15));
            mRegions.push_back(new Region(REG_FOO,
                                          *mStrings[STR_FOO],
                                          *mStrings[STR_FOO],
                                          *mStrings[STR_EMPTY],
                                          Region::ROLE_FUNCTION,
                                          Paradigm::SAMPLING,
                                          *mStrings[STR_SOURCE],
                                          7, 10));
            mRegions.push_back(new Region(REG_BAR,
                                          *mStrings[STR_BAR],
                                          *mStrings[STR_BAR],
                                          *mStrings[STR_EMPTY],
                                          Region::ROLE_FUNCTION,
                                          Paradigm::SAMPLING,
                                          *mStrings[STR_SOURCE],
                                          2, 4));

            mSourceLocs.push_back(new SourceLocation(SCL_MAIN_0,
                                                     *mStrings[STR_SOURCE],
                                                     14));
            mSourceLocs.push_back(new SourceLocation(SCL_FOO_0,
                                                     *mStrings[STR_SOURCE],
                                                     8));
            mSourceLocs.push_back(new SourceLocation(SCL_FOO_1,
                                                     *mStrings[STR_SOURCE],
                                                     9));
            mSourceLocs.push_back(new SourceLocation(SCL_BAR_0,
                                                     *mStrings[STR_SOURCE],
                                                     3));

            mContexts.push_back(new CallingContext(CC_M0,
                                                   *mRegions[REG_MAIN],
                                                   *mSourceLocs[SCL_MAIN_0],
                                                   0));
            mContexts.push_back(new CallingContext(CC_M0_F0,
                                                   *mRegions[REG_FOO],
                                                   *mSourceLocs[SCL_FOO_0],
                                                   mContexts[CC_M0]));
            mContexts.push_back(new CallingContext(CC_M0_F0_B0,
                                                   *mRegions[REG_BAR],
                                                   *mSourceLocs[SCL_BAR_0],
                                                   mContexts[CC_M0_F0]));
            mContexts.push_back(new CallingContext(CC_M0_F1,
                                                   *mRegions[REG_FOO],
                                                   *mSourceLocs[SCL_FOO_1],
                                                   mContexts[CC_M0]));
            mContexts.push_back(new CallingContext(CC_M0_F1_B0,
                                                   *mRegions[REG_BAR],
                                                   *mSourceLocs[SCL_BAR_0],
                                                   mContexts[CC_M0_F1]));
        }

        virtual void
        TearDown()
        {
            for_each(mStrings.begin(), mStrings.end(), delete_ptr<String>());
            for_each(mRegions.begin(), mRegions.end(), delete_ptr<Region>());
            for_each(mSourceLocs.begin(), mSourceLocs.end(),
                     delete_ptr<SourceLocation>());
            for_each(mContexts.begin(), mContexts.end(),
                     delete_ptr<CallingContext>());
        }

    protected:
        vector<String*>         mStrings;
        vector<Region*>         mRegions;
        vector<SourceLocation*> mSourceLocs;
        vector<CallingContext*> mContexts;
};


TEST_F(CallingContextT, testStatics)
{
    EXPECT_EQ(numeric_limits<CallingContext::IdType>::max(),
              CallingContext::NO_ID);

    EXPECT_EQ(CallingContext::NO_ID, CallingContext::UNDEFINED.getId());
    EXPECT_EQ(Region::UNDEFINED, CallingContext::UNDEFINED.getRegion());
    EXPECT_EQ(SourceLocation::UNDEFINED,
              CallingContext::UNDEFINED.getSourceLocation());
    EXPECT_EQ(static_cast<CallingContext*>(0), CallingContext::UNDEFINED.getParent());
    EXPECT_EQ(0, CallingContext::UNDEFINED.numChildren());
    EXPECT_THROW(CallingContext::UNDEFINED.getChild(0), out_of_range);
}


TEST_F(CallingContextT, testNumChildren)
{
    EXPECT_EQ(2, mContexts[CC_M0]->numChildren());
    EXPECT_EQ(1, mContexts[CC_M0_F0]->numChildren());
    EXPECT_EQ(1, mContexts[CC_M0_F1]->numChildren());
    EXPECT_EQ(0, mContexts[CC_M0_F0_B0]->numChildren());
    EXPECT_EQ(0, mContexts[CC_M0_F1_B0]->numChildren());
}


TEST_F(CallingContextT, testStreamOutput)
{
    // Special case: undefined source-code location
    {
        string expected("<undefined>");

        ostringstream result;
        result << CallingContext::UNDEFINED;
        EXPECT_EQ(expected, result.str());
    }

    // "Regular" calling context node w/o parent (root node)
    {
        string expected("CallingContext {\n"
                        "  id     = 0\n"
                        "  region = Region {\n"
                        "             id          = 0\n"
                        "             name        = String(2, \"main\")\n"
                        "             description = String(0, \"\")\n"
                        "             role        = function\n"
                        "             paradigm    = sampling\n"
                        "             filename    = String(1, \"test.c\")\n"
                        "             startLine   = 13\n"
                        "             endLine     = 15\n"
                        "           }\n"
                        "  scl    = SourceLocation {\n"
                        "             id       = 0\n"
                        "             filename = String(1, \"test.c\")\n"
                        "             line     = 14\n"
                        "           }\n"
                        "  parent = <undefined>\n"
                        "}");

        ostringstream result;
        result << *mContexts[CC_M0];
        EXPECT_EQ(expected, result.str());
    }

    // "Regular" calling context node with parent
    {
        string expected("CallingContext {\n"
                        "  id     = 1\n"
                        "  region = Region {\n"
                        "             id          = 1\n"
                        "             name        = String(3, \"foo\")\n"
                        "             description = String(0, \"\")\n"
                        "             role        = function\n"
                        "             paradigm    = sampling\n"
                        "             filename    = String(1, \"test.c\")\n"
                        "             startLine   = 7\n"
                        "             endLine     = 10\n"
                        "           }\n"
                        "  scl    = SourceLocation {\n"
                        "             id       = 1\n"
                        "             filename = String(1, \"test.c\")\n"
                        "             line     = 8\n"
                        "           }\n"
                        "  parent = 0\n"
                        "}");

        ostringstream result;
        result << *mContexts[CC_M0_F0];
        EXPECT_EQ(expected, result.str());
    }
}


TEST_F(CallingContextT, testEquality)
{
    // Self-comparison
    EXPECT_TRUE(*mContexts[CC_M0_F0] == *mContexts[CC_M0_F0]);

    // Different object with identical values
    CallingContext cct_eq(CC_M0_F0, *mRegions[REG_FOO],
                          *mSourceLocs[SCL_FOO_0], mContexts[CC_M0]);
    EXPECT_TRUE(*mContexts[CC_M0_F0] == cct_eq);

    // Different objects with difference in one relevant attribute
    CallingContext cct_neq0(CC_OOB, *mRegions[REG_FOO],
                            *mSourceLocs[SCL_FOO_0], mContexts[CC_M0]);
    EXPECT_FALSE(*mContexts[CC_M0_F0] == cct_neq0);

    CallingContext cct_neq1(CC_M0_F0, *mRegions[REG_BAR],
                            *mSourceLocs[SCL_FOO_0], mContexts[CC_M0]);
    EXPECT_FALSE(*mContexts[CC_M0_F0] == cct_neq1);

    CallingContext cct_neq2(CC_M0_F0, *mRegions[REG_FOO],
                            *mSourceLocs[SCL_FOO_1], mContexts[CC_M0]);
    EXPECT_FALSE(*mContexts[CC_M0_F0] == cct_neq2);

    CallingContext cct_neq3(CC_M0_F0, *mRegions[REG_FOO],
                            *mSourceLocs[SCL_FOO_0], 0);
    EXPECT_FALSE(*mContexts[CC_M0_F0] == cct_neq3);
}


TEST_F(CallingContextT, testInequality)
{
    // Self-comparison
    EXPECT_FALSE(*mContexts[CC_M0_F0] != *mContexts[CC_M0_F0]);

    // Different object with identical values
    CallingContext cct_eq(CC_M0_F0, *mRegions[REG_FOO],
                          *mSourceLocs[SCL_FOO_0], mContexts[CC_M0]);
    EXPECT_FALSE(*mContexts[CC_M0_F0] != cct_eq);

    // Different objects with difference in one relevant attribute
    CallingContext cct_neq0(CC_OOB, *mRegions[REG_FOO],
                            *mSourceLocs[SCL_FOO_0], mContexts[CC_M0]);
    EXPECT_TRUE(*mContexts[CC_M0_F0] != cct_neq0);

    CallingContext cct_neq1(CC_M0_F0, *mRegions[REG_BAR],
                            *mSourceLocs[SCL_FOO_0], mContexts[CC_M0]);
    EXPECT_TRUE(*mContexts[CC_M0_F0] != cct_neq1);

    CallingContext cct_neq2(CC_M0_F0, *mRegions[REG_FOO],
                            *mSourceLocs[SCL_FOO_1], mContexts[CC_M0]);
    EXPECT_TRUE(*mContexts[CC_M0_F0] != cct_neq2);

    CallingContext cct_neq3(CC_M0_F0, *mRegions[REG_FOO],
                            *mSourceLocs[SCL_FOO_0], 0);
    EXPECT_TRUE(*mContexts[CC_M0_F0] != cct_neq3);
}
