#pragma once

#include <concepts>
#include <ranges>
#include <string_view>
#include <type_traits>

namespace NS_Duplex {
namespace NS_Utils {
namespace NS_CharContainer {

template <typename T>
concept IsCharContainer = 
    std::ranges::forward_range<T>
    && std::same_as<std::remove_cvref_t<std::ranges::range_reference_t<T>>, char>
    // requires (T &t, const T &ct) {
    //     {  t.size() } -> std::convertible_to<std::size_t>;
    //     { ct.size() } -> std::convertible_to<std::size_t>;
    //     {  t.data() } -> std::convertible_to<char*>;
    //     { ct.data() } -> std::convertible_to<const char*>;
    //     { &*( t.begin()) } -> std::convertible_to<char*>;
    //     { &*(ct.begin()) } -> std::convertible_to<const char*>;
    //     { &*( t.end()  ) } -> std::convertible_to<char*>;
    //     { &*(ct.end()  ) } -> std::convertible_to<const char*>;
    // }
    && !std::same_as<std::remove_cvref_t<T>, std::string_view>
    && !std::is_pointer_v<std::decay_t<T>> // char*, char [], /*ref versions*/ (char*)&, char (&)[], /*r-value ref versions*/ std::move(char*), std::move(char []), and all their const versions
;

template <typename T>
concept IsCharContainerNonLvalref = IsCharContainer<T> && std::is_rvalue_reference_v<T&&>;

// normally, u don't need a template type restricted to only l-value reference, think twice
template <typename T>
concept IsCharContainerLvalref = IsCharContainer<T> && std::is_lvalue_reference_v<T>;

} // NS_CharContainer
} // NS_Utils
} // NS_Duplex