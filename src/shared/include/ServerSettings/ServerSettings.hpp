#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/basic_endpoint.hpp>

namespace NS_Duplex {
namespace ServerSettings {

    constexpr boost::asio::ip::port_type port = static_cast<unsigned short>(60000);
    constexpr std::size_t chunkSize = 65536;

}
}
