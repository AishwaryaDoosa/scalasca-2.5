%{
/****************************************************************************
**  SCALASCA    http://www.scalasca.org/                                   **
*****************************************************************************
**  Copyright (c) 1998-2019                                                **
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

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <functional>
#include <map>
#include <string>
#include <vector>

#include "Pattern.h"

using namespace std;


#define YYSTYPE std::string
#define YYDEBUG 1


/*--- Constants -----------------------------------------------------------*/

const char* copyright =
  "/****************************************************************************\n"
  "**  SCALASCA    http://www.scalasca.org/                                   **\n"
  "*****************************************************************************\n"
  "**  Copyright (c) 1998-2019                                                **\n"
  "**  Forschungszentrum Juelich GmbH, Juelich Supercomputing Centre          **\n"
  "**                                                                         **\n"
  "**  Copyright (c) 2009-2014                                                **\n"
  "**  German Research School for Simulation Sciences GmbH,                   **\n"
  "**  Laboratory for Parallel Programming                                    **\n"
  "**                                                                         **\n"
  "**  This software may be modified and distributed under the terms of       **\n"
  "**  a BSD-style license.  See the COPYING file in the package base         **\n"
  "**  directory for details.                                                 **\n"
  "****************************************************************************/\n"
  "\n\n";


/*--- Global variables ----------------------------------------------------*/

extern FILE* yyin;

long lineno = 1;
long depth  = 0;

string inpFilename;
string incFilename;
int    incLine;

string prolog;
string prefix("Patterns_gen");

map<string,Pattern*> id2pattern;    // pattern id |-> pattern
vector<Pattern*>     pattern;       // pattern list
Pattern*             current;       // current pattern
string               callbackgroup; // current callback group

/*--- Function prototypes -------------------------------------------------*/

int main(int argc, char** argv);

void write_header();
void write_impl();
void write_html();

string uppercase(const string& str);

void yyerror(const string& message);

extern int  yylex();
extern void include_file(const string& filename);


%}


%token STRING TEXT
%token CALLBACKS CLASS CLEANUP CONDITION DATA DESCR DOCNAME DIAGNOSIS HIDDEN
%token INCLUDE INFO INIT MODE NAME NODOCS PARENT PATTERN PROLOG STATICINIT
%token TYPE UNIT


%%

File            : Body
                ;


Body            : BodyItem
                | Body BodyItem
                ;


BodyItem        : Include
                | Pattern
                | Prolog
                ;


Include         : INCLUDE String
                  {
                    include_file($2);
                  }
                ;


Prolog          : PROLOG CodeBlock
                  {
                    prolog += $2;
                  }
                ;


Pattern         : PATTERN String
                  {
                    if (id2pattern.find($2) != id2pattern.end())
                      yyerror("Pattern \"" + $2 + "\" already defined!");

                    current = new Pattern($2);
                    pattern.push_back(current);
                    id2pattern.insert(make_pair($2, current));
                  }
                  '=' '[' PatternDef ']'
                  {
                    bool error = true;
                    string msg = "Incomplete pattern definition!\n";
                    switch(current->is_valid()) {
                      case DEF_NAME:
                        msg += "Missing NAME definition.";
                        break;
                      case DEF_CLASS:
                        msg += "Missing CLASS definition.";
                        break;
                      case DEF_INFO:
                        msg += "Missing INFO definition.";
                        break;
                      case DEF_UNIT:
                        msg += "Missing UNIT definition.";
                        break;
                      case DEF_MODE:
                        msg += "Missing MODE definition.";
                        break;
                      default:
                        error = false;
                        break;
                    }
                    if (error)
                      yyerror(msg);
                  }
                ;

PatternDef      : PatternDef DefItem
                | DefItem
                ;


DefItem         : Name
                | Classname
                | Docname
                | Parent
                | Type
                | Hidden
                | NoDocs
                | Info
                | Description
                | Diagnosis
                | Unit
                | Mode
                | Condition
                | Init
                | StaticInit
                | Cleanup
                | Data
                | Callbacks
                ;


Name            : NAME '=' String
                  {
                    if (!current->get_name().empty())
                      yyerror("Only one NAME definition allowed!");

                    current->set_name($3);
                  }
                ;


Classname       : CLASS '=' String
                  {
                    if (!current->get_classname().empty())
                      yyerror("Only one CLASS definition allowed!");

                    current->set_classname($3);
                  }
                ;


