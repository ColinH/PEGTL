// Copyright (c) 2018-2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#include <concepts>
#include <memory>

#include "test.hpp"

#include <tao/pegtl/extra/parse_tree.hpp>

namespace TAO_PEGTL_NAMESPACE
{
   // clang-format off
   struct A : one< 'a' > {};
   struct B : one< 'b' > {};
   struct C : one< 'c' > {};

   struct D : sor< seq< A, B >, seq< A, C > > {};
   struct E : star< A, B > {};
   struct F : seq< E > {};

   struct rematch_head : string< "a\nb" > {};
   struct rematch_a : one< 'a' > {};
   struct rematch_b : one< 'b' > {};
   struct rematch_check : seq< rematch_a, eol, rematch_b, eof > {};
   struct rematch_grammar : rematch< rematch_head, rematch_check > {};

   struct rematch_test_kw : keyword< "test" > {};
   struct rematch_keyword : sor< rematch_test_kw > {};
   struct rematch_identifier : plus< alpha > {};
   struct rematch_identifier_minus_keyword : minus< rematch_identifier, rematch_keyword > {};
   struct rematch_minus_grammar : seq< rematch_identifier_minus_keyword, eof > {};

#if defined( __cpp_exceptions )
   struct D2 : sor< try_catch_return_false< if_must< A, B > >, seq< A, C > > {};
#else
   struct D2 : D {};
#endif
   // clang-format on

   template< typename Rule >
   using selector = parse_tree::selector< Rule,
                                          parse_tree::store_content::on< A, B >,
                                          parse_tree::remove_content::on< C >,
                                          parse_tree::fold_one::on< D > >;

   template< typename Rule >
   using selector2 = parse_tree::selector< Rule,
                                           parse_tree::store_content::on< A >,
                                           parse_tree::discard_empty::on< B >,
                                           parse_tree::discard_empty::on< F >,
                                           parse_tree::fold_one::on< E > >;

   template< typename Rule >
   using rematch_selector = parse_tree::selector< Rule,
                                                  parse_tree::store_content::on< rematch_a, rematch_b > >;

   template< typename Rule >
   using rematch_minus_selector = parse_tree::selector< Rule,
                                                        parse_tree::store_content::on< rematch_test_kw > >;

   template< typename Input >
   class source_rewind_input
      : public Input
   {
   public:
      using data_t = typename Input::data_t;
      using error_position_t = typename Input::error_position_t;
      using rewind_position_t = error_position_t;

      using Input::Input;
      using Input::previous;
      using Input::previous_position;

      [[nodiscard]] auto rewind_position() const
      {
         return Input::current_position();
      }

      [[nodiscard]] const data_t* previous( const rewind_position_t& saved ) const noexcept
      {
         return Input::previous( saved.base() );
      }

      void rewind_to_position( const rewind_position_t& saved ) noexcept
      {
         Input::rewind_to_position( saved.base() );
      }

      [[nodiscard]] auto previous_position( const rewind_position_t& saved ) const
      {
         return saved;
      }
   };

   namespace transform_constraints
   {
      using input = view_input<>;

      struct missing
      {};

      struct transform_with_input
      {
         static int& transform( const input&, std::unique_ptr< int >&, int& ) noexcept;
      };

      struct transform_without_input
      {
         static int& transform( std::unique_ptr< int >&, int& ) noexcept;
      };

      struct both_transforms
         : transform_with_input,
           transform_without_input
      {
         using transform_with_input::transform;
         using transform_without_input::transform;
      };

      template< typename Selector >
      concept has_transform = requires( const input& in, std::unique_ptr< int >& node, int& state ) {
         { parse_tree::internal::transform< Selector >( in, node, state ) } -> std::same_as< void >;
      };

      // Transform results are discarded; absence selects the no-op fallback.
      static_assert( has_transform< transform_with_input > );
      static_assert( has_transform< transform_without_input > );
      static_assert( has_transform< missing > );
      static_assert( !has_transform< both_transforms > );
   }  // namespace transform_constraints

#if defined( __cpp_exceptions )
   template< typename Result >
   struct unwind_node
      : parse_tree::basic_node< unwind_node< Result >, view_input<>::error_position_t >
   {
      template< typename Rule, typename ParseInput >
      Result unwind( const ParseInput& /*unused*/, int& calls )
      {
         ++calls;
         if constexpr( !std::same_as< Result, void > ) {
            return false;
         }
      }
   };
#endif

