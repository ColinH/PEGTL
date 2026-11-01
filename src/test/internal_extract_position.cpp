// Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <type_traits>

#include "test.hpp"

#include <tao/pegtl/internal/extract_position.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   struct test_type_1
   {};

   struct test_type_2
   {
      int m_int = 42;

      [[nodiscard]] const int* current() const noexcept
      {
         return &m_int;
      }

      [[nodiscard]] static test_type_1 current_position() noexcept
      {
         return {};
      }
   };

   struct test_type_3
   {
      int m_int = 42;
      test_type_1 m_pos;

      [[nodiscard]] const int* current() const noexcept
      {
         return &m_int;
      }

      [[nodiscard]] const test_type_1& current_position() const noexcept
      {
         return m_pos;
      }
   };

   struct test_type_4
   {
      [[nodiscard]] static test_type_1 current_position() noexcept
      {
         return {};
      }
   };

   struct test_type_5
   {
      test_type_4 m_tok;

      [[nodiscard]] const test_type_4* current() const noexcept
      {
         return &m_tok;
      }

      [[nodiscard]] static int current_position() noexcept
      {
         return 0;
      }
   };

   struct test_type_6
   {
      test_type_1 m_pos;

      [[nodiscard]] const test_type_1& current_position() const noexcept
      {
         return m_pos;
      }
   };

   struct test_type_7
   {
      test_type_6 m_tok;

      [[nodiscard]] const test_type_6* current() const noexcept
      {
         return &m_tok;
      }

      [[nodiscard]] static int current_position() noexcept
      {
         return 0;
      }
   };

   struct ref_qualified_position
   {
      [[nodiscard]] int current_position() &&;
   };

   static_assert( !internal::has_current_position< test_type_1 > );
   static_assert( internal::has_current_position< const test_type_3& > );
   static_assert( internal::has_current_position< ref_qualified_position > );
   static_assert( !internal::has_current_position< ref_qualified_position& > );

   void unit_test()
   {
      {
         test_type_1 t1;
         auto e1 = internal::extract_position( t1 );
         static_assert( std::is_same_v< decltype( e1 ), test_type_1 > );
         (void)e1;
      }
      {
         test_type_2 t2;
         decltype( auto ) e2 = internal::extract_position( t2 );
         static_assert( std::is_same_v< decltype( e2 ), test_type_1 > );
         (void)e2;
      }
      {
         test_type_3 t3;
         decltype( auto ) e3 = internal::extract_position( t3 );
         static_assert( std::is_same_v< decltype( e3 ), const test_type_1& > );
         (void)e3;
      }
      {
         test_type_5 t5;
         decltype( auto ) e5 = internal::extract_position( t5 );
         static_assert( std::is_same_v< decltype( e5 ), test_type_1 > );
         (void)e5;
      }
      {
         test_type_7 t7;
         decltype( auto ) e7 = internal::extract_position( t7 );
         static_assert( std::is_same_v< decltype( e7 ), const test_type_1& > );
         (void)e7;
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
