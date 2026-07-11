
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>
#include "../shared/include/Socket/client/TcpClientImpl.hpp"
#include "../shared/include/Socket/client/SillyClient.hpp"
#include "../shared/include/ServerSettings/ServerSettings.hpp"


using namespace NS_Duplex;

template <typename TBuffer, std::size_t VChunkSize>
using SillyClient = NS_Socket::NS_Client::SillyClient<TBuffer, VChunkSize>;

int main() {
    try {
        boost::asio::io_context ioContext;
        SillyClient<std::string, ServerSettings::chunkSize>::start(
            ioContext.get_executor(), 
            boost::asio::ip::make_address("127.0.0.1"), 
            ServerSettings::port);
        ioContext.run();
    } catch (const std::exception& e) {
        spdlog::critical("Unhandled exception in main: {}", e.what());
    } catch (...) {
        spdlog::critical("Unknown crash occurred!");
    }
}