#pragma once

#include <boost/asio/buffer.hpp>
#include <boost/system/detail/error_code.hpp>
#include <ranges>
#include <string_view>
#include <memory>
#include <concepts>
#include <tuple>
#include <type_traits>

#include <boost/asio.hpp>
#include <spdlog/spdlog.h>
#include <vector>



#include "../utils/CharContainer.hpp"
#include "utils/CharContainer.hpp"

namespace {

    // --- concept alias ---

    template <typename T>
    concept IsCharContainer = NS_Duplex::NS_Utils::NS_CharContainer::IsCharContainer<T>;

    template <typename T>
    concept IsCharContainerNonLvalref = NS_Duplex::NS_Utils::NS_CharContainer::IsCharContainerNonLvalref<T>;

    template <typename T>
    concept IsStaticSizeCharContainer = NS_Duplex::NS_Utils::NS_CharContainer::IsStaticSizeCharContainer<T>;

}

namespace NS_Duplex {
namespace NS_Socket {
    

// template <typename Socket>
// concept HasRequiredFuncForTcpConnectionSocket = 
    
// ;


namespace TcpSocketImpl {

    using AsioIpAddress          = boost::asio::ip::address;
    using AsioIpPort             = boost::asio::ip::port_type;
    
    using AsioSocketShutdownType = boost::asio::socket_base::shutdown_type;
    
    using AsioMutableBuffer = boost::asio::mutable_buffer;
    using AsioConstBuffer = boost::asio::const_buffer;
    
    using AsioTcp = boost::asio::ip::tcp;
    using AsioTcpEndpoint = AsioTcp::endpoint;
    using AsioTcpSocket   = AsioTcp::socket;
    using AsioTcpAcceptor = AsioTcp::acceptor;
    using AsioTcpResolver = AsioTcp::resolver;
    using AsioIOExecutor = AsioTcpSocket::executor_type;

    // --- socket concept-behaving function helpers ---
    
    template <typename T>
    concept IsBuffer = 
        IsCharContainer<T>
        && std::ranges::output_range<T, char>
        && std::ranges::contiguous_range<T>
        && requires (T &t, const char *data, std::size_t size) {
            t.resize(size); // TBuffer must be resizable (and no-limit in capacity)
        }
    ;


    static consteval bool vIsValidChunkSize(std::size_t vChunkSize) {
        return vChunkSize >= 1024;
    }

    template <typename T>
    concept IsTcpSocket = 
        requires {
            typename T::Buffer;
            { T::vChunkSize } -> std::convertible_to<const std::size_t&>;
        } 
        && IsBuffer<typename T::Buffer>
        && vIsValidChunkSize(T::vChunkSize)
        && std::constructible_from<T, AsioIOExecutor>
        && requires (T t, const T ct) {
            {  t.getIOExecutor() } -> std::convertible_to<const AsioIOExecutor&>;
            
            {  t.getSocket() } -> std::convertible_to<AsioTcpSocket&>;
            { ct.getSocket() } -> std::convertible_to<const AsioTcpSocket&>;
            {  t.getBuffer() } -> std::convertible_to<typename T::Buffer&>;
            { ct.getBuffer() } -> std::convertible_to<const typename T::Buffer&>;
        }
    ;

    template <typename TSocket> requires (IsTcpSocket<TSocket>)
    using BufferOf = typename TSocket::Buffer;

    template <typename TSocket> requires (IsTcpSocket<TSocket>)
    static consteval std::size_t chunkSizeOf() { return TSocket::vChunkSize; }

    // --- token concept-behaving function helpers ---

    template <typename T, typename TSocket>
    concept IsWriteToken = std::invocable<T, std::unique_ptr<TSocket>, boost::system::error_code>;

    template <typename T, typename TSocket>
    concept IsReadToken = std::invocable<T, std::unique_ptr<TSocket>, boost::system::error_code, std::size_t>;


    // --- core functions ---

