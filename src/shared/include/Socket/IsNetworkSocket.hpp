#pragma once

#include <string>
#include <type_traits>
#include <concepts>

#include <boost/asio.hpp>

namespace NS_Duplex {
namespace NS_Socket {

template<typename T>
concept IsNetworkSocket = requires(std::remove_cvref_t<T> t) {
    // must be able to query a remote endpoint without modifying the socket
    { t.remote_endpoint() }; 
    
    // endpoint must expose a port converting to an integer
    { t.remote_endpoint().port() } -> std::integral;
    
    // endpoint must expose an address string representation
    { t.remote_endpoint().address().to_string() } -> std::convertible_to<std::string>;
};

static_assert(IsNetworkSocket<boost::asio::ip::tcp::socket>, 
              "WrongConceptImplementation: Boost socket does NOT satisfy IsNetworkSocket concept!");

} // NS_Socket
} // NS_Duplex