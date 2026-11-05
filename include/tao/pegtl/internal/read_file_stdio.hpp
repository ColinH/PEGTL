// Copyright (c) 2014-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_INTERNAL_READ_FILE_STDIO_HPP
#define TAO_PEGTL_INTERNAL_READ_FILE_STDIO_HPP

#include <cstdio>
#include <exception>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>

#include "../config.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   [[nodiscard]] inline std::FILE* read_file_open( const std::filesystem::path& path )
   {
      errno = 0;
#if defined( _MSC_VER ) || defined( __MINGW32__ )
      std::FILE* file;
      if( ::_wfopen_s( &file, path.c_str(), L"rb" ) == 0 ) {
         return file;
      }
#if defined( __cpp_exceptions )
      const std::error_code ec( errno, std::system_category() );
      throw std::filesystem::filesystem_error( "_wfopen_s() failed", path, ec );
#else
      std::perror( "_wfopen_s() failed" );
      std::terminate();
#endif
#else
      if( auto* file = std::fopen( path.c_str(), "rbe" ) ) {
         return file;
      }
#if defined( __cpp_exceptions )
      const std::error_code ec( errno, std::system_category() );
      throw std::filesystem::filesystem_error( "std::fopen() failed", path, ec );
#else
      std::perror( "std::fopen() failed" );
      std::terminate();
#endif
#endif
   }

   struct read_file_close
   {
      void operator()( FILE* f ) const noexcept
      {
         (void)std::fclose( f );
      }
   };

   class read_file_stdio
   {
   public:
      explicit read_file_stdio( const std::filesystem::path& path )
         : read_file_stdio( read_file_open( path ), path )
      {}

      read_file_stdio( FILE* file, const std::filesystem::path& path )  // NOLINT(modernize-pass-by-value)
         : m_path( path ),
           m_file( file )
      {}

      read_file_stdio( const read_file_stdio& ) = delete;
      read_file_stdio( read_file_stdio&& ) = delete;

      ~read_file_stdio() = default;

      read_file_stdio& operator=( const read_file_stdio& ) = delete;
      read_file_stdio& operator=( read_file_stdio&& ) = delete;

      [[nodiscard]] std::size_t size() const
      {
         errno = 0;
         if( std::fseek( m_file.get(), 0, SEEK_END ) != 0 ) {
            // LCOV_EXCL_START
#if defined( __cpp_exceptions )
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::fseek() failed [SEEK_END]", m_path, ec );
#else
            std::perror( "std::fseek() failed [SEEK_END]" );
            std::terminate();
#endif
            // LCOV_EXCL_STOP
         }
         errno = 0;
         const auto s = std::ftell( m_file.get() );
         if( s < 0 ) {
            // LCOV_EXCL_START
#if defined( __cpp_exceptions )
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::ftell() failed", m_path, ec );
#else
            std::perror( "std::ftell() failed" );
            std::terminate();
#endif
            // LCOV_EXCL_STOP
         }
         errno = 0;
         if( std::fseek( m_file.get(), 0, SEEK_SET ) != 0 ) {
            // LCOV_EXCL_START
#if defined( __cpp_exceptions )
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::fseek() failed [SEEK_SET]", m_path, ec );
#else
            std::perror( "std::fseek() failed [SEEK_SET]" );
            std::terminate();
#endif
            // LCOV_EXCL_STOP
         }
         return static_cast< std::size_t >( s );
      }

      [[nodiscard]] std::string read_string() const
      {
         std::string nrv;
         if( const std::size_t s = size(); s > 0 ) {
#if defined ( __cpp_exceptions )
            std::exception_ptr ep;

            nrv.resize_and_overwrite( s, [ this, &ep ]( char* buffer, const std::size_t length ) -> std::size_t {
               try {
                  read_impl( buffer, length );
                  return length;
               }
               catch( ... ) {
                  ep = std::current_exception();
                  return 0;
               }
            });

            if( ep ) {
               std::rethrow_exception( ep );
            }
#else
            nrv.resize_and_overwrite( s, [ this ]( char* buffer, const std::size_t length ) {
               read_impl( buffer, length );
               return length;
            });
#endif
         }
         return nrv;
      }

   private:
      const std::filesystem::path m_path;
      const std::unique_ptr< std::FILE, read_file_close > m_file;

      void read_impl( void* buffer, const std::size_t length ) const
      {
         errno = 0;
         if( std::fread( buffer, length, 1, m_file.get() ) != 1 ) {
            // LCOV_EXCL_START
#if defined( __cpp_exceptions )
            const std::error_code ec( errno, std::system_category() );
            throw std::filesystem::filesystem_error( "std::fread() failed", m_path, ec );
#else
            std::perror( "std::fread() failed" );
            std::terminate();
#endif
            // LCOV_EXCL_STOP
         }
      }
   };

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
