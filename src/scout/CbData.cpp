/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2015                                                **
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


#include <config.h>
#include "CbData.h"

#include <cstddef>

#include <pearl/Enter_rep.h>
#include <pearl/Region.h>
#include <pearl/ThreadCreate_rep.h>
#include <pearl/ThreadBegin_rep.h>

#if defined(_MPI)
  #include <pearl/LocalData.h>
  #include <pearl/RemoteData.h>
#endif   // _MPI

#include "Callstack.h"

using namespace std;
using namespace pearl;
using namespace scout;


//---------------------------------------------------------------------------
//
//  class CbData
//
//---------------------------------------------------------------------------

std::map <uint64_t, pearl::Callpath*> CbData::mThreadStubNodeParents;


//--- Constructors & destructor ---------------------------------------------

/**
 *  @brief Constructor.
 *
 *  Creates a new callback data object.
 **/

CbData::CbData()
    : mLocal(NULL),
      mRemote(NULL),
      mInvLocal(NULL),
      mInvRemote(NULL),
      mDefs(NULL),
      mTrace(NULL),
      mSynchpointHandler(NULL),
      mTmapCacheHandler(NULL),
      mOmpEventHandler(NULL) 
{
    mCallstack = new Callstack;
  #if defined(_MPI)
    mLocal     = new LocalData;
    mRemote    = new RemoteData;
    mInvLocal  = new LocalData;
    mInvRemote = new RemoteData;
  #endif   // _MPI
}


/**
 *  @brief Destructor.
 *
 *  Frees up all occupied resources and destroys the instance.
 **/
CbData::~CbData()
{
    delete mCallstack;
  #if defined(_MPI)
    delete mLocal;
    delete mRemote;
    delete mInvLocal;
    delete mInvRemote;
  #endif   // _MPI
}


//--- Pre- and postprocessing -----------------------------------------------

/**
 *  @brief Postprocessing after an event is handled.
 *
 *  Updates a number of attributes after the given @a event has been handled.
 *  In particular, the current callstack is updated and the local and remote
 *  event sets are cleared.
 *
 *  @param  event  Processed event
 **/
void
CbData::postprocess(const Event& event)
{
#ifdef _OPENMP
    if (event->isOfType(THREAD_CREATE))
    {
        ThreadCreate_rep& create = event_cast<ThreadCreate_rep>(*event);

        // 1st step: store id for the parent of the stub node
        //mThreadStubNodeIds[create.getSequenceCount()]=event.get_cnode()->getId();
        #pragma omp critical 
        mThreadStubNodeParents[create.getSequenceCount()] = event.get_cnode();
    }
    // create wait threading: stub node handling    
    else if (event->isOfType(THREAD_BEGIN))
    {
        ThreadBegin_rep& begin = event_cast<ThreadBegin_rep>(*event);

        // ensure the creating thread did his work - busy wait
        bool waiting = true;
        while (waiting)
        {
            #pragma omp critical 
            {
                if (mThreadStubNodeParents.find(begin.getSequenceCount()) != mThreadStubNodeParents.end())
                    waiting = false;
            }
        }

        Callpath* parent = mThreadStubNodeParents[begin.getSequenceCount()];
        //store starting time for the time calculation of the stub node
        //  next == ENTER of thread function
        //  usual event order: thread begin, <thread function>
        //  checking for next normal enter (no flushing, pausing)
        Event next = event.next();
        while (next.is_valid())
        {
            if (next->isOfType(GROUP_ENTER))
            {
                Enter_rep& test = event_cast<Enter_rep>(*next);
                if (test.getRegion().getRole() != Region::ROLE_ARTIFICIAL)
                    break;
            }
            next=next.next();
        }
        // if the loop didn't find a matching enter, your trace is broken
        assert (next.is_valid());

        mThreadStartTime[begin.getSequenceCount()]=next->getTimestamp();

        // get the matching child from the parent we stored in the CREATE
        const Region& nextThreadRegion = next.get_cnode()->getRegion();
        for (uint32_t i = 0; i < parent->numChildren(); ++i)
        {
            if (parent->getChild(i).getRegion() == nextThreadRegion)
            {
                mThreadStubNodeIds[begin.getSequenceCount()] = parent->getChild(i).getId();
                break;
            }
        }
    }
#endif   // _OPENMP

    // Update callstack
    mCallstack->update(event);

    // Clear event sets
  #if defined(_MPI)
    mLocal->clear();
    mRemote->clear();
    mInvLocal->clear();
    mInvRemote->clear();
  #endif   // _MPI

    // Clear delay analysis info
    mRwait  = 0;
    mPrevSp = Event();
    mDelayInfo.mMap.clear();
    mDelayInfo.mSum = 0.0;
    mWaitInfo.mMap.clear();
    mWaitInfo.mSum  = 0.0;
    mPropSpMap.clear();
    mSumScales.clear();
    mMaxScales.clear();
}
