// Copyright (c) 2023-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_INPUTS_HPP
#define TAO_PEGTL_STREAM_INPUTS_HPP

#include <cstddef>

#include "../ascii.hpp"
#include "../config.hpp"
#include "../system.hpp"

#include "../internal/input_with_funcs.hpp"
#include "../internal/input_with_lines.hpp"

#include "buffers.hpp"

#include "stream_input.hpp"
#include "stream_input_with_source.hpp"

#include "text_stream_input.hpp"
#include "text_stream_input_with_source.hpp"

#include "stream_input_with_auto_discard.hpp"

namespace TAO_PEGTL_NAMESPACE
{
   using tao_stream_eol = TAO_PEGTL_DEFAULT_EOL;  // Details might change.

   template< typename Buffer, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using stream_input = internal::input_with_funcs< internal::input_with_lines< Eol, internal::stream_input_with_source< Buffer, InputSource, ErrorSource > > >;
   template< typename Buffer, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using stream_auto_input = internal::input_with_funcs< internal::input_with_lines< Eol, internal::stream_input_with_auto_discard< internal::stream_input_with_source< Buffer, InputSource, ErrorSource > > > >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_cstream_input = stream_input< alloc_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_cstring_input = stream_input< alloc_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_istream_input = stream_input< alloc_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using alloc_iterator_input = stream_input< alloc_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_cstream_input = stream_input< array_cstream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_cstring_input = stream_input< array_cstring_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_istream_input = stream_input< array_istream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size, typename Sentinel = InputIterator >
   using array_iterator_input = stream_input< array_iterator_buffer< InputIterator, BufferSize, ChunkSize, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_cstream_input = stream_input< other_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_cstring_input = stream_input< other_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_istream_input = stream_input< other_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using other_iterator_input = stream_input< other_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_cstream_auto_input = stream_auto_input< alloc_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_cstring_auto_input = stream_auto_input< alloc_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_istream_auto_input = stream_auto_input< alloc_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using alloc_iterator_auto_input = stream_auto_input< alloc_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_cstream_auto_input = stream_auto_input< array_cstream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_cstring_auto_input = stream_auto_input< array_cstring_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_istream_auto_input = stream_auto_input< array_istream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size, typename Sentinel = InputIterator >
   using array_iterator_auto_input = stream_auto_input< array_iterator_buffer< InputIterator, BufferSize, ChunkSize, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_cstream_auto_input = stream_auto_input< other_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_cstring_auto_input = stream_auto_input< other_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_istream_auto_input = stream_auto_input< other_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using other_iterator_auto_input = stream_auto_input< other_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Buffer, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using text_stream_input = internal::input_with_funcs< internal::text_stream_input_with_source< Buffer, Eol, InputSource, ErrorSource > >;
   template< typename Buffer, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using text_stream_auto_input = internal::input_with_funcs< internal::stream_input_with_auto_discard< internal::text_stream_input_with_source< Buffer, Eol, InputSource, ErrorSource > > >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_text_cstream_input = text_stream_input< alloc_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_text_cstring_input = text_stream_input< alloc_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_text_istream_input = text_stream_input< alloc_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using alloc_text_iterator_input = text_stream_input< alloc_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_text_cstream_input = text_stream_input< array_cstream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_text_cstring_input = text_stream_input< array_cstring_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_text_istream_input = text_stream_input< array_istream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size, typename Sentinel = InputIterator >
   using array_text_iterator_input = text_stream_input< array_iterator_buffer< InputIterator, BufferSize, ChunkSize, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_text_cstream_input = text_stream_input< other_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_text_cstring_input = text_stream_input< other_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_text_istream_input = text_stream_input< other_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using other_text_iterator_input = text_stream_input< other_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_text_cstream_auto_input = text_stream_auto_input< alloc_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_text_cstring_auto_input = text_stream_auto_input< alloc_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using alloc_text_istream_auto_input = text_stream_auto_input< alloc_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using alloc_text_iterator_auto_input = text_stream_auto_input< alloc_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_text_cstream_auto_input = text_stream_auto_input< array_cstream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_text_cstring_auto_input = text_stream_auto_input< array_cstring_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size >
   using array_text_istream_auto_input = text_stream_auto_input< array_istream_buffer< BufferSize, ChunkSize >, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, std::size_t BufferSize = default_buffer_size, std::size_t ChunkSize = default_chunk_size, typename Sentinel = InputIterator >
   using array_text_iterator_auto_input = text_stream_auto_input< array_iterator_buffer< InputIterator, BufferSize, ChunkSize, Sentinel >, Eol, InputSource, ErrorSource >;

   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_text_cstream_auto_input = text_stream_auto_input< other_cstream_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_text_cstring_auto_input = text_stream_auto_input< other_cstring_buffer, Eol, InputSource, ErrorSource >;
   template< typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource >
   using other_text_istream_auto_input = text_stream_auto_input< other_istream_buffer, Eol, InputSource, ErrorSource >;
   template< typename InputIterator, typename Eol = tao_stream_eol, typename InputSource = void, typename ErrorSource = InputSource, typename Sentinel = InputIterator >
   using other_text_iterator_auto_input = text_stream_auto_input< other_iterator_buffer< InputIterator, Sentinel >, Eol, InputSource, ErrorSource >;

}  // namespace TAO_PEGTL_NAMESPACE

// FCOV_EXCL_FILE
#endif
