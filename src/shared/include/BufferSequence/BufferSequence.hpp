#pragma once

#include <concepts>
#include <stdexcept>
#include <vector>
#include <cstddef>
#include <type_traits>

#include <boost/asio.hpp>


// namespace {
//     template <typename TDerived, std::size_t N>
//     concept HasSubTypeRequiredFunctions = requires(
//         TDerived d, 
//         const char *constCharPtr, char *charPtr, std::size_t size
//     ) {
//         { TDerived( constCharPtr, size ) } -> std::same_as<TDerived>; // constructor
//         { d.writeStaticMetadata(charPtr) } -> std::same_as<std::size_t>;
//     };
// }

template <typename TDerived, std::size_t BUFFER_SIZE>
class BufferSequence {

public:
    using Derived = TDerived;

    // default constructor
    BufferSequence() = delete;
    // copy constructor
    BufferSequence(const BufferSequence<TDerived, BUFFER_SIZE> &other) = delete;
    // copy assignment operator
    BufferSequence &operator=(const BufferSequence<TDerived, BUFFER_SIZE> &other) = delete;

    // move constructor
    BufferSequence(BufferSequence<TDerived, BUFFER_SIZE> &&other)
        : m_data(std::move(other.m_data))
        , m_size(std::move(other.m_size))
        , m_resultBuffers(std::move(other.m_resultBuffers))
        , m_firstStr(std::move(other.m_firstStr))
    {
        std::size_t firstBufferSize = m_resultBuffers[0].size();
        this->m_resultBuffers[0] = boost::asio::buffer(m_firstStr.data(), firstBufferSize);
    }

    // move assignment operator
    BufferSequence &operator=(BufferSequence<TDerived, BUFFER_SIZE> &&other) {
        if (this == &other) {
            return *this;
        }
        this->m_data = std::move(other.m_data);
        this->m_size = std::move(other.m_size);
        this->m_resultBuffers = std::move(other.m_resultBuffers);
        this->m_firstStr = std::move(other.m_firstStr);

        std::size_t firstBufferSize = m_resultBuffers[0].size();
        this->m_resultBuffers[0] = boost::asio::buffer(m_firstStr.data(), firstBufferSize);

        return *this;
    }
    
    // destructor
    constexpr ~BufferSequence() {
        static_assert(
            std::is_base_of_v<BufferSequence<Derived, BUFFER_SIZE>, Derived>,
            "CRTP Error: Subclass must inherit PUBLICLY from BufferSequence!");
        static_assert(
            std::is_convertible_v<Derived*, BufferSequence*>,
            "CRTP Error: Subclass must grant friendship to the BufferSequence Base class!");
        static_assert(
            std::is_constructible_v<Derived, const char*, std::size_t>,
            "CRTP Error, Subclass must implement `constructor(const char *, std::size_t)`" );
        static_assert(
            requires (const Derived& d, char *charPtr) {
                { d.writeStaticMetadata(charPtr) } -> std::same_as<std::size_t>;
            },
            "CRTP Error: Subclass must implement 'constexpr std::size_t writeStaticMetadata(char*) const'" );

    }

    static Derived of(const char *data, std::size_t size) {
        Derived seq(data, size); // subclass must set this class as friend
        seq.makeBuffers();
        return seq;
    }

    std::vector<boost::asio::const_buffer>::const_iterator cbegin() {
        return m_resultBuffers.cbegin();
    }

    std::vector<boost::asio::const_buffer>::const_iterator cend() {
        return m_resultBuffers.cend();
    }

protected:

    const char *m_data;
    std::size_t m_size;
    std::vector<boost::asio::const_buffer> m_resultBuffers;
    std::array<char, BUFFER_SIZE> m_firstStr; // heap allocation

    constexpr BufferSequence(const char *data, std::size_t size)
        : m_data(data)
        , m_size(size)
        , m_resultBuffers()
        , m_firstStr()
    {
        if (size == 0) { throw std::invalid_argument("size of data cannot be 0"); }
    }

private:

    TDerived &self() {
        return static_cast<TDerived &>(*this);
    }
    const TDerived &self() const {
        return static_cast<const TDerived &>(*this);
    }
    
    void makeBuffers() {
        const std::size_t sizeMetadata = writeFirstString();

        std::size_t numBufferNeeded = (sizeMetadata + m_size + BUFFER_SIZE - 1) / BUFFER_SIZE;
        m_resultBuffers.reserve(numBufferNeeded);

        if (numBufferNeeded == 1) {
            const char *constFirstStrPtr = m_firstStr.data();
            m_resultBuffers.emplace_back( boost::asio::buffer(constFirstStrPtr, sizeMetadata + m_size) );
            return;
        }
        std::size_t sizeDataCovered = BUFFER_SIZE - sizeMetadata;
        const char *constFirstStrPtr = m_firstStr.data();
        m_resultBuffers.emplace_back( boost::asio::buffer(constFirstStrPtr, sizeMetadata + sizeDataCovered) );

        std::size_t sizeDataLeft = m_size - sizeDataCovered;
        const char *src = m_data + sizeDataCovered;
        for (int i = 1; i < numBufferNeeded - 1; ++i) {
            m_resultBuffers.emplace_back( boost::asio::buffer(src, BUFFER_SIZE) );
            src += BUFFER_SIZE;
            sizeDataLeft -= BUFFER_SIZE;
        }
        assert(sizeDataLeft < BUFFER_SIZE);
        m_resultBuffers.emplace_back( boost::asio::buffer(src, sizeDataLeft) );
    }

    std::size_t writeFirstString() {
        char *raw = m_firstStr.data();
        std::size_t lenUsed = self().writeStaticMetadata(raw); // subclass must set this class as friend
        lenUsed += writeSize(raw + lenUsed);
        const std::size_t lenMetadata = lenUsed;
        std::size_t spaceLeft = BUFFER_SIZE - lenMetadata;
        if (spaceLeft < m_size) {
            std::memcpy(raw + lenUsed, m_data, spaceLeft);
            return lenMetadata;
        }
        std::memcpy(raw + lenUsed, m_data, m_size);
        return lenMetadata;
    }

    std::size_t writeSize(char *dest) const { // TODO: sizeof(std::size_t) may not be consistent across different hardwares
        constexpr static std::size_t numBytes = sizeof(std::size_t);
        std::memcpy(dest, &(m_size), numBytes);
        dest[numBytes] = ':';
        return numBytes + 1;
    }
};