// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_BUFFERS_HPP
#define TAO_PEGTL_STREAM_BUFFERS_HPP

#include <cstdlib>

#include "../config.hpp"

#if !defined TAO_PEGTL_DEFAULT_BUFFER_SIZE
#define TAO_PEGTL_DEFAULT_BUFFER_SIZE 4000
#endif

#if !defined TAO_PEGTL_DEFAULT_CHUNK_SIZE
#define TAO_PEGTL_DEFAULT_CHUNK_SIZE 1000
#endif

#include "cstream_reader.hpp"
#include "cstring_reader.hpp"
#include "istream_reader.hpp"
#include "iterator_reader.hpp"

#include "alloc_buffer.hpp"
#include "array_buffer.hpp"
#include "other_buffer.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   inline constexpr std::size_t default_buffer_size = TAO_PEGTL_DEFAULT_BUFFER_SIZE;
   inline constexpr std::size_t default_chunk_size = TAO_PEGTL_DEFAULT_CHUNK_SIZE;

   using alloc_cstream_buffer = internal::alloc_buffer< char, internal::cstream_reader >;
   using alloc_cstring_buffer = internal::alloc_buffer< char, internal::cstring_reader >;
   using alloc_istream_buffer = internal::alloc_buffer< char, internal::istream_reader >;
   template< typename InputIterator, typename Sentinel = InputIterator >
   using alloc_iterator_buffer = internal::alloc_buffer< char, internal::iterator_reader< InputIterator, Sentinel > >;

   template< std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_cstream_buffer = internal::array_buffer< char, internal::cstream_reader, BufferSize, ChunkSize >;
   template< std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_cstring_buffer = internal::array_buffer< char, internal::cstring_reader, BufferSize, ChunkSize >;
   template< std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_istream_buffer = internal::array_buffer< char, internal::istream_reader, BufferSize, ChunkSize >;
   template< typename InputIterator, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size, typename Sentinel = InputIterator >
   using array_iterator_buffer = internal::array_buffer< char, internal::iterator_reader< InputIterator, Sentinel >, BufferSize, ChunkSize >;

   using other_cstream_buffer = internal::other_buffer< char, internal::cstream_reader >;
   using other_cstring_buffer = internal::other_buffer< char, internal::cstring_reader >;
   using other_istream_buffer = internal::other_buffer< char, internal::istream_reader >;
   template< typename InputIterator, typename Sentinel = InputIterator >
   using other_iterator_buffer = internal::other_buffer< char, internal::iterator_reader< InputIterator, Sentinel > >;

}  // namespace TAO_PEGTL_NAMESPACE

// FCOV_EXCL_FILE
#endif
