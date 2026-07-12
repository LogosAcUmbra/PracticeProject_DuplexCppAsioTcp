#include <vector>

#include <boost/asio.hpp>

#include "../shared/include/Loggings/InitSpdlog.hpp"
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
    NS_Loggings::initSpdlog();
    try {


        boost::asio::io_context ioContext;
        TcpServer<SillyConnection<std::vector<char>, ServerSettings::chunkSize>> server(
            ioContext.get_executor(), 
            Protocol::endpoint(Protocol::v6(), ServerSettings::port));
        ioContext.run();

        
    } catch (const std::exception& e) {
        spdlog::critical("--- FAILURE ENCOUNTERED ---");
        spdlog::dump_backtrace();
        spdlog::critical("Unhandled exception caught in main: {}", e.what());
    } catch (...) {
        spdlog::critical("--- FAILURE ENCOUNTERED ---");
        spdlog::dump_backtrace();
        spdlog::critical("Unknown crash caught in main");
    }
    
}