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
 *  @brief Implementation of the class MultiRunConfig.
 *
 *  This file provides the implementation of the class MultiRunConfig and
 *  related functions.
 **/
/*-------------------------------------------------------------------------*/


#include <config.h>

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "MultiRunConfig.h"

using namespace std;


// A config file has two valid states: inside a singular GLOBAL variable
// section or in a RUN variable section. All other entries are ignored.
enum CfgStates
{
    UNDEFINED,
    GLOBAL,
    RUN
};


MultiRunConfig::MultiRunConfig()
{
    // Separation between multi-run measurements with configuration file and
    // pure repetitions, the latter keeps the original environment.
    mHasMultirunCfg = false;

    mRunsPerCfg = 1;
}


string
MultiRunConfig::checkEnvBlacklist()
{
    string blacklist = "";

    if (!mHasMultirunCfg && (mRunsPerCfg <= 1))
    {
        // No need to check if there is no multi-run config or additional runs
        return "";
    }

    string cmd = "scorep-info --help >/dev/null 2>&1";
    int    ret = system(cmd.c_str());
    if (ret != 0)
    {
        message(
            "Error: 'scorep-info' not available, required for checking the environment in multi run measurements with config files!",
            true);
        exit(1);
    }

    // Check list of variables that should not be set in the global environment.
    // Instead use global section of run config file for consistency

    // all scorep variables
    cmd = "scorep-info config-vars --values | sed 's/=.*$//g'";
    FILE* pipe = popen(cmd.c_str(), "r");
    if (!pipe)
    {
        message("Error: Could not run:" + cmd, true);
        exit(1);
    }

    char confvar[1024];
    while (fgets(confvar, 1024, pipe))
    {
        string env_val = "";
        confvar[strlen(confvar) - 1] = '\0';
        mClearList.push_back(string(confvar));
        if (getenv(confvar))
        {
            env_val = getenv(confvar);
        }
        if (env_val.length() > 0)
        {
            blacklist += string(confvar) + "=" + env_val + " ";
        }
    }
    pclose(pipe);

    // Scalasca variables from a hard-coded list:
    string scal_list[9] = {
        "SCAN_ANALYZE_OPTS", "SCAN_CLEAN",          "SCAN_MPI_LAUNCHER",
        "SCAN_MPI_RANKS",    "SCAN_OVERWRITE",      "SCAN_SETENV",
        "SCAN_TARGET",       "SCAN_TRACE_ANALYZER", "SCAN_WAIT"
    };

    mClearList.insert(mClearList.end(), scal_list, scal_list+9);

    for (int i = 0; i < 9; i++)
    {
        string env_val = "";
        if (getenv(scal_list[i].c_str()))
        {
            env_val = getenv(scal_list[i].c_str());
        }
        if (env_val.length() > 0)
        {
            blacklist += scal_list[i] + "=" + env_val + " ";
        }
    }

    if (!mHasMultirunCfg)
    {
        // repeats only, we need mClearList for later use, but not the blacklist
        blacklist = "";
    }

    return blacklist;
}


void
MultiRunConfig::setRunVariables(int runId)
{
    if (!mHasMultirunCfg)
    {
        return;
    }

    if (mPerRunVariables.size() > 0)
    {
        // Unset all variables used in run configurations.
        // The clean list contains Score-P and Scalasca related variables as well as all
        // variables from all run configurations.
        // If either of these are intended to be used, they have to be set explicitly
        // either in the global section or in respective run configurations
        // Only reset environment with config file, pure repetitions keep their environment.
        if (mHasMultirunCfg)
        {
            for (int i = 0; i < mClearList.size(); i++)
            {
                unsetenv(mClearList[i].c_str());
            }
        }
        else
        {
            return;
        }

        // Set variables from the global section, can be overwritten in the per run settings except for special cases.
        for (map< string, string >::iterator it = mGlobalVariables.begin();
             it != mGlobalVariables.end();
             ++it)
        {
            message("Set global var: " + it->first + " to " + it->second);
            setenv(it->first.c_str(), it->second.c_str(), 1);
        }

        // Set variables for this run.
        if (runId < mPerRunVariables.size())
        {
            stringstream sstm;
            sstm << runId;
            message("Run no: " + sstm.str());
            for (map< string, string >::iterator it =
                     mPerRunVariables[runId].begin();
                 it != mPerRunVariables[runId].end();
                 ++it)
            {
                message("Set var: " + it->first + " to " + it->second);
                setenv(it->first.c_str(), it->second.c_str(), 1);
            }
        }
    }
}


