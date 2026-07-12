#pragma once

#include <boost/system/detail/error_code.hpp>
#include <cassert>
#include <cstring>
#include <memory>
#include <concepts>
#include <utility>

#include <boost/asio.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/socket_base.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>


#include "../TcpSocketImpl.hpp"


namespace NS_Duplex {
namespace NS_Socket {
namespace NS_Client {

namespace TcpClientImpl {

    // namespace Self = TcpClientImpl;
    namespace Super = TcpSocketImpl;
    
    using Super::AsioIpAddress;
    using Super::AsioIpPort;
    using Super::AsioSocketShutdownType;

    using Super::AsioIOExecutor;
    using Super::AsioTcp;
    using Super::AsioTcpEndpoint;
    using Super::AsioTcpSocket;
    using Super::AsioTcpResolver;

    
    // --- socket concept function helpers ---

    using Super::IsBuffer;
    using Super::vIsValidChunkSize;

    template <typename T>
    concept IsTcpClient = Super::IsTcpSocket<T>;

    template <typename TClient> requires IsTcpClient<TClient>
    using BufferOf = typename TClient::Buffer;

    template <typename TClient> requires IsTcpClient<TClient>
    inline consteval std::size_t chunkSizeOf() { return Super::chunkSizeOf<TClient>(); }


    // --- token concept function helpers ---

    template <typename T, typename TClient>
    concept IsConnectToken = 
        std::invocable<T, std::unique_ptr<TClient>, boost::system::error_code>;
    
    using Super::IsWriteToken;
    using Super::IsReadToken;


    // --- core functions ---

    /// this function creates the TClient instance
    template <typename TClient, typename TConnectToken>
        requires IsConnectToken<TConnectToken, TClient>
    inline void asyncConnect(
        const AsioIOExecutor &ioEx, 
        std::string_view host, std::string_view service, 
        TConnectToken &&token
    ) {
        AsioTcpResolver resolver(ioEx);
        resolver.async_resolve(
            host, 
            service, 
            [   &ioEx, 
                token = std::forward<TConnectToken>(token)
            ] (
                const boost::system::error_code &ec, 
                const AsioTcpResolver::results_type &endpts
            ) mutable {
                std::unique_ptr<TClient> uSock(ioEx);
                boost::asio::async_connect(
                    socket = uSock->getSocket(), 
                    endpts, 
                    [   uSock = std::move(uSock),
                        token = std::forward<TConnectToken>(token)
                    ] (
                        const boost::system::error_code &ec,
                        const AsioTcpEndpoint &/*endpt*/
                    ) mutable {
                        std::move(token)(uSock, ec);
                    }
                );
            }
        );
    }
    
    /// this function creates the TClient instance
    template <typename TClient, typename TConnectToken>
        requires IsConnectToken<TConnectToken, TClient>
    inline void asyncConnect(
        const AsioIOExecutor &ioEx, 
        const AsioIpAddress &address, AsioIpPort portNum, 
        TConnectToken &&token
    ) {
        auto uSock = std::make_unique<TClient>(ioEx);
        AsioTcpEndpoint endpt(address, portNum);
        AsioTcpSocket &socket = uSock->getSocket();
        socket.async_connect(
            endpt, 
            [
                uSock = std::move(uSock), 
                token = std::forward<TConnectToken>(token)
            ] ( const boost::system::error_code &ec ) mutable {
                std::move(token)(std::move(uSock), ec);
            }
        );
    }

    // -- core functions from Super --- (rewrite because Self have different restrictions to T than Super)

    template <typename TClient, typename TWriteToken>
        requires IsTcpClient<TClient>
            && IsWriteToken<TWriteToken, TClient>
    inline void asyncWriteMsg(
        std::unique_ptr<TClient> uSock, 
        std::string_view svMsg, 
        TWriteToken &&token
    ) { Super::asyncWriteMsg(std::move(uSock), svMsg, std::forward<TWriteToken>(token)); }

    template <typename TClient, typename TCharContainer, typename TWriteToken>
        requires IsTcpClient<TClient>
            && IsWriteToken<TWriteToken, TClient> 
            && IsCharContainerNonLvalref<TCharContainer>
    inline void asyncWriteMsg(
        std::unique_ptr<TClient> uSock, 
        TCharContainer &&msg,
        TWriteToken &&token
    ) { Super::asyncWriteMsg(std::move(uSock), std::move(msg), std::forward<TWriteToken>(token)); } 

    template <typename TClient, typename TReadToken>
        requires IsTcpClient<TClient>
            && IsReadToken<TReadToken, TClient>
    void asyncRead(
        std::unique_ptr<TClient> uSock, 
        std::size_t numBytesToRead,
        TReadToken &&token
    ) { Super::asyncRead(std::move(uSock), numBytesToRead, std::forward<TReadToken>(token)); }

}; // class TcpClientImpl


} // NS_Client
} // NS_Socket
} // NS_Duplex