//---------------------------------------------------------------------------- 
// picoasm
//
// Stripped out the GUI from kpicosim to create a command-line Picoblaze
// assembler and VHDL/verilog file generator.
//
//---------------------------------------------------------------------------- 

//---------------------------------------------------------------------------- 
// History - most recent first 
/*****************************************************************************
02/02/2007 V 0.2
* Added assembler listing file = src file name with .log ext.
* By default look for template file in source dir.
* Suppress symbol table print to cout (in CAssembler::buildSymbolTable()). 
* Added bVHDL parm to CAssembler::exportVHDL() to process either VHDL or 
  verilog. 
* George found problem with instrOR_SX_SY in CAssembler::addInstruction().
  Fixed it here. 
/*****************************************************************************
02/01/2007 V 0.1
Initial version based on kpicosim version 0.1.
* Uses cassembler, cpicoblaze, and cinstruction.cpp/.h files and
  types.h, hexcodes.h
* Modifications:
  - In any of these, I changed "\r\n" to use endl. This makes the endlines
    in cout platform independent.
  - In cassembler, I removed references to the QT m_messageList.
  - My code is in main.cpp        
* Just does VHDL output, but planning for adding verilog.
*****************************************************************************/
//---------------------------------------------------------------------------- 

#include "cassembler.h"
#include "cpicoblaze.h"
#include "cinstruction.h"

#include <string>
#include <iostream>
#include <stdio.h>      // printf
#include <unistd.h>     // getopt
#include <libgen.h>     // dirname, basename
#include <strings.h>    // strcasecmp
#include <string.h>

#define LA_PICOASM_DEF_TPL      "ROM_form.v"      // default template file
#define LA_PICOASM_VERILOG_EXT  ".v"              // verilog file ext
#define LA_PICOASM_VHDL_EXT     ".vhd"            // VHDL file ext
#define LA_PICOASM_LISTING_EXT  ".log"            // assembler listing ext 

using namespace std;

static const char version[] = "0.2";
static const char AppDescription[] = "Picoblaze Assembler based on kpicosim";

CPicoBlaze *m_picoBlaze;
CAssembler *m_assembler; 

// function prototypes ------------------------- 
bool printListing(string strFileName, string outputFileName);
void usage(string strName);

//---------------------------------------------------------------------------- 
// usage
// Print app usage
//
// parms: strName: app name
//
//  ret: none
//---------------------------------------------------------------------------- 
void usage(string strName){

  printf("%s Version %s - %s\n", strName.c_str(), version, AppDescription);
  printf("USAGE:\n");
  printf(" -i <input file>      Picoblaze source file\n");
  printf(" [-t <template file>] Verilog/VHDL template file.\n");
  printf(" [-v <output file>]   write generated Verilog output to file.\n");  
  printf(" [-v <output file>]   write generated VHDL output to file.\n");  
  printf(" [-m <module name>]   Verilog module or VHDL entity name.\n"
	 "                      Default = input file base name.\n");  
  printf(" [-H <filename>]      Output assembled code in special ASCII hex format to file.\n");
  printf(" [-l <filename>]      name file to put debug listing.\n");
}

