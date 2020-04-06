#ifndef __CHIMERA_LOGGIN_H__
#define __CHIMERA_LOGGIN_H__

#include <iostream>
#include <unordered_map>
#include <memory>
#include <string>
#include <vector>

namespace chimera
{
namespace logging
{

enum class Level
{
    DEBUG,
    INFO,
    WARN,
    ERROR,
};

class Log
{
public:
    Log(Level level, const std::string &msg);

protected:
    Level level_;
    std::string message_;
};

class Logs
{
public:
    Logs() = default;
};

class Logger
{
public:
    Logger() = default;
    virtual ~Logger() = default;

    virtual void log(Level level, const std::string &msg) = 0;

protected:
};

class StdOutLogger : public Logger
{
public:
    void log(Level level, const std::string &msg) override;

protected:
};

class DebuggingLogger : public Logger
{
public:
    void log(Level level, const std::string &msg) override;

protected:
    std::unordered_map<Level, std::vector<Log>> m_log_map;
};

void setDefaultLogger(std::shared_ptr<Logger> logger);

std::shared_ptr<Logger> getDefaultLogger();

void error();

} // namespace logging
} // namespace chimera

#endif // __CHIMERA_LOGGIN_H__