Docname         : DOCNAME '=' String
                  {
                    current->set_docname($3);
                  }
                ;


Parent          : PARENT '=' String
                  {
                    if (current->get_parent() != "NONE")
                      yyerror("Only one PARENT definition allowed!");


                    map<string,Pattern*>::iterator it = id2pattern.find($3);
                    if (it == id2pattern.end())
                      yyerror("Unknown pattern \"" + $3 + "\"!");
                    if (it->second == current)
                      yyerror("A pattern cannot be its own parent!");

                    current->set_parent($3);
                  }
                ;


Type            : TYPE '=' String
                  {
                    if ($3 != "MPI" &&
                        $3 != "MPIDEP" &&
                        $3 != "MPI_RMA" &&
                        $3 != "OMP" &&
                        $3 != "OMPDEP" &&
                        $3 != "PTHREAD" &&
                        $3 != "Generic")
                      yyerror("Unknown pattern type \"" + $3 + "\"");

                    current->set_type($3);
                  }
                ;


Hidden          : HIDDEN
                  {
                    current->set_hidden();
                  }
                ;


NoDocs          : NODOCS
                  {
                    current->set_nodocs();
                  }
                ;


Info            : INFO '=' String
                  {
                    if (!current->get_info().empty())
                      yyerror("Only one INFO definition allowed!");

                    current->set_info($3);
                  }
                ;


Description     : DESCR '=' TextBlock
                  {
                    if (!current->get_descr().empty())
                      yyerror("Only one DESCR definition allowed!");

                    current->set_descr($3);
                  }
                ;


Diagnosis       : DIAGNOSIS '=' TextBlock
                  {
                    if (!current->get_diagnosis().empty())
                      yyerror("Only one DIAGNOSIS definition allowed!");

                    current->set_diagnosis($3);
                  }
                ;


Unit            : UNIT '=' String
                  {
                    if (!current->get_unit().empty())
                      yyerror("Only one UNIT definition allowed!");

                    current->set_unit($3);
                  }
                ;


Mode            : MODE '=' String
                  {
                    if (!current->get_mode().empty())
                      yyerror("Only one MODE definition allowed!");
                    if ($3 != "inclusive" &&
                        $3 != "exclusive")
                      yyerror("Unknown pattern mode \"" + $3 + "\"");

                    current->set_mode($3);
                  }
                ;

Condition       : CONDITION '=' String
                  {
                    if (!current->get_condition().empty())
                      yyerror("Only one CONDITION definition allowed!");

                    current->set_condition($3);
                  }
                ;


Init            : INIT '=' CodeBlock
                  {
                    if (!current->get_init().empty())
                      yyerror("Only one INIT definition allowed!");

                    current->set_init($3);
                  }
                ;


StaticInit      : STATICINIT '=' CodeBlock
                  {
                    if (!current->get_staticinit().empty())
                      yyerror("Only one STATICINIT definition allowed!");

                    current->set_staticinit($3);
                  }
                ;


Cleanup         : CLEANUP '=' CodeBlock
                  {
                    if (!current->get_cleanup().empty())
                      yyerror("Only one CLEANUP definition allowed!");

                    current->set_cleanup($3);
                  }
                ;


Data            : DATA '=' CodeBlock
                  {
                    if (!current->get_data().empty())
                      yyerror("Only one DATA definition allowed!");

                    current->set_data($3);
                  }
                ;


Callbacks       : CALLBACKS '(' String ')' 
                  {
                    callbackgroup = $3;
                  }
                  '=' '[' CbList ']'
                | CALLBACKS
                  {
                    callbackgroup = "";
                  }
                  '=' '[' CbList ']'
                ;


CbList          : CbList CbItem
                | CbItem
                ;


CbItem          : String '=' CodeBlock
                  {
                    if (!current->add_callback(callbackgroup, $1, $3))
                      yyerror("Callback \"" + $1 + "\" already defined!");
                  }
                ;

String          : '"' STRING '"'
                  {
                    $$ = $2;
                  }
                ;


CodeBlock       : '{'
                  {
                    char line_str[32];
                    snprintf(line_str, 32, "\n#line %ld \"", lineno);

                    $$ = line_str +
                         (incFilename.empty() ? inpFilename : incFilename) +
                         "\"\n";
                  }
                  Text '}'
                  {
                    $$ = $2 + $3;
                  }
                ;


