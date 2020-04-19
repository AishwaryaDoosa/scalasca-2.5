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


#include <config.h>
#include "Hypothesis.h"

#include "HypothesisPart.h"

using namespace std;
using namespace pearl;
using namespace silas;

/**
 *  Standard constructor
 */
Hypothesis::Hypothesis()
{
}

/**
 * Destructor cleaning up registered partials hypotheses
 */
Hypothesis::~Hypothesis()
{
    while (!parts.empty())
    {
        delete parts.back(); 
        parts.pop_back();
    }
}

/**
 * Add a new partial hypothesis
 * @param part Pointer to an object of a partial hypothesis
 */
void Hypothesis::add_part(HypothesisPart* part)
{
    parts.push_back(part);
}

void Hypothesis::register_callbacks(const int run, CallbackManager* cbmanager) const
{
  for (std::vector<HypothesisPart*>::const_iterator it = parts.begin(); it != parts.end(); ++it)
    (*it)->register_callbacks(run, cbmanager);
}
