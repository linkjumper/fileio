#include <fstream>
#include "FileWriter.h"

namespace fileio {
    void write_file(std::string const& filename, std::string const& content) {
        std::ofstream fs;
        fs.exceptions(std::ofstream::failbit|std::ofstream::badbit);
        fs.open(filename);
        fs << content;
    }

    std::string read_file(std::string const& filename) {
        std::ifstream fs;
        fs.exceptions(std::ifstream::failbit|std::ifstream::badbit);
        fs.open(filename);
        return {(std::istreambuf_iterator<char>(fs)), std::istreambuf_iterator<char>()};
    }
}
