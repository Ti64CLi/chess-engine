#ifndef __UTILS_HPP__
#define __UTILS_HPP__

#include <vector>
#include <string>

#define ID(file, rank)    ((file) + 8 * (rank)) // true line (top left is ID(0, 7) and not ID(0, 0))
#define FILE(id)          ((id) % 8) // true column
#define RANK(id)            ((id) / 8) // true line

namespace utils {

const std::vector<std::string> split(const std::string s);
char toLowerCase(char c);
int idFromCaseName(const std::string &caseName);
std::string caseNameFromId(unsigned int id);

}

#endif