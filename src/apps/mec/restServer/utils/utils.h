// @Alessandro Noferi
//

#ifndef __INET_UTILS_H
#define __INET_UTILS_H

#include <vector>
#include <string>
#include <string.h>
#include <stdlib.h>

#ifdef _WIN32
# include <io.h>
# include <stdio.h>
#else // ifdef _WIN32
# include <unistd.h>
#endif // ifdef _WIN32


namespace inet {

namespace utils {

std::vector<std::string> splitString(std::string str, std::string delim);

} // namespace httptools

} // namespace inet

#endif // ifndef __INET_HTTPUTILS_H