void
MultiRunConfig::load(const string& fileName)
{
    string   line;
    ifstream input_file(fileName.c_str());

    if (!input_file)
    {
        message("Error: Opening file " + fileName, true);
        exit(1);
    }

    // Store the source file name for later documentation.
    mInputCfgFiles.push_back(fileName);
    mCfgFileStartCfg.push_back(mPerRunVariables.size());

    CfgStates cfg_state              = UNDEFINED;
    bool      been_in_run_section    = false;
    bool      been_in_global_section = false;
    mParsingLineNumber = 0;
    mParsingErrorCount = 0;
    while (getline(input_file, line))
    {
        mParsingLineNumber++;

        // Remove unwanted clutter from line.
        string clean_line = cleanLine(line);

        // Skip if is empty or comment.
        if ((clean_line.size() == 0) || (line[0] == '#'))
        {
            continue;
        }

        // Order of separator checks relevant as they may contain similar checks
        if (isGlobalSeparator(clean_line))
        {
            if (  (been_in_run_section && !been_in_global_section)
               || been_in_global_section)
            {
                // Global has to be the first section if it exists at all
                // and only one global section for consistency.
                parserError(
                    "Either multiple global sections or run sections before the global section!");
            }
            been_in_global_section = true;
            cfg_state              = GLOBAL;
            continue;
        }

        if (isRunSeparator(clean_line))
        {
            been_in_run_section = true;
            cfg_state           = RUN;
            mPerRunVariables.resize(mPerRunVariables.size() + 1);
            continue;
        }

        // Deal with lines containing envrionment variables.
        if (cfg_state != UNDEFINED)
        {
            pair< string, string > valuepair = getValuePair(clean_line);
            mClearList.push_back(valuepair.first);
            if (cfg_state == GLOBAL)
            {
                mGlobalVariables[valuepair.first] = valuepair.second;
            }
            else if (cfg_state == RUN)
            {
                mPerRunVariables[mPerRunVariables.size()
                                 - 1][valuepair.first] = valuepair.second;
            }
        }
        else
        {
            // UNDEFINED state
            parserError(
                "Variable statements are not allowed outside global or run sections, '--' or '-' have to come first!");
        }
    }

    if (!been_in_run_section && !been_in_global_section)
    {
        parserError(
            "Neither run nor global section defined, there has to be at least one valid section!");
    }

    // check if the parsing resulted in any errors and abort.
    if (mParsingErrorCount)
    {
        cerr << endl << "Aborting. Configuration file parsing of '" << fileName
             << "' encountered " << mParsingErrorCount << " errors." << endl;
        exit(1);
    }

    if (mPerRunVariables.size() > 0)
    {
        mHasMultirunCfg = true;
    }
}


