// Copyright (c) 2025-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_STREAM_ITERATOR_READER_HPP
#define TAO_PEGTL_STREAM_ITERATOR_READER_HPP

#include <cstddef>
#include <iterator>
#include <utility>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< std::input_iterator InputIterator, std::sentinel_for< InputIterator > Sentinel = InputIterator >
      requires std::indirectly_copyable< InputIterator, char* >
   class iterator_reader
   {
   public:
      iterator_reader( InputIterator begin, Sentinel end )
         : m_run( std::move( begin ) ),
           m_end( std::move( end ) )
      {}

      [[nodiscard]] std::size_t read( char* buffer, const std::size_t length )
      {
         std::size_t done = 0;

         while( ( m_run != m_end ) && ( done < length ) ) {
            buffer[ done++ ] = *m_run;
            ++m_run;
         }
         return done;
      }

   protected:
      InputIterator m_run;
      Sentinel m_end;
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
