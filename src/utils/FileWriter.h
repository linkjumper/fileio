#include <string>

namespace fileio {
    void        write_file(std::string const& filename, std::string const& content);
    std::string read_file(std::string const& filename);
}
