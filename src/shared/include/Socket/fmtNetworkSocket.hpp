#pragma once

#include <fmt/base.h>
#include <fmt/core.h>
#include <fmt/format.h>

#include <boost/system.hpp>

#include "IsNetworkSocket.hpp"

namespace {
    using boostSysEc = boost::system::error_code;

    // concept alias IsNetworkSocket
    template <typename T>
    concept IsNetworkSocket = NS_Duplex::NS_Socket::IsNetworkSocket<T>;
}

namespace fmt {

// Specialization of fmt::formatter
template<typename NetworkSocket>
    requires IsNetworkSocket<NetworkSocket>
struct fmt::formatter<NetworkSocket> : formatter<string_view> {
    
    // Track format configurations
    char presentation = 'g'; // 'g' = general, 'p' = port only, 'a' = address only

    // Parses format specifiers: "{}", "{:p}", "{:a}"
    constexpr auto parse(format_parse_context& ctx) 
            -> format_parse_context::iterator {
        auto &superSelf = static_cast<formatter<string_view> &>(*this);
        
        format_parse_context::iterator it = ctx.begin(), end = ctx.end();

        if (it != end && (*it == 'g' || *it == 'p' || *it == 'a')) {
            presentation = *it++;
        }
        if (it != end && *it != '}') {
            presentation = 'g'; // back to general when invalid specifier
        }

        ctx.advance_to(it);

        return fmt::formatter<string_view>::parse(ctx);
    }

    template <typename FormatContext>
    auto format(const NetworkSocket& socket, FormatContext& ctx) const {\
        std::string result;
        try {
            auto ep = socket.remote_endpoint();
            auto addr_str = fmt::to_string(ep.address().to_string());
            auto port_val = ep.port();

            result = (presentation == 'a') 
                ? fmt::format("{}", addr_str)
                : ((presentation == 'p') 
                    ? fmt::format("{}", port_val)
                    : fmt::format("[{}]:{}", addr_str, port_val) // Default "general" format: [addr_str]:port_val
                );
            
        } catch (const std::exception& e) {
            result = fmt::format("SocketFormatFailed(Error: {})", e.what());
        }
        return formatter<string_view>::format(result, ctx);
    }
};

template <>
struct fmt::formatter<boostSysEc> : formatter<string_view> {
    // parse is defined by inheritance

    auto format(const boostSysEc &ec, format_context &ctx) const
            -> format_context::iterator {
        return formatter<string_view>::format(ec.to_string(), ctx);
    }
};

} // fmt