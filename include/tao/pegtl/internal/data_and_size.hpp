// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_DATA_AND_SIZE_HPP
#define TAO_PEGTL_INTERNAL_DATA_AND_SIZE_HPP

#include <cstddef>
#include <cstdint>
#include <type_traits>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename Data, typename Size = std::uint8_t >
   class data_and_size
   {
   public:
      static_assert( !std::is_pointer_v< Data > );
      static_assert( !std::is_reference_v< Data > );
      static_assert( !std::is_member_pointer_v< Data > );

      constexpr data_and_size() noexcept = default;  // NOLINT(cppcoreguidelines-pro-type-member-init)

      constexpr data_and_size( const Data d, const Size s ) noexcept
         : m_data( d ),
           m_size( s )
      {}

      [[nodiscard]] constexpr bool empty() const noexcept
      {
         return m_size == 0;
      }

      [[nodiscard]] constexpr Size size() const noexcept
      {
         return m_size;
      }

      [[nodiscard]] constexpr Data data() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept
      {
         return !empty();
      }

   private:
      Data m_data;
      Size m_size = 0;
   };

   template< typename Data >
   class data_and_size< Data, void >
   {
   public:
      static_assert( !std::is_pointer_v< Data > );
      static_assert( !std::is_reference_v< Data > );
      static_assert( !std::is_member_pointer_v< Data > );

      constexpr data_and_size() noexcept = default;

      constexpr explicit data_and_size( const Data* d ) noexcept
         : m_data( d )
      {}

      [[nodiscard]] constexpr bool empty() const noexcept
      {
         return m_data == nullptr;
      }

      [[nodiscard]] constexpr std::size_t size() const noexcept
      {
         return empty() ? 0 : 1;
      }

      [[nodiscard]] constexpr const Data& data() const noexcept
      {
         return *m_data;
      }

      [[nodiscard]] constexpr const Data* pointer() const noexcept
      {
         return m_data;
      }

      [[nodiscard]] constexpr explicit operator bool() const noexcept
      {
         return !empty();
      }

   private:
      const Data* m_data = nullptr;
   };

   template< typename Data >
   data_and_size( const Data* ) -> data_and_size< Data, void >;

   using char_and_size = data_and_size< char, std::uint8_t >;
   using char32_and_size = data_and_size< char32_t, std::uint8_t >;

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
