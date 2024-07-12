#include "include/utils.hpp"
#include <vector>
#include <string>
#include <sstream>

namespace utils {

const std::vector<std::string> split(const std::string s) {
    std::vector<std::string> splitString;
    std::stringstream ss(s);
    std::string word;

    while(ss >> word) {
        splitString.push_back(word);
    }

    return splitString;
}

char toLowerCase(char c) {
    char res = c;

    if ('A' <= c && c <= 'Z') {
        res = c - 'A' + 'a';
    }

    return res;
}

int idFromCaseName(const std::string &caseName) {
    char file = toLowerCase(caseName[0]), rank = caseName[1];

    if (!('a' <= file && file <= 'h')) {
        return -1;
    }

    if (!('1' <= rank && rank <= '8')) {
        return -1;
    }

    return ID(file - 'a', rank - '1');
}

std::string caseNameFromId(unsigned int id) {
    std::string caseName;

    caseName += 'a' + FILE(id);
    caseName += '1' + RANK(id);

    return caseName;
}

}