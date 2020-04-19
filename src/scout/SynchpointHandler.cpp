/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2018                                                **
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


#include <config.h>

#include "SynchpointHandler.h"

#include "CbData.h"
#include "Callstack.h"
#include "Roles.h"
#include "scout_types.h"
#include "user_events.h"
#include "fixed_set.hh"

#include <pearl/Buffer.h>
#include <pearl/CallbackManager.h>
#include <pearl/CommSet.h>
#include <pearl/Event.h>
#include <pearl/EventSet.h>
#include <pearl/GlobalDefs.h>
#include <pearl/LocalData.h>
#ifdef _MPI
#include <pearl/MpiCollEnd_rep.h>
#include <pearl/MpiComm.h>
#endif
#include <pearl/Region.h>
#include <pearl/RemoteData.h>
#include <pearl/RemoteEventSet.h>

#include <set>

using namespace scout;
using namespace pearl;
using namespace std;

struct SynchpointHandler::SpHImpl
{
    //
    // --- private data --------------------------------------------------
    //

    typedef std::map<pearl::Event, fixed_set<int>, const pearl::EventKeyCompare>
      MpiRanksMap;
    typedef std::set<pearl::Event, const pearl::EventKeyCompare>
      EventSet;

    SynchpointInfoMap mSynchpoints;
    MpiRanksMap       mSynchRanks;
    EventSet          mOmpSynchpoints;


    //
    // --- query interface
    //

    bool isSynchpoint(const Event& e) const {
        return mSynchpoints.find(e) != mSynchpoints.end();
    }

    bool isWaitstate(const Event& e) const {
        SynchpointInfoMap::const_iterator it = mSynchpoints.find(e);

        return (it != mSynchpoints.end() && (it->second.waitTime > 0.0));
    }

    Event findPreviousOmpSynchpoint(const Event& e) const {
        EventSet::const_iterator it = mOmpSynchpoints.find(e);

        // assert(it != mOmpSynchpoints.end());

        EventSet::const_reverse_iterator rit(it);

        // assert(rit != mOmpSynchpoints.rend());
        if (rit != mOmpSynchpoints.rend())
            return *rit;

        return Event();
    }

    Event findPreviousMpiSynchpoint(const Event& e, int rank) const {
        MpiRanksMap::const_iterator it = mSynchRanks.find(e);

        assert(it != mSynchRanks.end());

        MpiRanksMap::const_reverse_iterator rit(it);

        for ( ; rit != mSynchRanks.rend() && !rit->second.contains(rank); ++rit)
            ;

        assert(rit != mSynchRanks.rend());

        return rit->first;
    }

    Event findPreviousMpiGroupSynchpoints(const Event&          from,
                                          const CommSet&        group,
                                          int                   rank,
                                          uint32_t              nprocs,
                                          std::map<int, Event>& spmap) const
    {
        spmap.clear();

        fixed_set<int> remaining(0, nprocs - 1);

        if (group.numRanks() == nprocs)
            remaining.fill();
        else
            for (unsigned i = 0; i < group.numRanks(); ++i)
                remaining.insert(group.getGlobalRank(i));

        remaining.erase(rank);

        MpiRanksMap::const_iterator it = mSynchRanks.find(from);
        assert(it != mSynchRanks.end());
        MpiRanksMap::const_reverse_iterator rit(it);

        for ( ; rit != mSynchRanks.rend() && !remaining.empty(); ++rit) {
            if (rit->second >= remaining)
                remaining.clear();
            else {
                fixed_set<int> iset = remaining.intersection_with(rit->second);

                for (fixed_set<int>::iterator it = iset.begin(); it != iset.end(); ++it)
                    spmap[group.getLocalRank(*it)] = rit->first;

                remaining -= iset;
            }
        }

        assert(remaining.empty());

        return (--rit)->first;
    }

    SynchpointInfo getSynchpointInfo(const Event& e) const {
        if (!e.is_valid()) {
            SynchpointInfo spi = { 0, 0, 0, 0 }; return spi;
        }

        SynchpointInfoMap::const_iterator it = mSynchpoints.find(e);

        if (it == mSynchpoints.end()) {
            SynchpointInfo spi = { 0, 0, 0, 0 }; return spi;
        }

        return it->second;
    }

    SynchpointInfoMap getSynchpoints() const {
        return mSynchpoints;
    }