int main(int argc, char **argv)
{

  string strSrcFile;
  string strTplFile;
  string strVerilogFileName;
  string strVHDLFileName;
  string strEntityName;
  string hexFileName;
  string listingFileName;

  int iCompareLen;

  const char optstring[] = "i:t:d:m:l:V:v:H:";
  bool bOptErr = false;
  int optch;

  char *cpTemp;
  char *cpBase;
  char *cpExt;

  bool bRet; 
  int iRet = 0;

  while ((optch = getopt(argc, argv, optstring)) != -1){
    switch (optch)
      {
      case 'i': // input source file 
	strSrcFile = optarg;
	break;

      case 't': // input template file
	strTplFile = optarg;
	break;

      case 'm': // entity or module name (for verilog / VHDL template)
	strEntityName = optarg;
	break;

      case 'v': // verilog output filename (constructed from verilog / VHDL template)
	strVerilogFileName = optarg;
	break;

      case 'V': // VHDL output filename  
	strVHDLFileName = optarg;
	break;

      case 'H': // hex output filename
        hexFileName = optarg;
        break;

      case 'l': // listing output filename
        listingFileName = optarg;
        break;

      default:
	cout << "ERR: Unknown command line option" << endl; 
	bOptErr = true;
	break;

      } // switch
  } // while

  // check cmd line options
  if (strSrcFile.empty()){
    cout << "ERR: Input source file missing." << endl; 
    usage(basename(argv[0]));
    return (-1);
  }
  
  if (bOptErr){
    usage(basename(argv[0]));
    return (-1);
  }
  
  if (strEntityName.empty()){
    // build entity name from source file name with no path or extension 
    cpTemp = strdup(strSrcFile.c_str());
    cpBase = basename(cpTemp);
    cpExt = strrchr(cpBase, '.');
    if (cpExt != NULL){
      *cpExt = '\0';
    }
    strEntityName = cpBase;
    free(cpTemp);
  }
  
  m_picoBlaze = new CPicoBlaze();
  m_assembler = new CAssembler();

  m_assembler->setCode(m_picoBlaze->code);
  m_assembler->setFilename(strSrcFile);

  if (!m_assembler->assemble())
  {
    cout << "Error compiling assembly file " << strSrcFile << endl;
    iRet = -1;
  }
  else 
  {
    // m_picoBlaze->code->Print();
    if (!listingFileName.empty())
    {
      printListing(strSrcFile, listingFileName);
      cout << "Generated listing file " << listingFileName << endl;
    }

    if (!strVHDLFileName.empty())
    {
	cout << "ERR: VHDL entity file not generated" << endl;
        if (m_assembler->exportVHDL(strTplFile, strVHDLFileName, strEntityName, true))
        {
          cout << "Generated VHDL entity file " << strVHDLFileName << endl;
        }
        else 
        {
          cout << "ERR: VHDL entity file not generated" << endl;
          iRet = -1;
        }
    }
	
    if (!strVerilogFileName.empty())
    {
      cout << "ERR: Verilog module file not generated" << endl;
      if (m_assembler->exportVHDL(strTplFile, strVerilogFileName, strEntityName, false))
      {
        cout << "Generated VHDL entity file " << strVerilogFileName << endl;
      }
      else
      {
	cout << "ERR: Verilog module file not generated" << endl;
        iRet = -1;
      }
    }

    if (!hexFileName.empty())
    {
      if (m_assembler->exportHex(hexFileName))
      {          
        cout << "Generated hex file '" << hexFileName << "'" << endl;
      }
      else 
      {
	cout << "ERR: Hex file not generated" << endl;
        iRet = -1;
      }
    }
  }

  delete m_assembler;
  delete m_picoBlaze;

  return(iRet);
}

//---------------------------------------------------------------------------- 
// printListing
// Print assembler listing
//
// parms: strFileName: picoblaze source file name
//
//  ret: True: good print   False: problem
//---------------------------------------------------------------------------- 
bool printListing(string strFileName, string strAsmListing){

  FILE *f ;

  f = fopen( strFileName.c_str(), "r" ) ;

  if ( f == NULL ) {
    cout << "ERR: Unable to load file '" << strFileName << "'" << endl;
    return (false);
  }

  char *cpTemp;
  char *cpExt;
  FILE *fListing;

  fListing = fopen(strAsmListing.c_str(), "w") ;

  if ( fListing == NULL ) {
    cout << "ERR: Unable to open assembler listing file '" 
	 << strAsmListing << "'" << endl;
    fclose(f);
    return (false);
  }

  char buf[ 256 ] ;
  int linenr = 0 ;
  int iCodeLine = 0;
  uint16_t uiAddr = 0;
  uint32_t uiHexCode = 0;

  CInstruction *instr = m_picoBlaze->code->getInstruction( uiAddr ) ;

  if (instr == NULL){
    iCodeLine = 999999;  // no code in file - use a giant line number
  } else {
    iCodeLine = instr->getSourceLine();
    uiHexCode = instr->getHexCode();
  }

  fprintf(fListing, "\n");
  fprintf(fListing, "%s  Version %s\n", AppDescription, version);   // heading
  fprintf(fListing, "Source File: %s\n", strFileName.c_str());
  fprintf(fListing, "\n");
  fprintf(fListing, "Line  Addr Instr  Source Code\n");   
  //                "llll  AAA  HHHHH  SSSSSSSSS...", 

  while( fgets( buf, sizeof( buf ), f ) ) {
    // if this is the next code line 
    if (linenr == iCodeLine){
      fprintf(fListing, 
	      "%4d  %03x  %05x  %s", 
	      linenr + 1, uiAddr, uiHexCode, buf);  // one-base line number
      // get next instruction
      uiAddr++;
      instr = m_picoBlaze->code->getInstruction( uiAddr ) ;
      if (instr == NULL){
	// no more code in file
	iCodeLine = 999999;  // no code in file - use a giant line number
      } else {
	iCodeLine = instr->getSourceLine();
	uiHexCode = instr->getHexCode();
      }
    } else {
      fprintf(fListing, "%4d              %s", 
	      linenr + 1, buf);                    // one-based line number
    }
    linenr++;
  }

  fclose (f);
  fclose(fListing);
		
  return (true);

}
