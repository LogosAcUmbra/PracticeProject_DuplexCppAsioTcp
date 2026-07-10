#pragma once

#include <boost/asio/buffer.hpp>
#include <cassert>
#include <cstddef>
#include <cstring>

#include <boost/asio.hpp>

#include "BufferSequence.hpp"


template <std::size_t BUFFER_SIZE, bool IS_WIDE>
class MsgBufferSequence : public BufferSequence< MsgBufferSequence<BUFFER_SIZE, IS_WIDE> , BUFFER_SIZE > {

    static_assert(BUFFER_SIZE >= 128);

public:

    using Super = BufferSequence<MsgBufferSequence<BUFFER_SIZE, IS_WIDE>, BUFFER_SIZE>;

    friend Super;

public:

    MsgBufferSequence() = delete;
    MsgBufferSequence(const MsgBufferSequence<BUFFER_SIZE, IS_WIDE> &other) = delete;
    MsgBufferSequence &operator=(const MsgBufferSequence<BUFFER_SIZE, IS_WIDE> &other) = delete;

    MsgBufferSequence(MsgBufferSequence<BUFFER_SIZE, IS_WIDE> &&other)
        : Super(std::move(other)) 
    {}

    static constexpr char DATATYPE_STR[] = "msg:";


private:
    constexpr MsgBufferSequence(const char *data, std::size_t size)
        : Super(data, size)
    {}

    constexpr std::size_t writeStaticMetadata(char *dest) const {
        std::size_t filled = 0;
        std::size_t lenDatatypeStr = sizeof(DATATYPE_STR)-1; // remove '\0'
        if consteval {
            for ( ; filled < lenDatatypeStr; ++filled) {
                dest[filled] = DATATYPE_STR[filled];
            }
        } else {
            filled = 0;
            std::memcpy(dest, DATATYPE_STR, lenDatatypeStr); filled += lenDatatypeStr;
        }
        if constexpr (IS_WIDE) {
            dest[filled++] = 'w';
        }
        dest[filled++] = ':';
        return filled;
    }

};