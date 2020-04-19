/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2015                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  Copyright (c) 2009-2014                                                **
**  German Research School for Simulation Sciences GmbH,                   **
**  Laboratory for Parallel Programming                                    **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


#ifndef SCOUT_CBDATA_H
#define SCOUT_CBDATA_H

#include <set>
#include <set>

#include <pearl/CallbackData.h>
#include <pearl/Event.h>

#include "scout_types.h"
#include "fixed_set.hh"

/*-------------------------------------------------------------------------*/
/**
 * @file  CbData.h
 * @brief Declaration of the CbData struct.
 *
 * This header file provides the declaration of the CbData struct which
 * stores the data required for the pattern search replay.
 */
/*-------------------------------------------------------------------------*/


//--- Forward declarations --------------------------------------------------

namespace pearl
{

class LocalData;
class RemoteData;
class GlobalDefs;
class LocalTrace;

}   // namespace pearl


namespace scout
{
/*-------------------------------------------------------------------------*/

class OmpEventHandler;
class SynchpointHandler;
class TmapCacheHandler;
class Callstack;

/**
 * @struct CbData
 * @brief  Datatype aggregating all data which is required for the pattern
 *         search replay.
 *
 * The CbData struct provides a datatype which aggregates a number of values
 * which are required during the pattern search replay. In most cases, this
 * is data which has to be passed between callbacks (i.e., from a base pattern
 * to a more specific sub-pattern).
 *
 * @todo Review, potentially rename and document all undocumented attributes.
 */
/*-------------------------------------------------------------------------*/

struct CbData
    : public pearl:: CallbackData
{
    /// @brief   Local data
    /// @details Collects local pattern constituents of point-to-point
    ///          communication patterns before sending them to the peer.
    pearl::LocalData* mLocal;

    /// @brief   Remote data
    /// @details Stores remote pattern constituents of point-to-point
    ///          communication patterns after receiving them from the peer.
    pearl::RemoteData* mRemote;

    /// @brief   Local event set
    /// @details Collects local pattern constituents of point-to-point
    ///          communication patterns before sending them to the peer.
    pearl::LocalData* mInvLocal;

    /// @brief   Remote event set
    /// @details Stores remote pattern constituents of point-to-point
    ///          communication patterns after receiving them from the peer.
    pearl::RemoteData* mInvRemote;

    /// Global definitions object
    pearl::GlobalDefs* mDefs;
    /// Local trace object
    pearl::LocalTrace* mTrace;

    /// MPI Synchpoint Handler

    SynchpointHandler*    mSynchpointHandler;
    TmapCacheHandler*     mTmapCacheHandler;
    OmpEventHandler*      mOmpEventHandler;

    /// Container for Information on collective operations
    CollectiveInfo mCollinfo;

    /// @brief   Detected waiting time
    /// @details Used to transfer the already calculated waiting time to
    ///          sub-patterns.
    pearl::timestamp_t mIdle;
    /// @brief   Detected completion time
    pearl::timestamp_t mCompletion;

    /// Remote waiting time (delay analysis)
    double            mRwait;
    /// Current synchronization interval begin (delay analysis)
    pearl::Event      mPrevSp;
    /// Delay info (delay analysis)
    TimeMapSum        mDelayInfo;
    /// Wait time info (delay analysis)
    TimeMapSum        mWaitInfo;
    /// Propagating synchpoints (delay analysis)
    SynchpointInfoMap mPropSpMap;
    /// Input scale values (delay analysis of MPI collectives)
    std::map<uint32_t, double>     mInputScales;
    /// Sum scales (delay analysis of MPI collectives)
    std::map<uint32_t, ScaleMap>   mSumScales;
    /// Max scales (delay analysis of MPI collectives)
    std::map<uint32_t, ScaleMap>   mMaxScales;
    /// Delay costs for MPI collectives
    std::map<uint32_t, TimeMapSum> mCollDelayInfo;
    /// Scale for OpenMP idleness
    double            mOmpIdleScale;
    /// rank for synchpoints
    int mSyncRank;

    /// Global critical-path profile (extended critical-path analysis)
    std::vector<double> mGlobalCriticalPath;
    /// Local critical-path profile (extended critical-path analysis)
    std::vector<double> mLocalCriticalPath;
    /// Local non-waiting time profile (extended critical-path analysis)
    std::vector<double> mLocalTimeProfile;

    /// temporary storage for an event
    pearl::Event mEvent;

    /// time and local rank for RMA ops
    TimeRank mLastPost;
    TimeRank mLastComplete;
    TimeRank mLastRmaOp;

    TimeVec2 mTimevec;

    /// Stores GATS-Synchpoint ranks in backwards replay
    fixed_set<int> mGatsSyncRanks;

    /// Stores remote comm_id for WfP
    uint32_t mCommId;

    /// Stores extra info for WfP synchpoints
    CommInfoMap mSyncPointCommInfo;


    /// @brief Detected count;
    int mCount;

    /// @brief Number of Targets in RMA epoch
    int mCountTarget;

    /// @brief Number of Origins in RMA epoch
    int mCountOrigin;

    /// Current call stack
    Callstack* mCallstack;

    /// Thread stub node management
    std::map <uint64_t, uint32_t>                mThreadStubNodeIds;
    static std::map <uint64_t, pearl::Callpath*> mThreadStubNodeParents;
    std::map <uint64_t, double>                  mThreadStartTime;

    /// @}
    /// @name Asynchronous analysis
    /// @}

    /// @brief Cnode id
    pearl::Callpath::IdType mCallpathId;

    /// window pointer
    pearl::MpiWindow* mWindow;

    std::set<int>* mLocations;

    /// @}
    /// @name Constructors & destructor
    /// @{

    CbData();
    virtual
    ~CbData();

    /// @}
    /// @name Pre- and postprocessing
    /// @{

    virtual void
    postprocess(const pearl::Event& event);

    /// @}
};
}   // namespace scout


#endif   // !SCOUT_CBDATA_H
