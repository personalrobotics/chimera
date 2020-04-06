#include "chimera/logging.h"

namespace chimera
{
namespace logging
{

static std::shared_ptr<Logger> defaultLogger = nullptr;

Log::Log(Level level, const std::string &msg) : level_(level), message_(msg)
{
    // Do nothing
}

std::shared_ptr<Logger> getDefaultLogger()
{
    if (defaultLogger == nullptr)
    {
        defaultLogger = std::make_shared<StdOutLogger>();
    }

    return defaultLogger;
}

void StdOutLogger::log(Level level, const std::string &msg)
{
    //
}

void DebuggingLogger::log(Level level, const std::string& msg)
{
  auto logs = m_log_map[level];
  logs.emplace_back(Log(level, msg));
}

void setDefaultLogger(std::shared_ptr<Logger> logger)
{
    defaultLogger = std::move(logger);
}

void error(const std::string &msg)
{
    //  auto log = Log(Log::ERROR, msg);
  getDefaultLogger()->log(Level::ERROR, msg);
}


} // namespace logging
} // namespace chimera
