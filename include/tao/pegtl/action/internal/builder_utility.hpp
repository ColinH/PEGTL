// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_ACTION_INTERNAL_BUILDER_UTILITY_HPP
#define TAO_PEGTL_ACTION_INTERNAL_BUILDER_UTILITY_HPP

#include <cassert>
#include <cstddef>
#include <optional>
#include <tuple>
#include <type_traits>
#include <utility>
#include <variant>

#include "../../apply_mode.hpp"
#include "../../config.hpp"
#include "../../match.hpp"
#include "../../nothing.hpp"
#include "../../rewind_mode.hpp"
#include "../../type_list.hpp"

#include "../../internal/dependent_false.hpp"

#include "../build_for.hpp"
#include "../require_apply.hpp"
#include "../require_apply0.hpp"
#include "../value_traits.hpp"

#include "single_rule_action.hpp"

namespace TAO_PEGTL_NAMESPACE::internal
{
   template< typename T >
   struct value_slot
   {
      using value_t = T;

      void store( T&& value )
      {
         assert( !stored );
         stored.emplace( std::move( value ) );
      }

      [[nodiscard]] T&& get()
      {
         assert( stored );
         return std::move( *stored );
      }

      std::optional< T > stored;
   };

   template< typename T >
   struct multi_slot
      : value_slot< T >
   {};

   template< typename T >
   struct multi_slot< std::optional< T > >
   {
      using value_t = T;

      void store( T&& value )
      {
         assert( !stored );
         stored.emplace( std::move( value ) );
      }

      [[nodiscard]] std::optional< T >&& get() noexcept
      {
         return std::move( stored );
      }

      std::optional< T > stored;
   };

   template< template< typename... > class Traits, typename T, typename... Args >
   [[nodiscard]] decltype( auto ) make_multi_impl( Args&&... args )
   {
      if constexpr( requires { Traits< T >::make( std::forward< Args >( args )... ); } ) {
         return Traits< T >::make( std::forward< Args >( args )... );
      }
      else {
         return T( std::forward< Args >( args )... );
      }
   }

   template< template< typename... > class Traits, typename T, typename... Args >
   [[nodiscard]] T make_multi( Args&&... args )
   {
      return make_multi_impl< Traits, T >( std::forward< Args >( args )... );
   }

   template< template< typename... > class Traits >
   struct multi
   {
      template< typename T, typename Fields = Traits< T >::fields_t >
      struct builder;

      template< typename T, typename... Fields >
      struct builder< T, type_list< Fields... > >
      {
         using slots_t = std::tuple< multi_slot< Fields >... >;

         static constexpr std::size_t size = sizeof...( Fields );

         template< std::size_t I >
         using part_t = std::tuple_element_t< I, slots_t >::value_t;

         template< std::size_t I, typename ParseInput >
         void put( part_t< I >&& value, const ParseInput& /*unused*/ )
         {
            std::get< I >( slots ).store( std::move( value ) );
         }

         [[nodiscard]] T finish() &&
         {
            return std::apply( []( auto&... slot ) { return make_multi< Traits, T >( slot.get()... ); }, slots );
         }

         slots_t slots;
      };
   };

   template< template< typename... > class Traits >
   struct repeat
   {
      template< typename T >
      struct builder
      {
         using element_t = Traits< T >::element_t;

         static constexpr std::size_t size = 1;

         template< std::size_t I >
         using part_t = element_t;

         template< std::size_t I, typename ParseInput >
         void put( part_t< I >&& value, const ParseInput& in )
         {
            static_assert( I == 0 );
            Traits< T >::add( in, result, std::move( value ) );
         }

         [[nodiscard]] T&& finish() && noexcept
         {
            return std::move( result );
         }

         T result;
      };
   };

   template< typename T >
   struct variant_builder;

   template< typename... Ts >
   struct variant_builder< std::variant< Ts... > >
   {
      using result_t = std::variant< Ts... >;

