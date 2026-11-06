// Copyright (c) 2022-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_ALLOC_BUFFER_HPP
#define TAO_PEGTL_STREAM_ALLOC_BUFFER_HPP

#include <cassert>
#include <concepts>
#include <cstddef>
#include <memory>
#include <utility>

#include "../config.hpp"
#include "../system.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Data, typename Reader >
   class alloc_buffer
   {
   public:
      using data_t = Data;

      template< typename... As >
         requires std::constructible_from< Reader, As... >
      alloc_buffer( const std::size_t size, const std::size_t chunk, As&&... as )
         : m_size( size ),
           m_chunk( chunk ),
           m_data( new Data[ size ] ),
           m_reader( std::forward< As >( as )... )
      {
         assert( chunk > 0 );
         assert( chunk < size );
      }

      alloc_buffer( alloc_buffer&& ) = delete;
      alloc_buffer( const alloc_buffer& ) = delete;

      ~alloc_buffer() = default;

      void operator=( alloc_buffer&& ) = delete;
      void operator=( const alloc_buffer& ) = delete;

      [[nodiscard]] Data* mutable_start() noexcept
      {
         return m_data.get();
      }

      [[nodiscard]] const Data* buffer_start() const noexcept
      {
         return m_data.get();
      }

      [[nodiscard]] std::size_t buffer_capacity() const noexcept
      {
         return m_size;
      }

      [[nodiscard]] std::size_t buffer_chunk_size() const noexcept
      {
         return m_chunk;
      }

      [[nodiscard]] const Reader& direct_reader() const noexcept
      {
         return m_reader;
      }

   protected:
      const std::size_t m_size;
      const std::size_t m_chunk;
      const std::unique_ptr< Data[] > m_data;

      TAO_PEGTL_NO_UNIQUE_ADDRESS Reader m_reader;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
