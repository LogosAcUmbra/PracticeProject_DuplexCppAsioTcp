#pragma once

#include <boost/system/detail/error_code.hpp>
#include <memory>
#include <string_view>
#include <print>

#include "TcpClientImpl.hpp"

namespace {
    namespace Impl = NS_Duplex::NS_Socket::NS_Client::TcpClientImpl;
}

namespace NS_Duplex {
namespace NS_Socket {
namespace NS_Client {



template <typename TBuffer, std::size_t VChunkSize>
    requires Impl::IsBuffer<TBuffer>
        && (Impl::vIsValidChunkSize(VChunkSize))
class SillyClient {
public:

    using Self = SillyClient;

    using AsioIpAddress = Impl::AsioIpAddress;
    using AsioIpPort    = Impl::AsioIpPort;
    using AsioSocketShutdownType = Impl::AsioSocketShutdownType;
    using AsioTcp           = Impl::AsioTcp;
    using AsioTcpEndpoint   = Impl::AsioTcpEndpoint;
    using AsioTcpSocket     = Impl::AsioTcpSocket;
    using AsioTcpResolver   = Impl::AsioTcpResolver;
    using AsioIOExecutor    = Impl::AsioIOExecutor;

    using Buffer = TBuffer;
    static constexpr std::size_t vChunkSize = VChunkSize;

    
    static void start(const AsioIOExecutor &ioContextEx, std::string_view host, std::string_view service) {
        Impl::asyncConnect<Self>(ioContextEx, host, service, afterConnect);
    }

    static void start(const AsioIOExecutor &ioContextEx, const AsioIpAddress &address, AsioIpPort portNum) {
        Impl::asyncConnect<Self>(ioContextEx, address, portNum, afterConnect);
    }

    static void afterConnect(std::unique_ptr<Self> uThis, const boost::system::error_code &ec) {
        if (!ec) {
            spdlog::info("connected");
            Impl::asyncRead(std::move(uThis), 65536, 
                [] (
                    std::unique_ptr<Self> uThis, 
                    const boost::system::error_code &ec, 
                    std::size_t numBytesRead
                ) mutable {
                    Buffer &uThisBuffer = uThis->getBuffer();
                    std::print("msg received: {}", std::string_view(std::ranges::data(uThisBuffer), std::ranges::size(uThisBuffer)));
                
                }
            );
        } else {
            spdlog::warn("connect failed");
        }
    }
    
    inline constexpr SillyClient(const AsioIOExecutor &ioEx)
        : m_socket(ioEx) {
    }

    inline constexpr const AsioIOExecutor&  getIOExecutor() { return m_socket.get_executor(); }
    
    inline constexpr AsioTcpSocket&       getSocket ()       { return m_socket; }
    inline constexpr const AsioTcpSocket& getSocket () const { return m_socket; }
    inline constexpr Buffer&       getBuffer ()       { return m_buffer; }
    inline constexpr const Buffer& getBuffer () const { return m_buffer; }

private:
    AsioTcpSocket m_socket;
    Buffer m_buffer;
};

} // NS_Client
} // NS_Connection
} // NS_Duplex