    SynchpointInfoMap getSynchpointsBetween(const Event& from, const Event& to) const {
        SynchpointInfoMap::const_iterator itf, itt;

        if (from.is_valid()) {
            itf = mSynchpoints.find(from);
            assert(itf != mSynchpoints.end());
            ++itf;
        } else {
            itf = mSynchpoints.begin();
        }

        if (to.is_valid()) {
            itt = mSynchpoints.find(to);
            assert(itt != mSynchpoints.end());
        } else {
            itt = mSynchpoints.end();
        }

        return SynchpointInfoMap(itf, itt);
    }


    //
    // --- helper functions ----------------------------------------------
    //

    //
    // --- main replay callbacks -----------------------------------------
    //

#ifdef _MPI

    SCOUT_CALLBACK(cb_pre_ls) {
        CbData* data = static_cast<CbData*>(cdata);

        SynchpointInfo spi;

        spi.waitTime       = data->mIdle;
        spi.totalTime      =
            event.leaveptr()->getTimestamp() - event.enterptr()->getTimestamp();
        spi.completionTime = data->mCompletion;

        fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);

        syncranks.insert(event->getComm()->getCommSet().getGlobalRank(event->getSource()));

        mSynchpoints.insert(make_pair(event, spi));
        mSynchRanks.insert(make_pair(event, syncranks));
    }

    SCOUT_CALLBACK(cb_pre_coll_12n) {
        CbData* data = static_cast<CbData*>(cdata);

        const CommSet&  commSet(event->getComm()->getCommSet());
        CollectiveInfo& ci(data->mCollinfo);
        const uint32_t  nranks = commSet.numRanks();
        SynchpointInfo  spi;

        spi.waitTime  = 0;
        spi.totalTime =
            event.leaveptr()->getTimestamp() - event.enterptr()->getTimestamp();

        fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);

        // --- gather remote times on root

        double*  rv      = (ci.my.mRank == ci.root.mRank) ? new double[nranks] : 0;
        double   myenter = ci.my.mTime;

        MPI_Gather(&myenter, 1, MPI_DOUBLE,
                   rv,       1, MPI_DOUBLE, ci.root.mRank, event->getComm()->getHandle());

        // --- set synchronization info on root

        if (ci.my.mRank == ci.root.mRank)
            for (uint32_t i = 0; i < nranks; ++i)
                if (ci.root.mTime > rv[i])
                    syncranks.insert(commSet.getGlobalRank(i));

        delete[] rv;

        // --- set synchronization info on other processes

        if (ci.my.mTime < ci.root.mTime) {
            spi.waitTime = ci.root.mTime - ci.my.mTime;
            syncranks.insert(commSet.getGlobalRank(ci.root.mRank));
        }

        // FIXME: Compute correct completion time
        spi.completionTime   = 0;

        if (!syncranks.empty()) {
            mSynchpoints.insert(make_pair(event, spi));
            mSynchRanks.insert(make_pair(event, syncranks));
        }
    }

    SCOUT_CALLBACK(cb_pre_coll_n21) {
        CbData* data = static_cast<CbData*>(cdata);

        const CommSet&  commSet(event->getComm()->getCommSet());
        CollectiveInfo& ci(data->mCollinfo);
        SynchpointInfo  spi;

        spi.waitTime  = 0;
        spi.totalTime =
            event.leaveptr()->getTimestamp() - event.enterptr()->getTimestamp();

        fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);

        if (ci.latest.mTime < ci.earliest_end.mTime) {
            // --- check for globally synchronizing reduction

            spi.waitTime = ci.latest.mTime - ci.my.mTime;

            const uint32_t nranks = commSet.numRanks();
            if (nranks == data->mDefs->numLocationGroups()) {
                syncranks.fill();
            } else {
                // sub-communicators: set synch info for each participating process
                for (uint32_t i = 0; i < nranks; ++i)
                    syncranks.insert(commSet.getGlobalRank(i));
            }
        } else if (ci.root.mTime < ci.latest.mTime) {
            // --- check if some process entered after root

            if (ci.my.mRank == ci.root.mRank && ci.my.mRank != ci.latest.mRank) {
                spi.waitTime = ci.latest.mTime - ci.root.mTime;
                syncranks.insert(commSet.getGlobalRank(ci.latest.mRank));
            }

            if (ci.my.mRank == ci.latest.mRank && ci.my.mRank != ci.root.mRank)
                syncranks.insert(commSet.getGlobalRank(ci.latest.mRank));
        }

        // FIXME: Compute correct completion time
        spi.completionTime = 0;

        if (!syncranks.empty()) {
            mSynchpoints.insert(make_pair(event, spi));
            mSynchRanks.insert(make_pair(event, syncranks));
        }
    }

    SCOUT_CALLBACK(cb_pre_coll_n2n) {
        CbData* data = static_cast<CbData*>(cdata);

        CollectiveInfo& ci(data->mCollinfo);

        if (ci.earliest_end.mTime < ci.latest.mTime)
            return;

        SynchpointInfo spi;

        spi.waitTime       = ci.latest.mTime - ci.my.mTime;
        spi.totalTime      =
            event.leaveptr()->getTimestamp() - event.enterptr()->getTimestamp();
        spi.completionTime = event->getTimestamp() - ci.earliest_end.mTime;

        fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);
        const uint32_t nranks = event->getComm()->getSize();

        if (nranks == data->mDefs->numLocationGroups()) {
            syncranks.fill();
        } else if (nranks > 1) {
            for (uint32_t i = 0; i < nranks; ++i)
                syncranks.insert(event->getComm()->getCommSet().getGlobalRank(i));
        }

        if (!syncranks.empty()) {
            mSynchpoints.insert(make_pair(event, spi));
            mSynchRanks.insert(make_pair(event, syncranks));
        }
    }

    SCOUT_CALLBACK(cb_pre_init_finalize) {
        CbData* data = static_cast<CbData*>(cdata);

        CollectiveInfo& ci(data->mCollinfo);

        SynchpointInfo spi;

        spi.waitTime       = ci.latest.mTime - ci.my.mTime;
        spi.totalTime      =
            event->getTimestamp() - data->mCallstack->top()->getTimestamp();
        spi.completionTime = event->getTimestamp() - ci.earliest_end.mTime;

        fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);
        syncranks.fill();

        if (!syncranks.empty()) {
            mSynchpoints.insert(make_pair(event, spi));
            mSynchRanks.insert(make_pair(event, syncranks));
        }
    }