void
MultiRunConfig::saveConfigAs(const string& fileName)
{
    if (!mHasMultirunCfg && (mRunsPerCfg <= 1))
    {
        return;
    }

    std::ofstream output_file(fileName.c_str());

    if (!output_file)
    {
        std::cerr << "Error opening file " << fileName << endl;
        exit(2);
    }


    if (!mHasMultirunCfg && (mRunsPerCfg > 1))
    {
        // just repetitions of the default environment
        // dump known and set Score-P and Scalasca variables as Global section of a cfg file
        output_file
            <<
            "# Generated multiple run configuration for a case with repeated runs only"
            << endl;
        output_file
            <<
            "# Contains only known variables, e.g., Score-P/Scalasca variables."
            << endl;
        output_file << "-- Global Variable Section" << endl;

        for (int i = 0; i < mClearList.size(); i++)
        {
            if (mClearList[i] != "SCOREP_EXPERIMENT_DIRECTORY")
            {
                // directory will be set automatically and differs from run to run
                if (getenv(mClearList[i].c_str()))
                {
                    output_file << mClearList[i] << "=" << getenv(
                        mClearList[i].c_str()) << endl;
                }
            }
        }

        output_file.close();

        return;
    }

    // If a config file was used

    output_file << "# Stored multiple run configuration" << endl;
    output_file << "# Used input config files:" << endl;
    for (int i = 0; i < mInputCfgFiles.size(); i++)
    {
        output_file << "#   " << mInputCfgFiles[i] << endl;
    }

    output_file << endl << "-- Global Variable Section" << endl;
    if (mCfgFileStartCfg.size() > 1)
    {
        output_file
            <<
            "# If combined from multiple files, the last setting of a variable is valid."
            << endl;
    }
    for (map< string, string >::iterator it = mGlobalVariables.begin();
         it != mGlobalVariables.end();
         ++it)
    {
        output_file << it->first.c_str() << "=" << it->second.c_str() << endl;
    }

    int fileCounter = 0;
    for (int i = 0; i < mPerRunVariables.size(); i++)
    {
        if (  (mCfgFileStartCfg.size() > 1)
           && (mCfgFileStartCfg[fileCounter] == i))
        {
            output_file << endl << "# Imported from config file: "
                        << mInputCfgFiles[fileCounter] << endl;
            fileCounter++;
        }
        output_file << endl << "- Run Config " << i + 1 << endl;
        for (map< string, string >::iterator it =
                 mPerRunVariables[i].begin();
             it != mPerRunVariables[i].end();
             ++it)
        {
            output_file << it->first.c_str() << "=" << it->second.c_str()
                        << endl;
        }
    }
    output_file.close();
}


int
MultiRunConfig::getNumSets()
{
    if (mPerRunVariables.size() == 0)
    {
        // If no set is explicitly stored, there still will be an implicit default run.
        return 1;
    }

    return mPerRunVariables.size();
}


// String manipulation and parsing methods

string
MultiRunConfig::cleanLine(const string& line)
{
    string clean  = line;
    string remove = " \t";


    clean.erase(0, clean.find_first_not_of(remove.c_str()));
    clean.erase(clean.find_last_not_of(remove.c_str()) + 1);

    return clean;
}


pair< string, string >
MultiRunConfig::getValuePair(const string& line)
{
    pair< string, string > key_value;
    size_t                 sep_pos = line.find('=');

    if (sep_pos == string::npos)
    {
        parserError("Key value pair '" + line + "' missing '=' separator!");

        return key_value;
    }
    key_value.first = line.substr(0, sep_pos);

    if (key_value.first.find(' ') != string::npos)
    {
        parserError(
            "Key '" + line
            + "' contains spaces, invalid environment variable!");

        return key_value;
    }
    if (sep_pos == line.length() - 1)
    {
        // empty value, no need to substr the value
        return key_value;
    }

    key_value.second = line.substr(sep_pos + 1, line.length() - (sep_pos + 1));

    return key_value;
}


bool
MultiRunConfig::isGlobalSeparator(const string& line)
{
    if (line.size() > 1)
    {
        if ((line[0] == '-') && (line[1] == '-'))
        {
            return true;
        }
    }

    return false;
}


bool
MultiRunConfig::isRunSeparator(const string& line)
{
    if (line.size() > 0)
    {
        if (line[0] == '-')
        {
            return true;
        }
    }

    return false;
}


void
MultiRunConfig::message(const string& msg,
                        bool          force)
{
    if (mVerbose || force)
    {
        cerr << msg << endl;
    }
}


void
MultiRunConfig::parserError(const string& msg)
{
    if (mParsingErrorCount == 0)
    {
        cerr << endl << "Parsing configuration file '" << mInputCfgFiles.back()
             << "':" << endl;
    }
    cerr << "Error line " << setw(3) << mParsingLineNumber << ": " << msg
         << endl;
    mParsingErrorCount++;
}
