#pragma once

#include <spdlog/spdlog.h>
#include "spdlog/async.h"
#include "spdlog/sinks/basic_file_sink.h"

namespace NS_Duplex {
namespace NS_Loggings {

inline void initSpdlog() {
    auto fileLogger = spdlog::basic_logger_mt<spdlog::async_factory>("duplexFileLogger", "logs/duplexCore.log");
    
    spdlog::register_logger(fileLogger);
    
    spdlog::set_default_logger(fileLogger);
    
    spdlog::enable_backtrace(32); 
    spdlog::set_level(spdlog::level::debug);
}

}
}