TextBlock       : '{' Text '}'
                  {
                    $$ = $2;
                  }
                ;


Text            : Text TEXT
                  {
                    $$ = $1 + $2;
                  }
                | TEXT
                  {
                    $$ = $1;
                  }
                ;


%%


/*
 *---------------------------------------------------------------------------
 *
 * gen_pattern <description file>
 *
 *---------------------------------------------------------------------------
 */

int main(int argc, char** argv)
{
  /* Check command line arguments */
  if (argc != 2) {
    fprintf(stderr, "Usage: gen_patterns <description file>\n");
    exit(EXIT_FAILURE);
  }

  /* Store input filename */
  inpFilename = argv[1];

  /* Open input file */
  yyin = fopen(inpFilename.c_str(), "r");
  if (!yyin) {
    fprintf(stderr, "Could not open file \"%s\".\n", inpFilename.c_str());
    exit(EXIT_FAILURE);
  }

  /* Parse input file */
  yyparse();

  /* Close input file */
  fclose(yyin);

  /* Write output */
  write_header();
  write_impl();
  write_html();

  /* Clean up */
  vector<Pattern*>::iterator it = pattern.begin();
  while (it != pattern.end()) {
    delete *it;
    ++it;
  }

  return EXIT_SUCCESS;
}


void write_header()
{
  /* Open header file */
  string filename = prefix + ".h";
  FILE* fp = fopen(filename.c_str(), "w");
  if (!fp) {
    fprintf(stderr, "Could not open output file \"%s\".\n", filename.c_str());
    exit(EXIT_FAILURE);
  }

  /* Temporary strings */
  string prefix_upper = uppercase(prefix);

  /* Write copyright notice */
  fprintf(fp, "%s", copyright);

  /* Open include guard */
  fprintf(fp, "#ifndef SCOUT_%s_H\n", prefix_upper.c_str());
  fprintf(fp, "#define SCOUT_%s_H\n", prefix_upper.c_str());
  fprintf(fp, "\n\n");

  /* Open namespace */
  fprintf(fp, "namespace scout\n"
              "{\n"
              "\n");

  /* Write pattern constants */
  int num = -1;
  fprintf(fp, "//--- Constants -------------------------------------------------------------\n\n");
  fprintf(fp, "const long PAT_%-30s = %3d;\n", "NONE", num++);
  vector<Pattern*>::const_iterator it = pattern.begin();
  while (it != pattern.end()) {
    fprintf(fp, "const long PAT_%-30s = %3d;\n", (*it)->get_id().c_str(), num++);
    ++it;
  }
  fprintf(fp, "\n\n");

  /* Write forward declarations */
  fprintf(fp, "//--- Forward declarations --------------------------------------------------\n"
              "\n"
              "class AnalyzeTask;\n"
              "\n\n");

  /* Write function prototypes */
  fprintf(fp, "//--- Function prototypes ---------------------------------------------------\n"
              "\n"
              "void create_patterns(AnalyzeTask* analyzer);\n"
              "\n\n");

  /* Close namespace */
  fprintf(fp, "}   /* namespace scout */\n"
              "\n\n");

  /* Close include guard */
  fprintf(fp, "#endif   /* !SCOUT_%s_H */\n", prefix_upper.c_str());

  /* Close file */
  fclose(fp);
}