      static constexpr std::size_t size = sizeof...( Ts );

      template< std::size_t I >
      using part_t = std::variant_alternative_t< I, result_t >;

      template< std::size_t I, typename ParseInput >
      void put( part_t< I >&& value, const ParseInput& /*unused*/ )
      {
         assert( !result );
         result.emplace( std::in_place_index< I >, std::move( value ) );
      }

      [[nodiscard]] result_t&& finish() && noexcept
      {
         assert( result );
         return std::move( *result );
      }

      std::optional< result_t > result;
   };

   template< typename T >
   struct optional_builder;

   template< typename T >
   struct optional_builder< std::optional< T > >
      : value_slot< T >
   {
      static constexpr std::size_t size = 1;

      template< std::size_t I >
      using part_t = T;

      template< std::size_t I, typename ParseInput >
      void put( part_t< I >&& value, const ParseInput& /*unused*/ )
      {
         static_assert( I == 0 );
         this->store( std::move( value ) );
      }

      [[nodiscard]] std::optional< T >&& finish() && noexcept
      {
         return std::move( this->stored );
      }
   };

   template< typename Object, template< typename... > class ObjectAction, typename Output >
   struct create_action
      : maybe_nothing
   {
      using object_t = std::conditional_t< std::is_void_v< Object >, typename Output::value_t, Object >;

      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, Output& out, [[maybe_unused]] States&&... st )
      {
         if constexpr( A == apply_mode::enabled ) {
            object_t object;
            if( Control< Rule >::template match< A, M, ObjectAction, Control >( in, object ) ) {
               out.store( std::move( object ) );
               return true;
            }
            return false;
         }
         else {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         }
      }
   };

   template< typename Rule, typename... Cases >
   struct case_lookup;

   template< typename Rule >
   struct case_lookup< Rule >
   {
      static_assert( dependent_false< Rule >, "rule not in cases" );
   };

   template< typename Rule, typename Case, typename... Cases >
   struct case_lookup< Rule, Case, Cases... >
      : std::conditional_t< std::is_same_v< Rule, typename Case::rule_t >, Case, case_lookup< Rule, Cases... > >
   {};

   template< template< typename... > class Traits >
   struct value_producer
   {
      template< typename Output >
      struct action
         : require_apply
      {
         template< typename ActionInput, typename... States >
         static void apply( const ActionInput& in, Output& out, States&&... /*unused*/ )
         {
            using value_t = Output::value_t;
            out.store( Traits< value_t >::convert( in ) );
         }
      };
   };

   template< auto... Values >
   struct const_producer
   {
      template< typename Output >
      struct action
         : require_apply0
      {
         template< typename... States >
         static void apply0( Output& out, States&&... /*unused*/ )
         {
            using value_t = Output::value_t;
            out.store( value_t( Values... ) );
         }
      };
   };

   template< typename Object, template< typename... > class ObjectAction >
   struct create_producer
   {
      template< typename Output >
      using action = internal::create_action< Object, ObjectAction, Output >;
   };

   template< typename T >
   using as_binding_t = std::conditional_t< requires { typename T::rules_t; }, T, build_for< T, value_producer< value_traits > > >;

   inline constexpr std::size_t no_index = static_cast< std::size_t >( -1 );

   template< typename Rule, typename Binding >
   inline constexpr bool binding_match = type_list_contains_v< Rule, typename Binding::rules_t >;

   template< typename Rule, typename... Bindings >
   struct binding_index;

   template< typename Rule >
   struct binding_index< Rule >
      : std::integral_constant< std::size_t, no_index >
   {};

   template< typename Rule, typename Binding, typename... Bindings >
   struct binding_index< Rule, Binding, Bindings... >
   {
      static constexpr std::size_t next = binding_index< Rule, Bindings... >::value;
      static constexpr std::size_t value = binding_match< Rule, Binding > ? 0 : ( next == no_index ? next : next + 1 );  // NOLINT(readability-avoid-nested-conditional-operator)
   };

   template< typename... Bindings >
   inline constexpr bool bindings_have_unique_rules = types_are_unique_v< type_list_concat_t< typename Bindings::rules_t... > >;

   template< typename Binding, typename Builder, std::size_t I >
   struct capture_action
      : maybe_nothing
   {
      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, Builder& builder, States&&... st )
      {
         if constexpr( A == apply_mode::enabled ) {
            using output_t = value_slot< typename Builder::template part_t< I > >;
            using producer_t = Binding::template producer_t< Rule >;
            using selected_action = producer_t::template action< output_t >;

            output_t output;
            if( Control< Rule >::template match< A, M, internal::single_rule_action< Rule, selected_action >::template type, Control >( in, output, st... ) ) {
               builder.template put< I >( output.get(), in );
               return true;
            }
            return false;
         }
         else {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         }
      }
   };

   template< typename Main, template< typename... > class Action, typename Builder, typename Bindings >
   struct action_router;

   template< typename Main, template< typename... > class Action, typename Builder, typename... Bindings >
   struct action_router< Main, Action, Builder, std::tuple< Bindings... > >
   {
      using bindings_t = std::tuple< Bindings... >;

      template< typename Rule >
      static constexpr std::size_t index = binding_index< Rule, Bindings... >::value;

      template< typename Rule >
      struct type
         : std::conditional_t< std::is_same_v< Rule, Main >, nothing< Rule >, Action< Rule > >
      {};

      template< typename Rule >
         requires( index< Rule > != no_index )
      struct type< Rule >
         : capture_action< std::tuple_element_t< index< Rule >, bindings_t >, Builder, index< Rule > >
      {};
   };

   template< typename Output, template< typename... > class Builder, typename... Bindings >
   struct compose_action
      : maybe_nothing
   {
      using builder_t = Builder< typename Output::value_t >;

      static_assert( sizeof...( Bindings ) == builder_t::size, "arguments must match the constructed value" );
      static_assert( bindings_have_unique_rules< Bindings... >, "arguments must use distinct rules" );

#if defined( __GNUC__ ) && !defined( __clang__ ) && ( __GNUC__ == 16 )
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
#endif

      template< typename Rule,
                apply_mode A,
                rewind_mode M,
                template< typename... > class Action,
                template< typename... > class Control,
                typename ParseInput,
                typename... States >
      [[nodiscard]] static bool match( ParseInput& in, Output& out, States&&... st )
      {
         if constexpr( A == apply_mode::enabled ) {
            static_assert( ( !binding_match< Rule, Bindings > && ... ), "compose must not capture its boundary rule" );

            builder_t builder;
            using router_t = action_router< Rule, Action, builder_t, std::tuple< Bindings... > >;

            if( Control< Rule >::template match< A, M, router_t::template type, Control >( in, builder, st... ) ) {
               out.store( std::move( builder ).finish() );
               return true;
            }
            return false;
         }
         else {
            return TAO_PEGTL_NAMESPACE::match< Rule, A, M, Action, Control >( in, st... );
         }
      }
#if defined( __GNUC__ ) && !defined( __clang__ ) && ( __GNUC__ == 16 )
#pragma GCC diagnostic pop
#endif
   };

   template< template< typename... > class Builder, typename... Args >
   struct composed
   {
      template< typename Output >
      using action = compose_action< Output, Builder, as_binding_t< Args >... >;
   };

   template< template< typename... > class Traits, typename... Args >
   using multi_producer = composed< multi< Traits >::template builder, Args... >;

   template< template< typename... > class Traits, typename Arg >
   using repeat_producer = composed< repeat< Traits >::template builder, Arg >;

   template< typename... Args >
   using variant_producer = composed< variant_builder, Args... >;

   template< typename Arg >
   using optional_producer = composed< optional_builder, Arg >;

}  // namespace TAO_PEGTL_NAMESPACE::internal

#endif
