#include "emulator.h"

#include <fstream>
#include <iostream>
#include <string>
#include <sys/stat.h>
#include <unistd.h>

namespace chimera
{
namespace test
{

//==============================================================================
bool exists_file(const std::string &name)
{
    std::ifstream f(name.c_str());
    return f.good();
}

//==============================================================================
std::vector<std::string> split(const std::string &buffer,
                               const std::string &separator)
{
    std::string line;
    std::vector<std::string> lines;

    for (std::size_t i = 0; i < buffer.size(); ++i)
    {
        if (buffer.compare(i, separator.size(), separator))
        {
            line.push_back(buffer[i]);
        }
        else
        {
            lines.push_back(line);
            line.resize(0);
        }
    }

    if (line.size())
        lines.push_back(line);

    return lines;
}

//==============================================================================
std::vector<const char *> convertArgs(const std::vector<std::string> &args)
{
    std::vector<const char *> argv;
    for (const auto &arg : args)
        argv.push_back(arg.data());

    return argv;
}

//==============================================================================
Emulator::Emulator()
{
    // Do nothing
}

//==============================================================================
void Emulator::Run()
{
    std::vector<std::string> args;
    args.push_back("");

    args.push_back("-p=" + GetBuildPath());

    if (!binding_.empty())
        args.push_back("-b=" + binding_);

    if (!config_filepath_.empty())
        args.push_back("-c=" + config_filepath_);

    if (!modulename_.empty())
        args.push_back("-m=" + modulename_);

    for (const auto &path : sources_)
    {
        const auto abs_path = GetExamplesDirPath() + path;
        // TODO: assumed path is a relative path
        if (!exists_file(abs_path))
        {
            continue;
        }
        else
        {
            args.push_back(abs_path);
        }
    }

    std::vector<const char *> argv = convertArgs(args);
    chimera::run(static_cast<int>(argv.size()), argv.data());

    exit(0);
}

//==============================================================================
void Emulator::Run(const std::string &args)
{
    std::vector<std::string> args_splits = split(args, " ");
    std::vector<const char *> argv;
    argv.push_back("");
    for (auto &arg : args_splits)
        argv.push_back(arg.data());

    chimera::run(static_cast<int>(argv.size()), argv.data());

    exit(0);
}

//==============================================================================
void Emulator::RunHelp()
{
    int argc = 2;
    std::vector<const char *> argv;
    argv.push_back("");
    argv.push_back("--help");

    chimera::run(argc, argv.data());

    exit(0);
}

//==============================================================================
void Emulator::RunHelpList()
{
    int argc = 2;
    std::vector<const char *> argv;
    argv.push_back("");
    argv.push_back("--help-list");

    chimera::run(argc, argv.data());

    exit(0);
}

//==============================================================================
void Emulator::RunVersion()
{
    int argc = 2;
    std::vector<const char *> argv;
    argv.push_back("");
    argv.push_back("--version");

    chimera::run(argc, argv.data());

    exit(0);
}

//==============================================================================
void Emulator::SetSource(const std::string &filename)
{
    sources_.clear();
    sources_.push_back(filename);
}

//==============================================================================
void Emulator::SetSources(const std::vector<std::string> &filenames)
{
    sources_ = filenames;
}

//==============================================================================
void Emulator::SetModuleName(const std::string &name)
{
    modulename_ = name;
}

//==============================================================================
void Emulator::SetBinding(const std::string &name)
{
    binding_ = name;
}

//==============================================================================
void Emulator::SetConfigurationFile(const std::string &path)
{
    config_filepath_ = GetExamplesDirPath() + path;
}

//==============================================================================
const std::string &Emulator::GetExamplesDirPath()
{
    static auto path = std::string(EXAMPLES_PATH);
    return path;
}

//==============================================================================
const std::string &Emulator::GetBuildPath()
{
    static auto path = std::string(BUILD_PATH);
    return path;
}

} // namespace test
} // namespace chimera
