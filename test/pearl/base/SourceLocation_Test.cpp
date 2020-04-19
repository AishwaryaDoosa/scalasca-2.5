/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2015                                                     **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


#include <config.h>
#include <pearl/SourceLocation.h>

#include <algorithm>
#include <limits>
#include <sstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include <pearl/String.h>

#include "Functors.h"

using namespace std;
using namespace testing;
using namespace pearl;
using namespace pearl::detail;


//--- Identifier constants --------------------------------------------------

enum StringIds
{
    STR_FOO,
    STR_BAR,
    STR_OOB
};


enum SclIds
{
    SCL_FOO,
    SCL_OOB
};


enum Lines
{
    LINE_FOO = 42,
    LINE_OOB
};


//--- SourceLocation tests --------------------------------------------------

// Test fixture used for source-code location definition tests
class SourceLocationT
    : public Test
{
    public:
        virtual void
        SetUp()
        {
            mStrings.push_back(new String(STR_FOO, "foo.cpp"));
            mStrings.push_back(new String(STR_BAR, "bar.cpp"));

            mSourceLocs.push_back(new SourceLocation(SCL_FOO,
                                                     *mStrings[STR_FOO],
                                                     LINE_FOO));
        }

        virtual void
        TearDown()
        {
            for_each(mStrings.begin(), mStrings.end(), delete_ptr<String>());
            for_each(mSourceLocs.begin(), mSourceLocs.end(),
                     delete_ptr<SourceLocation>());
        }

    protected:
        vector<String*>         mStrings;
        vector<SourceLocation*> mSourceLocs;
};


TEST_F(SourceLocationT, testStatics)
{
    EXPECT_EQ(numeric_limits<SourceLocation::IdType>::max(),
              SourceLocation::NO_ID);

    EXPECT_EQ(SourceLocation::NO_ID, SourceLocation::UNDEFINED.getId());
    EXPECT_EQ(String::UNDEFINED, SourceLocation::UNDEFINED.getFilename());
    EXPECT_EQ(0, SourceLocation::UNDEFINED.getLine());
}


TEST_F(SourceLocationT, testStreamOutput)
{
    // Special case: undefined source-code location
    {
        string expected("<undefined>");

        ostringstream result;
        result << SourceLocation::UNDEFINED;
        EXPECT_EQ(expected, result.str());
    }

    // "Regular" source-code location
    {
        string expected("SourceLocation {\n"
                        "  id       = 0\n"
                        "  filename = String(0, \"foo.cpp\")\n"
                        "  line     = 42\n"
                        "}");

        ostringstream result;
        result << *mSourceLocs[SCL_FOO];
        EXPECT_EQ(expected, result.str());
    }
}


TEST_F(SourceLocationT, testEquality)
{
    // Self-comparison
    EXPECT_TRUE(*mSourceLocs[SCL_FOO] == *mSourceLocs[SCL_FOO]);

    // Different object with identical values
    SourceLocation scl_eq(SCL_FOO, *mStrings[STR_FOO], LINE_FOO);
    EXPECT_TRUE(*mSourceLocs[SCL_FOO] == scl_eq);

    // Different objects with difference in one relevant attribute
    SourceLocation scl_neq0(SCL_OOB, *mStrings[STR_FOO], LINE_FOO);
    EXPECT_FALSE(*mSourceLocs[SCL_FOO] == scl_neq0);

    SourceLocation scl_neq1(SCL_FOO, *mStrings[STR_BAR], LINE_FOO);
    EXPECT_FALSE(*mSourceLocs[SCL_FOO] == scl_neq1);

    SourceLocation scl_neq2(SCL_FOO, *mStrings[STR_FOO], LINE_OOB);
    EXPECT_FALSE(*mSourceLocs[SCL_FOO] == scl_neq2);
}


TEST_F(SourceLocationT, testInequality)
{
    // Self-comparison
    EXPECT_FALSE(*mSourceLocs[SCL_FOO] != *mSourceLocs[SCL_FOO]);

    // Different object with identical values
    SourceLocation scl_eq(SCL_FOO, *mStrings[STR_FOO], LINE_FOO);
    EXPECT_FALSE(*mSourceLocs[SCL_FOO] != scl_eq);

    // Different objects with difference in one relevant attribute
    SourceLocation scl_neq0(SCL_OOB, *mStrings[STR_FOO], LINE_FOO);
    EXPECT_TRUE(*mSourceLocs[SCL_FOO] != scl_neq0);

    SourceLocation scl_neq1(SCL_FOO, *mStrings[STR_BAR], LINE_FOO);
    EXPECT_TRUE(*mSourceLocs[SCL_FOO] != scl_neq1);

    SourceLocation scl_neq2(SCL_FOO, *mStrings[STR_FOO], LINE_OOB);
    EXPECT_TRUE(*mSourceLocs[SCL_FOO] != scl_neq2);
}
