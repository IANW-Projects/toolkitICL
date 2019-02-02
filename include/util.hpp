/* TODO: Provide a license note */

#ifndef UTIL_H
#define UTIL_H

#if defined(_WIN32)
#include <io.h>
#define access _access_s
#else
#include <unistd.h>
#endif


// macros
#define STRINGIZE_(x) #x
#define STRINGIZE(x) STRINGIZE_(x)

#define ERROR_INFO "Error in line " STRINGIZE(__LINE__) " of " __FILE__ ":\n "


// file system functions
inline bool fileExists(char const* filename)
{
  return access(filename, 0) == 0;
}

inline bool fileExists(std::string const& filename)
{
  return fileExists(filename.c_str());
}


#endif // UTIL_H
