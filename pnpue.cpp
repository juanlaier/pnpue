/* == LINKER OPTIONS ======================================================= */
#pragma comment(linker, "/nodefaultlib:libcmt") // No MT static library
#pragma comment(lib, "msvcrt.lib")              // Use MSVCR100.DLL instead!
/* == DEFINES ============================================================== */
#define  _CRT_SECURE_NO_WARNINGS       // Disable MS compiler moan
/* == C INCLUDES =========================================================== */
#include <stdio.h>                     // Standard IO functions
#include <time.h>                      // Time functions
/* -- C++ INCLUDES --------------------------------------------------------- */
#include <map>                         // Want std::map class
#include <string>                      // Want std::string class
/* -- C++ NAMESPACES ------------------------------------------------------- */
using std::map;                        // Make std::map global namespace
using std::string;                     // Make std::string global namespace
/* == ENTRY ================================================================ */
const unsigned int main(const unsigned int uiArgC, const char **cpArgV) try
{ // Run pnputil command and stream the output
  FILE *fp = _popen("pnputil.exe -e", "rb");
  // If failed?
  if(!fp) throw string("Could not execute 'PNPUTIL.EXE'!");
  // Our little data record struct for our database
  typedef struct _STRVAR {
    string strPublishedName;
    string strDriverPackageProvider;
    string strClass;
    string strDriverDate;
    string strDriverVersion;
    string strSignerName;
  } STRVAR;
  // Version list typedef
  typedef map<const string, STRVAR> VERSIONLIST;
  typedef VERSIONLIST::iterator VERSIONLISTI;
  typedef map<const string, VERSIONLIST> VERSIONDB;
  typedef VERSIONDB::iterator VERSIONDBI;
  // Databases for chunked data and m3u8 variables and values
  VERSIONDB mData;
  // Current item
  STRVAR svCurrent;
  // Show progress line
  printf("Enumerating Windows Driver Store...");
  // Total items
  size_t stTotal = 0;
  // Until process finished
  while(!feof(fp))
  { // Check for error
    if(ferror(fp)) break;
    // Create buffer, reserve 1K and clear it
    string strLine(1024, '\0');
    // Read line and if not succeeded, keep retrying
    if(!fgets((char*)strLine.c_str(), 1024, fp)) continue;
    // Update size of string because we bypassed C++ functions
    strLine.resize(strlen(strLine.c_str()));
    // Remove any extra suffixed control characters
    while(!strLine.empty()&&strLine[strLine.length()-1]<32) strLine.pop_back();
    // Line empty? Try next line
    if(strLine.empty()) continue;
    // Find delimiter and ignore if not found
    const size_t stDelim = strLine.find(" :");
    if(stDelim == string::npos) continue;
    // Get field name, value and test
    const string strVar = strLine.substr(0, stDelim);
    if(strVar.empty()) continue;
    const string strVal = strLine.substr(28, string::npos);
    if(strVal.empty()) continue;
    if(strVar == "Published name")
      { svCurrent.strPublishedName = strVal; continue; }
    else if(strVar == "Driver package provider")
      { svCurrent.strDriverPackageProvider = strVal; continue; }
    else if(strVar == "Class")
      { svCurrent.strClass = strVal; continue; }
    else if(strVar == "Driver date and version")
    { // Separate
      const size_t stDelim = strVal.find(" ");
      if(stDelim == string::npos)
        svCurrent.strDriverDate = svCurrent.strDriverVersion = strVal;
      // Set version
      else svCurrent.strDriverDate = strVal.substr(6, 4)+'-'+
                                     strVal.substr(3, 2)+'-'+
                                     strVal.substr(0, 2),
           svCurrent.strDriverVersion = strVal.substr(stDelim, string::npos);
      // Next item
      continue;
    }
    else if(strVar == "Signer name")
      svCurrent.strSignerName = strVal;
    // Make id
    const string strId = svCurrent.strClass + " - "
                       + svCurrent.strDriverPackageProvider;
    // Last item add to database
    mData[strId][svCurrent.strDriverDate] = svCurrent;
    // Increase total
    ++stTotal;
  } // Close output
  if(fclose(fp)) throw string("Error closing stream input");
  // Show result
  printf(" %u drivers in %u categories.\n", stTotal, mData.size());
  // Enumerate the list
  for(VERSIONDBI iData = mData.begin(); iData != mData.end(); ++iData)
  { // Print category name
    printf("- %s\n", iData->first.c_str());
    // Get reference to data
    VERSIONLIST &vData = iData->second;
    // Print version numbers
    for(VERSIONLISTI iCur = vData.begin(); iCur != vData.end(); ++iCur)
    { // Get reference to data
      const STRVAR &svData = iCur->second;
      // Show item
      printf("  %s\t%s\t%s\n", svData.strPublishedName.c_str(),
                               svData.strDriverDate.c_str(),
                               svData.strDriverVersion.c_str());
    }
  } // Succeeded!
  return 0;
} // Catch errors
catch(const string &strError)
{ // Show error
  fprintf(stderr, "Error: %s!\n", strError.c_str());
  // If there was an errno set? Explain it
  if(errno != 0) fprintf(stderr, "- Reason: %s (%u).\n",
    _sys_errlist[errno], errno);
  // Failed
  return 1;
}
/* == END-OF-FILE ========================================================== */