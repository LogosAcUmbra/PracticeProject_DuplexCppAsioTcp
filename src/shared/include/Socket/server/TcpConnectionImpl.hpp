#pragma once

#include <boost/system/detail/error_code.hpp>
#include <cassert>
#include <cstring>
#include <memory>
#include <concepts>
#include <type_traits>
#include <utility>

#include <boost/asio.hpp>
#include <boost/asio/write.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/any_io_executor.hpp>
#include <boost/asio/socket_base.hpp>

#include <fmt/core.h>
#include <fmt/format.h>
#include <spdlog/spdlog.h>


#include "../../utils/CharContainer.hpp"
#include "../TcpSocketImpl.hpp"


namespace NS_Duplex {
namespace NS_Socket {
namespace NS_Server {


namespace TcpConnectionImpl {

    namespace Self = TcpConnectionImpl;
    namespace Super = TcpSocketImpl;
    
    using Super::AsioIpAddress;
    using Super::AsioIpPort;
    using Super::AsioSocketShutdownType;

    using Super::AsioTcp;
    using Super::AsioTcpEndpoint;
    using Super::AsioTcpSocket;
    using Super::AsioTcpAcceptor;
    using Super::AsioTcpResolver;
    using Super::AsioIOExecutor;

    // --- socket concept-behaving function helpers ---

    using Super::IsBuffer;
    using Super::vIsValidChunkSize;

    template <typename T>
    concept IsTcpConnection = 
        Super::IsTcpSocket<T>
        && requires (std::unique_ptr<T> ut, boost::system::error_code ec) { 
            T::handle(std::move(ut), ec);
        }
    ;

    template <typename TConn> requires IsTcpConnection<TConn>
    using BufferOf = typename TConn::Buffer;

    template <typename TConn> requires IsTcpConnection<TConn>
    static consteval std::size_t chunkSizeOf() { return Super::chunkSizeOf<TConn>(); }

    // --- token concept-behaving function helpers ---
    
    using Super::IsWriteToken;
    using Super::IsReadToken;

    // --- core functions ---


    // -- core functions from Super --- (rewrite because Self have different restrictions to T than Super)

    template <typename TConn, typename TWriteToken>
        requires IsTcpConnection<TConn>
            && IsWriteToken<TWriteToken, TConn>
    static inline void asyncWriteMsg(
        std::unique_ptr<TConn> uSock, 
        std::string_view svMsg, 
        TWriteToken &&token
    ) { Super::asyncWriteMsg(std::move(uSock), svMsg, std::forward<TWriteToken>(token)); }

    template <typename TConn, typename TCharContainer, typename TWriteToken>
        requires IsTcpConnection<TConn>
            && IsWriteToken<TWriteToken, TConn> 
            && IsCharContainer<TCharContainer>
    static inline void asyncWriteMsg(
        std::unique_ptr<TConn> uSock, 
        TCharContainer &&msg,
        TWriteToken &&token
    ) { Super::asyncWriteMsg(std::move(uSock), std::move(msg), std::forward<TWriteToken>(token)); } 

    template <typename TClient, typename TReadToken>
        requires IsTcpConnection<TClient>
            && IsReadToken<TReadToken, TClient>
    static void asyncRead(
        std::unique_ptr<TClient> uSock, 
        std::size_t numBytesToRead,
        TReadToken &&token
    ) { Super::asyncRead(std::move(uSock), numBytesToRead, std::forward<TReadToken>(token)); }

}; // class MsgTcpConnectionImpl


} // NS_Server
} // NS_Socket
} // NS_Duplex