   void unit_test()
   {
#if defined( __cpp_exceptions )
      {
         view_input<> in( "" );
         int calls = 0;
         TAO_PEGTL_TEST_THROWS( ( void )parse_tree::parse< must< failure >, unwind_node< void > >( in, calls ) );
         TAO_PEGTL_TEST_ASSERT( calls > 0 );
      }
      {
         view_input<> in( "" );
         int calls = 0;
         TAO_PEGTL_TEST_THROWS( ( void )parse_tree::parse< must< failure >, unwind_node< bool > >( in, calls ) );
         TAO_PEGTL_TEST_ASSERT( calls == 0 );
      }
#endif

      {
         text_view_input<> in( "ac" );
         const auto r = parse_tree::parse< D, selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->is_root() );
         TAO_PEGTL_TEST_ASSERT( r->data.empty() );
         TAO_PEGTL_TEST_ASSERT( r->children.size() == 1 );

         const auto& d = r->children.front();
         TAO_PEGTL_TEST_ASSERT( !d->is_root() );
         TAO_PEGTL_TEST_ASSERT( d->is_type< D >() );
         TAO_PEGTL_TEST_ASSERT( d->data.empty() );

         TAO_PEGTL_TEST_ASSERT( d->children.size() == 2 );
         TAO_PEGTL_TEST_ASSERT( d->children.front()->is_type< A >() );
         TAO_PEGTL_TEST_ASSERT( d->children.back()->is_type< C >() );

         text_view_input<> in2( "x" );
         const auto r2 = parse_tree::parse< D, selector >( in2 );
         TAO_PEGTL_TEST_ASSERT( !r2 );
      }

      {
         text_view_input<> in( "aba" );
         const auto r = parse_tree::parse< E, selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->is_root() );
         TAO_PEGTL_TEST_ASSERT( r->data.empty() );

