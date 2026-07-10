#include <boost/asio.hpp>
#include <boost/asio/basic_stream_socket.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <vector>
#include "../shared/include/Socket/server/SillyConnection.hpp"
#include "../shared/include/Socket/server/TcpServer.hpp"
#include "../shared/include/ServerSettings/ServerSettings.hpp"

using Protocol = boost::asio::ip::tcp;

// namespace {
//     template <std::size_t N>
//     using charArray = std::array<char, N>;
//     static_assert(IsTcpConnectHandler<BasicMsgTcpConnection<charArray<1>>>);
// }

using namespace NS_Duplex;

template <typename TConn>
using TcpServer = NS_Socket::NS_Server::TcpServer<TConn>;

template <typename TBuffer, std::size_t vChunkSize>
using SillyConnection = NS_Socket::NS_Server::SillyConnection<TBuffer, vChunkSize>;


int main() {
    try {
        boost::asio::io_context ioContext;
        TcpServer<SillyConnection<std::vector<char>, 65536>> server(
            ioContext.get_executor(), 
            Protocol::endpoint(Protocol::v6(), ServerSettings::port));
        ioContext.run();
    } catch (const std::exception& e) {
        spdlog::critical("Unhandled exception in main: {}", e.what());
    } catch (...) {
        spdlog::critical("Unknown crash occurred!");
    }

    
}