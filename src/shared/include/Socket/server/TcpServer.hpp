#pragma once

#include <boost/asio.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <spdlog/spdlog.h>


#include "Socket/fmtNetworkSocket.hpp"
#include "Socket/TcpSocketImpl.hpp"
#include "TcpConnectionImpl.hpp"

namespace NS_Duplex {
namespace NS_Socket {
namespace NS_Server {

template <typename TConnSocket>
    requires TcpConnectionImpl::IsTcpConnection<TConnSocket>
class TcpServer {
public:
    using Self = TcpServer;

    using AsioIpAddress = TcpSocketImpl::AsioIpAddress;
    using AsioIpPort    = TcpSocketImpl::AsioIpPort;
    using AsioSocketShutdownType = TcpSocketImpl::AsioSocketShutdownType;
    using AsioTcp           = TcpSocketImpl::AsioTcp;
    using AsioTcpEndpoint   = TcpSocketImpl::AsioTcpEndpoint;
    using AsioTcpSocket     = TcpSocketImpl::AsioTcpSocket;
    using AsioTcpAcceptor   = TcpSocketImpl::AsioTcpAcceptor;
    using AsioTcpResolver   = TcpSocketImpl::AsioTcpResolver;
    using AsioIOExecutor    = TcpSocketImpl::AsioIOExecutor;

    using ConnSocket = TConnSocket;


    explicit TcpServer(const AsioIOExecutor &ioContextEx, const AsioTcpEndpoint &endpt) 
        : m_acceptor(ioContextEx, endpt)
        , m_isAccepting(true)
    {
        asyncAccept();
    }

    void asyncAccept() {
        if (!m_isAccepting) {
            return;
        }
        auto conn = std::make_unique<TConnSocket>(getIOExecutor());
        AsioTcpSocket &connSocket = conn->getSocket();
        m_acceptor.async_accept(
            connSocket, 
            [   this, 
                conn=std::move(conn)
            ] (const boost::system::error_code& ec) mutable {
                TConnSocket::handle(std::move(conn), ec);
                this->asyncAccept();
            }
        );
    }

private:
    AsioTcpAcceptor m_acceptor;
    bool m_isAccepting;

    const AsioIOExecutor &getIOExecutor() { return m_acceptor.get_executor(); }
};


} // NS_Server
} // NS_Socket
} // NS_NS_