void write_impl()
{
  /* Open implementation file */
  string filename = prefix + ".cpp";
  FILE* fp = fopen(filename.c_str(), "w");
  if (!fp) {
    fprintf(stderr, "Could not open output file \"%s\".\n", filename.c_str());
    exit(EXIT_FAILURE);
  }

  /* Write copyright notice */
  fprintf(fp, "%s", copyright);

  /* Write common includes */
  fprintf(fp, "#include <config.h>\n"
              "#include \"Patterns_gen.h\"\n\n"
              "#include <cassert>\n"
              "#include <inttypes.h>\n\n"
              "#include <pearl/CallbackManager.h>\n"
              "#include <pearl/Callpath.h>\n"
              "#include <pearl/GlobalDefs.h>\n\n");
  fprintf(fp, "#include \"AnalyzeTask.h\"\n"
              "#include \"CbData.h\"\n"
              "#include \"MpiPattern.h\"\n"
              "#ifdef _OPENMP\n"
              "#  include \"OmpPattern.h\"\n"
              "#  include \"PthreadPattern.h\"\n"
              "#endif\n\n");

  /* Write "using" directives */
  fprintf(fp, "using namespace std;\n"
              "using namespace pearl;\n"
              "using namespace scout;\n"
              "\n\n");

  /* Write prolog */
  fprintf(fp, "%s\n\n", prolog.c_str());

  /* Write class implementation */
  vector<Pattern*>::const_iterator it = pattern.begin();
  while (it != pattern.end()) {
    (*it)->write_impl(fp);
    ++it;
  }

  /* Write function implementation */
  fprintf(fp, "//--- Implementation --------------------------------------------------------\n"
              "\n"
              "void scout::create_patterns(AnalyzeTask* analyzer)\n"
              "{\n");
  it = pattern.begin();
  while (it != pattern.end()) {
    if ((*it)->skip_impl()) {
      ++it;
      continue;
    }

    const struct guard {
      const char *type, *begin, *end;
    } guards[] = {
      { "MPI",    
        "#if defined(_MPI)\n", 
        "#endif   /* _MPI */\n" 
      },
      { "MPIDEP", 
        "#if defined(_MPI)\n", 
        "#endif   /* _MPI */\n" 
      },
      { "MPI_RMA",
        "#if defined(_MPI) && defined(HAS_MPI2_1SIDED)\n", 
        "#endif   /* _MPI && HAS_MPI2_1SIDED */\n"
      },
      { "MPI_RMADEP",
        "#if defined(_MPI) && defined(HAS_MPI2_1SIDED)\n", 
        "#endif   /* _MPI && HAS_MPI2_1SIDED */\n"
      },
      { "OMP",
        "#if defined(_OPENMP)\n",
        "#endif   /* _OPENMP */\n"        
      },
      { "OMPDEP",
        "#if defined(_OPENMP)\n",
        "#endif   /* _OPENMP */\n"        
      },
      { "PTHREAD",
        "#if defined(_OPENMP)\n",
        "#endif   /* _OPENMP */\n"        
      },
      { "ARMCI",
        "#if defined(HAS_ARMCI)\n",
        "#endif   /* HAS_ARMCI */\n"
      },
      { "ARMCIDEP",
        "#if defined(HAS_ARMCI)\n",
        "#endif   /* HAS_ARMCI */\n"
      },

      { 0, 0, 0 }
    };

    const struct guard* g;

    for (g = guards; g->type && (*it)->get_type() != g->type; ++g)
      ;

    if (g->begin)
      fprintf(fp, "%s", g->begin);

    if (!(*it)->get_condition().empty())
      fprintf(fp, "  if (%s)\n  ", (*it)->get_condition().c_str());

    fprintf(fp, "  analyzer->addPattern(new %s());\n",
                (*it)->get_classname().c_str());

    if (g->end)
      fprintf(fp, "%s", g->end);

    ++it;
  }
  fprintf(fp, "}\n\n\n");

  /* Close file */
  fclose(fp);
}


void write_html()
{
  /* Open documentation file */
  string filename = prefix + ".html";
  FILE* fp = fopen(filename.c_str(), "w");
  if (!fp) {
    fprintf(stderr, "Could not open output file \"%s\".\n", filename.c_str());
    exit(EXIT_FAILURE);
  }

  /* Write file header */
  fprintf(fp, "<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 4.01 Transitional//EN\">\n"
              "<html>\n"
              "<head>\n"
              "<title>Performance properties</title>\n"
              "</head>\n"
              "<body>\n");

  /* Write page header */
  fprintf(fp, "<h2>Performance properties</h2>\n\n");

  /* Write class documentation */
  bool isFirst = true;
  vector<Pattern*>::const_iterator it = pattern.begin();
  while (it != pattern.end()) {
    (*it)->write_html(fp, isFirst);
    isFirst = false;
    ++it;
  }

  /* Close file */
  fclose(fp);
}


void yyerror(const string& message)
{
  if (!incFilename.empty())
    fprintf(stderr, "In included file \"%s\":\n   ", incFilename.c_str());

  fprintf(stderr, "Line %ld: %s\n", lineno, message.c_str());
  exit(EXIT_FAILURE);
}


struct fo_toupper : public std::unary_function<int,int> {
  int operator()(int x) const {
    return std::toupper(x);
  }
};


string uppercase(const string& str)
{
  string result(str);

  transform(str.begin(), str.end(), result.begin(), fo_toupper());

  return result;
}