    /**
     * @brief CAUTION: this asyncWriteMsg overload does not copy or handle lifetime of the given msg.
     * 
     * @tparam TSocket: type of socket
     * @tparam TUSockToken: type of token, a function object invocable with (std::unique_ptr<Socket>, boost::system::error_code)
     * @param uSock std::unique_ptr<Socket>
     * @param msg std::string_view
     * @param token can be a one-time function
     * @return void 
     */
    template <typename TSocket, typename TWriteToken>
        requires IsTcpSocket<TSocket>
            && IsWriteToken<TWriteToken, TSocket>
    static void asyncWriteMsg(
        std::unique_ptr<TSocket> uSock, 
        std::string_view svMsg, 
        TWriteToken &&token
    ) {
        spdlog::info("overload 1 (string_view) is used");
        
        auto &socket = uSock->getSocket();
        auto buffer = boost::asio::buffer(svMsg);
        
        boost::asio::async_write(
            socket, 
            buffer, 
            [
                uSock = std::move(uSock),
                token = std::forward<TWriteToken>(token)
            ] (
                boost::system::error_code ec, 
                std::size_t bytesWritten
            ) mutable {
                auto &socket = uSock->getSocket();
                // write-operation succeeded
                if (!ec) {
                    std::move(token)(std::move(uSock), ec);
                    return;
                }
                // The socket was closed or the operation was canceled safely
                if (ec == boost::asio::error::operation_aborted) {
                    // close connection
                    spdlog::warn("unexpected operation_aborted. info: {{"
                        "socket: {{{}}}, action: {{write}}", socket);
                    socket.shutdown(AsioSocketShutdownType::shutdown_both, ec);
                    boost::system::error_code ignoredEc;
                    socket.close(ignoredEc);
                    return;
                }
                // other unexpected error
                spdlog::warn("network error. info: {{"
                        "socket: {{{}}}, action: {{write}}, error_code: {{{}}}}}", socket, ec);
            }
        );
    }

    /**
     * @brief this asyncWriteMsg overload accepts a r-value reference and handle lifetime of the given msg.
     * 
     * @tparam TSocket: type of socket
     * @tparam TCharContainer: type of msg
     * @tparam TWriteToken: type of token, a function object invocable with (std::unique_ptr<Socket>, boost::system::error_code)
     * @param uSock std::unique_ptr<Socket>
     * @param msg TCharContainer &&
     * @param token can be a one-time function
     * @return void
     */
    template <typename TSocket, typename TCharContainer, typename TWriteToken>
    requires IsTcpSocket<TSocket>
        && IsWriteToken<TWriteToken, TSocket> 
        && IsCharContainer<TCharContainer>
    static void asyncWriteMsg(
        std::unique_ptr<TSocket> uSock, 
        TCharContainer &&msg, // universal reference
        TWriteToken &&token
    ) {
        spdlog::info("overload 2 (TCharContainer &&) is used");

        if (msg.size() > chunkSizeOf<TSocket>()) {
            throw std::invalid_argument("not implemented (BufferSequence)");
        }
        BufferOf<TSocket> &uSockBuffer = uSock->getBuffer();
        if constexpr (std::is_lvalue_reference_v<TCharContainer>) {
            if constexpr (std::is_assignable_v<BufferOf<TSocket> &, TCharContainer &>) {
                uSockBuffer = msg;
            } else {
                uSockBuffer.resize(std::ranges::size(msg));
                std::ranges::copy(msg, std::ranges::begin(uSockBuffer));
            }
        } else {
            if constexpr (std::is_assignable_v<BufferOf<TSocket> &, TCharContainer &&>) {
                uSockBuffer = std::move(msg);
            } else {
                uSockBuffer.resize(std::ranges::size(msg));
                std::ranges::move(msg, std::ranges::begin(uSockBuffer));
            }
        }
        asyncWriteMsg<TSocket, TWriteToken>(
            std::move(uSock),
            std::string_view( std::ranges::data(uSockBuffer), std::ranges::size(uSockBuffer) ), 
            std::forward<TWriteToken>(token)
        );
    }


    template <typename TSocket, typename TReadToken>
    requires IsTcpSocket<TSocket>
        && IsReadToken<TReadToken, TSocket>
    static void asyncRead(
        std::unique_ptr<TSocket> uSock, 
        std::size_t numBytesToRead,
        TReadToken &&token
    ) {
        AsioTcpSocket &uSockSocket = uSock->getSocket();
        BufferOf<TSocket> &uSockBuffer = uSock->getBuffer();
        uSockBuffer.resize(numBytesToRead);
        boost::asio::async_read(
            uSockSocket, 
            boost::asio::buffer(uSockBuffer, numBytesToRead),
            [   uSock = std::move(uSock),
                token = std::forward<TReadToken>(token)
            ] (const boost::system::error_code &ec, std::size_t numBytesRead) mutable {
                std::move(token)(std::move(uSock), ec, numBytesRead);
            }
        );
    }
};

} // NS_Socket
} // NS_Duplex