#endif // _MPI

#ifdef _OPENMP
    SCOUT_CALLBACK(cb_pre_omp_mgmt_fork) {
        CbData* data = static_cast<CbData*>(cdata);

        SynchpointInfo spi;

        spi.waitTime  = 0;
        // time between fork event and enter parallel region
        spi.totalTime = data->mIdle;
        spi.completionTime = 0;

        // an OMP fork is always a synchpoint
        mSynchpoints.insert(make_pair(event, spi));
        mOmpSynchpoints.insert(event);
    }

    SCOUT_CALLBACK(cb_pre_omp_barrier) {
        CbData* data = static_cast<CbData*>(cdata);

        SynchpointInfo spi;
        Event enter = data->mCallstack->top();

        spi.waitTime       = 0;
        spi.totalTime      = event->getTimestamp() - enter->getTimestamp();
        spi.completionTime = 0;

        // an OpenMP barrier is always a synchpoint
        mSynchpoints.insert(make_pair(event, spi));
        mOmpSynchpoints.insert(event);
    }

    SCOUT_CALLBACK(cb_pre_omp_barrier_wait) {
        CbData* data = static_cast<CbData*>(cdata);

        // set waiting time in synchpoint info
        if (data->mIdle > 0) {
            SynchpointInfoMap::iterator it = mSynchpoints.find(event);

            assert(it != mSynchpoints.end());

            it->second.waitTime = data->mIdle;
        }
    }
#endif // _OPENMP

    //
    // --- backward synchpoint-detection (bws) callbacks -----------------
    //

#ifdef _MPI
    SCOUT_CALLBACK(cb_bws_pre_recv) {
        CbData* data = static_cast<CbData*>(cdata);

        // send flag whether this recv was a late sender

        Buffer* buf = new Buffer(16);
        SynchpointInfoMap::const_iterator it = mSynchpoints.find(event);

        buf->put_uint32(it != mSynchpoints.end() && it->second.waitTime > 0 ? 1 : 0);
        data->mLocal->add_buffer(buf, BUFFER_SYNC);
        data->mLocal->add_event(event.leaveptr(), ROLE_LEAVE_RECV_SYNC);
    }

    SCOUT_CALLBACK(cb_bws_post_send) {
        CbData* data = static_cast<CbData*>(cdata);

        // get message and see whether this was a late-sender

        if (data->mRemote->get_buffer(BUFFER_SYNC)->get_uint32()) {
            RemoteEvent recv_leave = data->mRemote->get_event(ROLE_LEAVE_RECV_SYNC);
            SynchpointInfo spi;

            spi.waitTime       = 0;
            spi.totalTime      =
                event.leaveptr()->getTimestamp() - event.enterptr()->getTimestamp();
            spi.completionTime = 0;

            if (event.leaveptr()->getTimestamp() > recv_leave->getTimestamp())
                spi.completionTime =
                    event.leaveptr()->getTimestamp() - recv_leave->getTimestamp();

            fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);
            syncranks.insert(event->getComm()->getCommSet().getGlobalRank(event->getDestination()));

            mSynchpoints.insert(make_pair(event, spi));
            mSynchRanks.insert(make_pair(event, syncranks));
        }
    }

    SCOUT_CALLBACK(cb_bws_pre_lr) {
        CbData* data = static_cast<CbData*>(cdata);

        Event completion = event.completion();

        // register local late-receiver instance

        SynchpointInfo spi;

        spi.waitTime       = data->mIdle;
        spi.totalTime      =
            completion.leaveptr()->getTimestamp() - completion.enterptr()->getTimestamp();
        spi.completionTime = data->mCompletion;

        fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);
        syncranks.insert(event->getComm()->getCommSet().getGlobalRank(event->getDestination()));

        mSynchpoints.insert(make_pair(completion, spi));
        mSynchRanks.insert(make_pair(completion, syncranks));
    }
