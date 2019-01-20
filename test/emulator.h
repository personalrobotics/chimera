#include <string>
#include <vector>
#include "chimera/chimera.h"

namespace chimera
{
namespace test
{

class Emulator
{
public:
    Emulator();

    void Run();

    static void Run(const std::string &args);

    static void RunHelp();
    static void RunHelpList();
    static void RunVersion();

    void SetSource(const std::string &filename);
    void SetSources(const std::vector<std::string> &filenames);

    void SetModuleName(const std::string &name);

    void SetBinding(const std::string &name);

    void SetConfigurationFile(const std::string &path);

    static const std::string &GetExamplesDirPath();
    static const std::string &GetBuildPath();

private:
    /// Binding definition name (boost_python/pybind11) for option '-b'
    std::string binding_;

    /// YAML configuration filename for option '-c'
    std::string config_filepath_;

    /// Output top-level module name for option '-m'
    std::string modulename_;

    /// Sources paths
    std::vector<std::string> sources_;
};

} // namespace test
} // namespace chimera
