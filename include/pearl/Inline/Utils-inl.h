/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2016-2017                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


#include <cstdio>


namespace pearl
{
// --- Conversion to string -------------------------------------------------

inline std::string
toStdString(const int value)
{
    // Conservative upper bound for length of character representation
    const unsigned int maxLength = 4 * sizeof(int);

    char      result[maxLength];
    const int length = std::snprintf(result, maxLength, "%d", value);

    return std::string(result, result + length);
}


inline std::string
toStdString(const long value)
{
    // Conservative upper bound for length of character representation
    const unsigned int maxLength = 4 * sizeof(long);

    char      result[maxLength];
    const int length = std::snprintf(result, maxLength, "%ld", value);

    return std::string(result, result + length);
}


inline std::string
toStdString(const long long value)
{
    // Conservative upper bound for length of character representation
    const unsigned int maxLength = 4 * sizeof(long long);

    char      result[maxLength];
    const int length = std::snprintf(result, maxLength, "%lld", value);

    return std::string(result, result + length);
}


inline std::string
toStdString(const unsigned int value)
{
    // Conservative upper bound for length of character representation
    const unsigned int maxLength = 4 * sizeof(unsigned int);

    char      result[maxLength];
    const int length = std::snprintf(result, maxLength, "%u", value);

    return std::string(result, result + length);
}


inline std::string
toStdString(const unsigned long value)
{
    // Conservative upper bound for length of character representation
    const unsigned int maxLength = 4 * sizeof(unsigned long);

    char      result[maxLength];
    const int length = std::snprintf(result, maxLength, "%lu", value);

    return std::string(result, result + length);
}


inline std::string
toStdString(const unsigned long long value)
{
    // Conservative upper bound for length of character representation
    const unsigned int maxLength = 4 * sizeof(unsigned long long);

    char      result[maxLength];
    const int length = std::snprintf(result, maxLength, "%llu", value);

    return std::string(result, result + length);
}
}    // namespace pearl
