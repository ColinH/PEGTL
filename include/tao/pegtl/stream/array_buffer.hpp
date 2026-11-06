// Copyright (c) 2022-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_ARRAY_BUFFER_HPP
#define TAO_PEGTL_STREAM_ARRAY_BUFFER_HPP

#include <array>
#include <concepts>
#include <cstddef>
#include <type_traits>
#include <utility>

#include "../config.hpp"
#include "../system.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Data, typename Reader, std::size_t Size, std::size_t Chunk >
   class array_buffer
   {
   public:
      using data_t = Data;

      static_assert( Chunk > 0 );
      static_assert( Chunk < Size );

      template< typename... As >
         requires( ( !std::same_as< array_buffer, std::remove_cvref_t< As > > ) && ... ) && std::constructible_from< Reader, As... >
      explicit array_buffer( As&&... as )
         : m_reader( std::forward< As >( as )... )
      {}

      array_buffer( array_buffer&& ) = delete;
      array_buffer( const array_buffer& ) = delete;

      ~array_buffer() = default;

      void operator=( array_buffer&& ) = delete;
      void operator=( const array_buffer& ) = delete;

      [[nodiscard]] Data* mutable_start() noexcept
      {
         return m_data.data();
      }

      [[nodiscard]] const Data* buffer_start() const noexcept
      {
         return m_data.data();
      }

      [[nodiscard]] std::size_t buffer_capacity() const noexcept
      {
         return Size;
      }

      [[nodiscard]] std::size_t buffer_chunk_size() const noexcept
      {
         return Chunk;
      }

      [[nodiscard]] const Reader& direct_reader() const noexcept
      {
         return m_reader;
      }

   protected:
      std::array< Data, Size > m_data;

      TAO_PEGTL_NO_UNIQUE_ADDRESS Reader m_reader;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
