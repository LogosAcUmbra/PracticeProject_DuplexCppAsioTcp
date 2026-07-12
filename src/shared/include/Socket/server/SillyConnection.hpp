#pragma once

#include "TcpConnectionImpl.hpp"
#include <boost/system/detail/error_code.hpp>


namespace {
    namespace Impl = NS_Duplex::NS_Socket::NS_Server::TcpConnectionImpl;
}

namespace NS_Duplex {
namespace NS_Socket {
namespace NS_Server {

template <typename TBuffer, std::size_t VChunkSize>
    requires Impl::IsBuffer<TBuffer>
        && (Impl::vIsValidChunkSize(VChunkSize))
class SillyConnection {
public:

    using Self = SillyConnection;

    using AsioIpAddress = Impl::AsioIpAddress;
    using AsioIpPort    = Impl::AsioIpPort;
    using AsioSocketShutdownType = Impl::AsioSocketShutdownType;
    using AsioTcp           = Impl::AsioTcp;
    using AsioTcpEndpoint   = Impl::AsioTcpEndpoint;
    using AsioTcpSocket     = Impl::AsioTcpSocket;
    using AsioTcpAcceptor   = Impl::AsioTcpAcceptor;
    using AsioTcpResolver   = Impl::AsioTcpResolver;
    using AsioIOExecutor    = Impl::AsioIOExecutor;

    using Buffer = TBuffer;
    static constexpr std::size_t vChunkSize = VChunkSize;

public:

    explicit SillyConnection(AsioIOExecutor ioEx) : m_socket(ioEx) {}

    static void handle(std::unique_ptr<Self> uThis, boost::system::error_code ec) {
        if (!ec) {
            spdlog::info("socket connected, Info: {{ socket info: {{{}}} }}", uThis->m_socket);
            asyncSayHi(std::move(uThis),
                [] (std::unique_ptr<Self> uThis) {}
            );
        } else {
            spdlog::warn("socket connection failed, Info: {{ socket info: {{{}}}, ec: {{{}}} }}", uThis->m_socket, ec);
        }
    }

private:
    template <typename TUptrToken>
    requires std::invocable<TUptrToken, std::unique_ptr<Self>>
    static void asyncSayHi(
        std::unique_ptr<Self> uThis, 
        TUptrToken &&token
    ) {
        Impl::asyncWriteMsg(
            std::move(uThis), 
            fmt::format("Hello World! I am the Server and the connection is built! My size_t is using {} bytes", sizeof(std::size_t)), 
            [token = std::forward<TUptrToken>(token)] (std::unique_ptr<Self> uThis, const boost::system::error_code &ec) mutable {
                std::move(token)(std::move(uThis));
            }
        );
    }

public:
    // SillyConnection() {}


    inline constexpr const AsioIOExecutor&  getIOExecutor() { return m_socket.get_executor(); }
    
    inline constexpr AsioTcpSocket&       getSocket ()       { return m_socket; }
    inline constexpr const AsioTcpSocket& getSocket () const { return m_socket; }
    inline constexpr Buffer&       getBuffer ()       { return m_buffer; }
    inline constexpr const Buffer& getBuffer () const { return m_buffer; }

private:
    AsioTcpSocket m_socket;
    Buffer m_buffer;
};

} // NS_Server
} // NS_Socket
} // NS_Duplex
