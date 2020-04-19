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


#ifndef PEARL_MPICANCELLED_REP_H
#define PEARL_MPICANCELLED_REP_H


#include <pearl/Event_rep.h>


/*-------------------------------------------------------------------------*/
/**
 *  @file    MpiCancelled_rep.h
 *  @ingroup PEARL_mpi
 *  @brief   Declaration of the class MpiCancelled_rep.
 *
 *  This header file provides the declaration of the class MpiCancelled_rep.
 **/
/*-------------------------------------------------------------------------*/


namespace pearl
{

/*-------------------------------------------------------------------------*/
/**
 *  @class   MpiCancelled_rep
 *  @ingroup PEARL_mpi
 *  @brief   %Event representation for MPI_CANCELLED events.
 **/
/*-------------------------------------------------------------------------*/

class PEARL_NOPAD_ATTR MpiCancelled_rep : public Event_rep
{
  public:
    /// @name Constructors & destructor
    /// @{

    MpiCancelled_rep(timestamp_t timestamp,
                     uint64_t    requestId);
    MpiCancelled_rep(const GlobalDefs& defs, Buffer& buffer);

    /// @}
    /// @name Event type information
    /// @{

    virtual event_t getType() const;
    virtual bool    isOfType(event_t type) const;

    /// @}
    /// @name Access event data
    /// @{

    virtual uint64_t getRequestId() const;

    /// @}
    /// @name Modify event data
    /// @{

    virtual void setRequestId(uint64_t requestId);

    /// @}


  protected:
    /// @name Generate human-readable output of event data
    /// @}

    virtual std::ostream& output(std::ostream& stream) const;

    /// @}
    /// @name Find previous/next communication request entries
    /// @{

    virtual uint32_t get_next_reqoffs() const;
    virtual uint32_t get_prev_reqoffs() const;

    virtual void     set_prev_reqoffs(uint32_t);
    virtual void     set_next_reqoffs(uint32_t);

    /// @}


  private:
    /// Non-blocking request ID
    uint64_t mRequestId;

    /// Offset to previous (test or request) event
    uint32_t m_prev_reqoffs;
};


}   // namespace pearl


#endif   // !PEARL_MPICANCELLED_REP_H
