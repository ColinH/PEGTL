// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_SYSTEM_HPP
#define TAO_PEGTL_SYSTEM_HPP

// clang-format off
#if defined( _WIN32 )
#  define TAO_PEGTL_SYSTEM_WINDOWS
#  define TAO_PEGTL_DEFAULT_EOL ascii::scan::lf_crlf
#  define TAO_PEGTL_MMAP_AVAILABLE
#  define TAO_PEGTL_MMAP_INCLUDE "mmap_file_win32.hpp"
#elif defined( __unix__ ) || ( defined( __APPLE__ ) && defined( __MACH__ ) )
#  define TAO_PEGTL_SYSTEM_POSIX
#  define TAO_PEGTL_DEFAULT_EOL ascii::scan::lf
#  include <unistd.h>
#  if defined( _POSIX_MAPPED_FILES )
#    define TAO_PEGTL_MMAP_AVAILABLE
#    define TAO_PEGTL_MMAP_INCLUDE "mmap_file_posix.hpp"
#  endif
#else
#  error "Unknown operating system!"
#endif

#if defined( TAO_PEGTL_SYSTEM_WINDOWS )
#  if !defined( __ORDER_BIG_ENDIAN__ )
#    define __ORDER_BIG_ENDIAN__ 4321
#  endif
#  if !defined( __ORDER_LITTLE_ENDIAN__ )
#    define __ORDER_LITTLE_ENDIAN__ 1234
#  endif
#  if !defined( __BYTE_ORDER__ )
#    define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#  endif

// Windows is always little-endian even when running on bi-endian capable architectures.

#  define TAO_PEGTL_BIG_ENDIAN_TYPE byteswap_endian
#  define TAO_PEGTL_LITTLE_ENDIAN_TYPE identity_endian
#  define TAO_PEGTL_ENDIAN_SUFFIXED( iDeNTiFieR ) iDeNTiFieR##le

#elif !defined( __BYTE_ORDER__ )
#  error "No byte order defined even though we are not on Windows!"
#elif __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__

#  define TAO_PEGTL_BIG_ENDIAN_TYPE identity_endian
#  define TAO_PEGTL_LITTLE_ENDIAN_TYPE byteswap_endian
#  define TAO_PEGTL_ENDIAN_SUFFIXED( iDeNTiFieR ) iDeNTiFieR##be

#elif __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__

#  define TAO_PEGTL_BIG_ENDIAN_TYPE byteswap_endian
#  define TAO_PEGTL_LITTLE_ENDIAN_TYPE identity_endian
#  define TAO_PEGTL_ENDIAN_SUFFIXED( iDeNTiFieR ) iDeNTiFieR##le

#else
#  error "Byte order is neither big nor little endian -- are you using a PDP-11?"
#endif

#if __has_cpp_attribute( msvc::no_unique_address )
#  define TAO_PEGTL_NO_UNIQUE_ADDRESS [[msvc::no_unique_address]]
#elif __has_cpp_attribute( no_unique_address )
#  define TAO_PEGTL_NO_UNIQUE_ADDRESS [[no_unique_address]]
#else
#  define TAO_PEGTL_NO_UNIQUE_ADDRESS
#endif
// clang-format on

// FCOV_EXCL_FILE
#endif
