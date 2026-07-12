
#include <boost/asio/io_context.hpp>
#include <boost/asio/ip/address.hpp>

#include "../shared/include/Loggings/InitSpdlog.hpp"
#include "../shared/include/Socket/client/SillyClient.hpp"
#include "../shared/include/ServerSettings/ServerSettings.hpp"


using namespace NS_Duplex;

template <typename TBuffer, std::size_t VChunkSize>
using SillyClient = NS_Socket::NS_Client::SillyClient<TBuffer, VChunkSize>;

int main() {
    NS_Loggings::initSpdlog();
    try {
        

        boost::asio::io_context ioContext;
        SillyClient<std::string, ServerSettings::chunkSize>::start(
            ioContext.get_executor(), 
            boost::asio::ip::make_address("127.0.0.1"), // localhost for now
            ServerSettings::port);
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