#endif // _MPI

    //
    // --- forward synchpoint-detection (fws) callbacks ------------------
    //

#ifdef _MPI
    SCOUT_CALLBACK(cb_fws_pre_send) {
        CbData* data = static_cast<CbData*>(cdata);

        // send flag whether this send was a late receiver

        Buffer* buf = new Buffer(16);
        SynchpointInfoMap::const_iterator it = mSynchpoints.find(event.completion());

        buf->put_uint32(it != mSynchpoints.end() && it->second.waitTime > 0 ? 1 : 0);
        data->mLocal->add_buffer(buf, BUFFER_SYNC);
        data->mLocal->add_event(event.completion().leaveptr(), ROLE_LEAVE_SEND_SYNC);
    }

    SCOUT_CALLBACK(cb_fws_post_recv) {
        CbData* data = static_cast<CbData*>(cdata);

        // get message and see wether this was a late-receiver

        if (data->mRemote->get_buffer(BUFFER_SYNC)->get_uint32()) {
            SynchpointInfo spi;

            RemoteEvent sendcmp_leave = data->mRemote->get_event(ROLE_LEAVE_SEND_SYNC);
            Event request = event.request();

            spi.waitTime       = 0;
            spi.totalTime      =
                request.leaveptr()->getTimestamp() - request.enterptr()->getTimestamp();
            spi.completionTime = 0;

            if (request.leaveptr()->getTimestamp() > sendcmp_leave->getTimestamp())
                spi.completionTime =
                    request.leaveptr()->getTimestamp() - sendcmp_leave->getTimestamp();

            fixed_set<int> syncranks(0, data->mDefs->numLocationGroups() - 1);
            syncranks.insert(event->getComm()->getCommSet().getGlobalRank(event->getSource()));

            if (mSynchpoints.find(request) == mSynchpoints.end()) {
                mSynchpoints.insert(make_pair(request, spi));
                mSynchRanks.insert(make_pair(request, syncranks));
            }
        }
    }