         TAO_PEGTL_TEST_ASSERT( r->children.size() == 2 );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->is_type< A >() );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->is_type< B >() );
      }

      {
         text_view_input<> in( "ab" );
         const auto r = parse_tree::parse< E, selector2 >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->is_root() );
         TAO_PEGTL_TEST_ASSERT( r->data.empty() );

         TAO_PEGTL_TEST_ASSERT( r->children.size() == 1 );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->is_type< A >() );
      }

      {
         text_view_input<> in( "aba" );
         const auto r = parse_tree::parse< F, selector2 >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->is_root() );
         TAO_PEGTL_TEST_ASSERT( r->data.empty() );
         TAO_PEGTL_TEST_ASSERT( r->children.size() == 1 );

         const auto& f = r->children.front();
         TAO_PEGTL_TEST_ASSERT( !f->is_root() );
         TAO_PEGTL_TEST_ASSERT( f->is_type< F >() );
         TAO_PEGTL_TEST_ASSERT( f->data.empty() );
         TAO_PEGTL_TEST_ASSERT( f->children.size() == 1 );

         const auto& a = f->children.front();
         TAO_PEGTL_TEST_ASSERT( !a->is_root() );
         TAO_PEGTL_TEST_ASSERT( a->is_type< A >() );
         TAO_PEGTL_TEST_ASSERT( !a->data.empty() );
         TAO_PEGTL_TEST_ASSERT( a->children.empty() );
      }

      {
         text_view_input<> in( "ac" );
         const auto r = parse_tree::parse< D2, selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->is_root() );
         TAO_PEGTL_TEST_ASSERT( r->data.empty() );

         TAO_PEGTL_TEST_ASSERT( r->children.size() == 2 );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->is_type< A >() );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->is_type< C >() );
      }

      {
         text_view_input<> in( "ac" );
         const auto r = parse_tree::parse< D2 >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->is_root() );
         TAO_PEGTL_TEST_ASSERT( r->data.empty() );
         TAO_PEGTL_TEST_ASSERT( r->children.size() == 1 );

         const auto& d2 = r->children.front();
         TAO_PEGTL_TEST_ASSERT( !d2->is_root() );
         TAO_PEGTL_TEST_ASSERT( d2->is_type< D2 >() );

         TAO_PEGTL_TEST_ASSERT( !d2->data.empty() );
         TAO_PEGTL_TEST_ASSERT( d2->begin.count == 0 );
         TAO_PEGTL_TEST_ASSERT( d2->end.count == 2 );
         TAO_PEGTL_TEST_ASSERT( d2->string() == "ac" );

         const auto& internal = d2->children.front();
         TAO_PEGTL_TEST_ASSERT( !internal->is_root() );
         TAO_PEGTL_TEST_ASSERT( internal->is_type< seq< A, C > >() );

         TAO_PEGTL_TEST_ASSERT( !internal->data.empty() );
         TAO_PEGTL_TEST_ASSERT( internal->begin.count == 0 );
         TAO_PEGTL_TEST_ASSERT( internal->end.count == 2 );
         TAO_PEGTL_TEST_ASSERT( internal->data == "ac" );

         TAO_PEGTL_TEST_ASSERT( internal->children.size() == 2 );
         TAO_PEGTL_TEST_ASSERT( internal->children.front()->is_type< A >() );
         TAO_PEGTL_TEST_ASSERT( internal->children.back()->is_type< C >() );
      }

      {
         std::string data( "a\nb" );
         text_view_input< ascii::scan::lf, char, std::string, std::string > in( "source", data );
         const auto r = parse_tree::parse< rematch_grammar, rematch_selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->children.size() == 2 );

         const auto& a = r->children.front();
         TAO_PEGTL_TEST_ASSERT( a->is_type< rematch_a >() );
         TAO_PEGTL_TEST_ASSERT( a->data == "a" );
         TAO_PEGTL_TEST_ASSERT( a->begin.source == "source" );
         TAO_PEGTL_TEST_ASSERT( a->begin == text_position_with_source( "source", text_position( 1, 1, 0 ) ) );
         TAO_PEGTL_TEST_ASSERT( a->end == text_position_with_source( "source", text_position( 1, 2, 1 ) ) );

         const auto& b = r->children.back();
         TAO_PEGTL_TEST_ASSERT( b->is_type< rematch_b >() );
         TAO_PEGTL_TEST_ASSERT( b->data == "b" );
         TAO_PEGTL_TEST_ASSERT( b->begin == text_position_with_source( "source", text_position( 2, 1, 2 ) ) );
         TAO_PEGTL_TEST_ASSERT( b->end == text_position_with_source( "source", text_position( 2, 2, 3 ) ) );
      }

      {
         using input_t = source_rewind_input< text_view_input< ascii::scan::lf, char, std::string, std::string > >;

         std::string data( "a\nb" );
         input_t in( "source", data );
         const auto r = parse_tree::parse< rematch_grammar, rematch_selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->children.size() == 2 );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->data == "a" );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->begin == text_position_with_source( "source", text_position( 1, 1, 0 ) ) );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->end == text_position_with_source( "source", text_position( 1, 2, 1 ) ) );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->data == "b" );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->begin == text_position_with_source( "source", text_position( 2, 1, 2 ) ) );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->end == text_position_with_source( "source", text_position( 2, 2, 3 ) ) );
      }

      {
         std::string data( "a\nb" );
         view_input< ascii::scan::lf, char, std::string, std::string > in( "source", data );
         const auto r = parse_tree::parse< rematch_grammar, rematch_selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->children.size() == 2 );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->data == "a" );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->begin.count == 0 );
         TAO_PEGTL_TEST_ASSERT( r->children.front()->end.count == 1 );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->data == "b" );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->begin.count == 2 );
         TAO_PEGTL_TEST_ASSERT( r->children.back()->end.count == 3 );
      }

      {
         std::string data( "content" );
         text_view_input< ascii::scan::lf, char, std::string, std::string > in( "source", data );
         const auto r = parse_tree::parse< rematch_minus_grammar, rematch_minus_selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->children.empty() );
      }

      {
         std::string data( "testing" );
         text_view_input< ascii::scan::lf, char, std::string, std::string > in( "source", data );
         const auto r = parse_tree::parse< rematch_minus_grammar, rematch_minus_selector >( in );
         TAO_PEGTL_TEST_ASSERT( r );
         TAO_PEGTL_TEST_ASSERT( r->children.empty() );
      }

      {
         std::string data( "test" );
         text_view_input< ascii::scan::lf, char, std::string, std::string > in( "source", data );
         TAO_PEGTL_TEST_ASSERT( !parse_tree::parse< rematch_minus_grammar, rematch_minus_selector >( in ) );
      }
   }

}  // namespace TAO_PEGTL_NAMESPACE

#include "main.hpp"
