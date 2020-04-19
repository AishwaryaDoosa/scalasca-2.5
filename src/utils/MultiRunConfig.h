/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 2018-2019                                                **
**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **
**                                                                         **
**  This software may be modified and distributed under the terms of       **
**  a BSD-style license.  See the COPYING file in the package base         **
**  directory for details.                                                 **
****************************************************************************/


/*-------------------------------------------------------------------------*/
/**
 *  @file
 *  @brief Declaration of the class MultiRunConfig.
 *
 *  This header file provides the declaration of the class MultiRunConfig and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#ifndef MULTIRUNCONFIG_H
#define MULTIRUNCONFIG_H


#include <map>
#include <string>
#include <utility>
#include <vector>

/*-------------------------------------------------------------------------*/
/**
 *  @class MultiRunConfig
 *  @brief Managing sets of variables for different run configurations.
 *
 **/
/*-------------------------------------------------------------------------*/

class MultiRunConfig
{
    public:
        MultiRunConfig();

        // Sets the number of runs per configuration and verbosity level of
        // the scan run
        void
        setInfo(unsigned int runsPerCfg,
                unsigned int verbose)
        {
            mVerbose    = verbose;
            mRunsPerCfg = runsPerCfg;
        }

        // Checks the global environment against a list of Score-P and Scalasca
        // environment variables that should not be set in case of a multi-run
        // experiment with config file
        std::string
        checkEnvBlacklist();

        // Set the variables stored for the runId and unset all other run
        // variables to avoid interference.
        void
        setRunVariables(int runId);

        // Load input config file
        void
        load(const std::string& fileName);

        // Store the configuration inside the archive for reference.  Generates
        // a configuration file for repeat-only measurements containing
        // Score-P/Scalasca variables.
        void
        saveConfigAs(const std::string& fileName);

        // Returns the number of run sets.
        int
        getNumSets();


    private:
        // The verbosity level of the scan run
        unsigned int mVerbose;

        // Number of runs for each run configuration
        unsigned int mRunsPerCfg;

        // True if at least one multi-run configuration file has been specified,
        // false otherwise
        bool mHasMultirunCfg;

        // Used config files
        std::vector< std::string >  mInputCfgFiles;
        std::vector< unsigned int > mCfgFileStartCfg;

        // Variables of the global section
        std::map< std::string, std::string > mGlobalVariables;

        // Variables of each run section
        std::vector< std::map< std::string, std::string > > mPerRunVariables;

        // Variables to be cleared before setting the run config.  Cumulative
        // list of Score-P and Scalasca variables, as well as variables set in
        // the configuration file.
        std::vector< std::string > mClearList;


        // String manipulation and parsing methods

        // Remove leading, trailing spaces etc
        std::string
        cleanLine(const std::string& line);

        // Parse an environment variable value pair
        std::pair< std::string, std::string >
        getValuePair(const std::string& line);

        // Recognize a global section of a configuration file
        bool
        isGlobalSeparator(const std::string& line);

        // Recognize a run section of a configuration file
        bool
        isRunSeparator(const std::string& line);


        // SCAN output based on verbosity
        void
        message(const std::string& msg,
                bool               force = false);

        // Error checking during config file parsing
        int mParsingLineNumber;
        int mParsingErrorCount;
        void
        parserError(const std::string& msg);
};


#endif    // !MULTIRUNCONFIG_H
