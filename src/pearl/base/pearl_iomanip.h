/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2013                                                **
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


#ifndef PEARL_IOMANIP_H
#define PEARL_IOMANIP_H


#include <iosfwd>


/*-------------------------------------------------------------------------*/
/**
 *  @file    pearl_iomanip.h
 *  @ingroup PEARL_base
 *  @brief   Declaration of PEARL-specific stream I/O functionality.
 *
 *  This header file provides the declaration of some PEARL-specific stream
 *  I/O manipulators and format flag handlers.
 **/
/*-------------------------------------------------------------------------*/


namespace pearl
{
/*-------------------------------------------------------------------------*/
/**
 *  @class   iendl
 *  @ingroup PEARL_base
 *  @brief   Stream manipulator implementing "endl with indentation".
 *
 *  This class implements a stream I/O manipulator performing an "endl with
 *  indentation". That is, after inserting a newline, the specified number
 *  of space characters is put to the output stream to indent the following
 *  line.
 **/
/*-------------------------------------------------------------------------*/

class iendl
{
    public:
        //--- Public methods -------------------------------

        /// @name Constructors & destructor
        /// @{

        /// @brief Constructor.
        ///
        /// Creates a new instance storing the number of spaces used to
        /// indent the following line.
        ///
        /// @param  indent  Number of spaces to indent
        ///
        explicit
        iendl(int indent);

        /// @}


    private:
        //--- Data members ---------------------------------

        /// Number of spaces to indent the following line
        int mIndent;


        //--- Private methods & friends --------------------

        friend std::ostream&
        operator<<(std::ostream& stream,
                   const iendl&  item);
};


//--- Related functions -----------------------------------------------------

/// @name Stream I/O functions
/// @{

/// @brief   Stream output operator.
/// @relates iendl
///
/// Inserts a newline, followed by the number of spaces defined by the
/// manipulator object @a item into the given output @a stream.
///
/// @param  stream  Output stream
/// @param  item    %iendl manipulator object
///
std::ostream&
operator<<(std::ostream& stream,
           const iendl&  item);

/// @}
/// @name Query/modify I/O stream format flags
/// @{

/// @brief Get indentation level.
///
/// Returns the current indentation level of the given @a stream.
///
/// @param  stream  Output stream
///
/// @return Indentation level
///
int
getIndent(std::ostream& stream);

/// @brief Set indentation level.
///
/// Sets the given indentation @a level on the output @a stream.
///
/// @param  stream  Output stream
/// @param  level   New indentation level
///
/// @return Output stream
///
std::ostream&
setIndent(std::ostream& stream,
          int           level);

/// @}
}   // namespace pearl


#endif   // !PEARL_IOMANIP_H