#endif // _MPI

    //
    // --- callback registration -----------------------------------------
    //

    void
    register_pre_pattern_callbacks(const CallbackManagerMap& cbm)
    {
        struct cb_evt_table_t
        {
            pearl::event_t evt;
            void           (SpHImpl::* cbp)(const pearl::CallbackManager &, int, const pearl::Event &, pearl::CallbackData*);
        };
        struct cb_uevt_table_t
        {
            int  evt;
            void (SpHImpl::* cbp)(const pearl::CallbackManager &, int, const pearl::Event &, pearl::CallbackData*);
        };

        // --- main forward replay callbacks

        const struct cb_evt_table_t main_evt_tbl[] = {
            { NUM_EVENT_TYPES, 0 }
        };

        const struct cb_uevt_table_t main_uevt_tbl[] = {
#ifdef _MPI
            { LATE_SENDER,  &SpHImpl::cb_pre_ls            },
            { COLL_12N,     &SpHImpl::cb_pre_coll_12n      },
            { COLL_N21,     &SpHImpl::cb_pre_coll_n21      },
            { COLL_N2N,     &SpHImpl::cb_pre_coll_n2n      },
            { SYNC_COLL,    &SpHImpl::cb_pre_coll_n2n      },
            { INIT_END,     &SpHImpl::cb_pre_init_finalize },
            { FINALIZE_END, &SpHImpl::cb_pre_init_finalize },
#endif
#ifdef _OPENMP
            { OMP_MGMT_FORK,     &SpHImpl::cb_pre_omp_mgmt_fork    },
            { OMP_EBARRIER,      &SpHImpl::cb_pre_omp_barrier      },
            { OMP_IBARRIER,      &SpHImpl::cb_pre_omp_barrier      },
            { OMP_EBARRIER_WAIT, &SpHImpl::cb_pre_omp_barrier_wait },
            { OMP_IBARRIER_WAIT, &SpHImpl::cb_pre_omp_barrier_wait },
#endif
            { 0, 0 }
        };

        // --- backward wait-state / synchpoint detection callbacks

        const struct cb_evt_table_t bws_evt_tbl[] = {
            { NUM_EVENT_TYPES, 0 }
        };

        const struct cb_uevt_table_t bws_uevt_tbl[] = {
#ifdef _MPI
            { LATE_RECEIVER,  &SpHImpl::cb_bws_pre_lr      },
            { PRE_RECV,       &SpHImpl::cb_bws_pre_recv    },
            { POST_SEND,      &SpHImpl::cb_bws_post_send   },
#endif
            { 0, 0 }
        };

        // --- forward synchpoint detection callbacks

        const struct cb_evt_table_t fws_evt_tbl[] = {
            { NUM_EVENT_TYPES, 0 }
        };

        const struct cb_uevt_table_t fws_uevt_tbl[] = {
#ifdef _MPI
            { PRE_SEND,       &SpHImpl::cb_fws_pre_send    },
            { POST_RECV,      &SpHImpl::cb_fws_post_recv   },
#endif
            { 0, 0 }
        };

        // --- global callback tables table :)

        const struct cb_tables_t
        {
            const char*                   stage;
            const struct cb_evt_table_t*  evts;
            const struct cb_uevt_table_t* uevts;
        }
        cb_tables[] = {
            { "",    main_evt_tbl, main_uevt_tbl },
            { "bws", bws_evt_tbl,  bws_uevt_tbl  },
            { "fws", fws_evt_tbl,  fws_uevt_tbl  },

            {     0,            0,             0 }
        };

        // --- register callbacks

        for (const struct cb_tables_t* t = cb_tables; t->stage; ++t) {
            CallbackManagerMap::const_iterator it = cbm.find(t->stage);

            assert(it != cbm.end());

            for (const struct cb_evt_table_t* c = t->evts; c->cbp; ++c) {
                (it->second)->register_callback(c->evt, PEARL_create_callback(this, c->cbp));
            }
            for (const struct cb_uevt_table_t* c = t->uevts; c->cbp; ++c) {
                (it->second)->register_callback(c->evt, PEARL_create_callback(this, c->cbp));
            }
        }
    }
};


SynchpointHandler::SynchpointHandler()
    : AnalysisHandler(),
      mP(new SpHImpl)
{
}


SynchpointHandler::~SynchpointHandler()
{
}

bool
SynchpointHandler::isSynchpoint(const Event& e) const
{
    return mP->isSynchpoint(e);
}

bool
SynchpointHandler::isWaitstate(const Event& e) const
{
    return mP->isWaitstate(e);
}

SynchpointInfo
SynchpointHandler::getSynchpointInfo(const Event& e) const
{
    return mP->getSynchpointInfo(e);
}

Event
SynchpointHandler::findPreviousMpiSynchpoint(const Event& e, int rank) const
{
    return mP->findPreviousMpiSynchpoint(e, rank);
}

Event
SynchpointHandler::findPreviousOmpSynchpoint(const Event& e) const
{
    return mP->findPreviousOmpSynchpoint(e);
}

Event
SynchpointHandler::findPreviousMpiGroupSynchpoints(const Event&          from,
                                                   const CommSet&        commSet,
                                                   int                   rank,
                                                   uint32_t              nprocs,
                                                   std::map<int, Event>& spmap) const
{
    return mP->findPreviousMpiGroupSynchpoints(from, commSet, rank, nprocs, spmap);
}

SynchpointInfoMap
SynchpointHandler::getSynchpoints() const
{
    return mP->getSynchpoints();
}

SynchpointInfoMap
SynchpointHandler::getSynchpointsBetween(const Event& from, const Event& to) const
{
    return mP->getSynchpointsBetween(from, to);
}

void
SynchpointHandler::register_pre_pattern_callbacks(const CallbackManagerMap& cbm)
{
    mP->register_pre_pattern_callbacks(cbm);
}

void
SynchpointHandler::register_post_pattern_callbacks(const CallbackManagerMap& cbm)
{
    // mP->register_post_pattern_callbacks(cbm);
}
