// Copyright (c) 2026 Dr. Colin Hirsch and Daniel Frey
// Distributed under the Boost Software License, Version 1.0.
// (See accompanying file LICENSE_1_0.txt or copy at https://www.boost.org/LICENSE_1_0.txt)

#ifndef TAO_PEGTL_SRC_EXAMPLE_OPEN_METRICS_CHECK_HPP
#define TAO_PEGTL_SRC_EXAMPLE_OPEN_METRICS_CHECK_HPP

#include <cmath>
#include <cstddef>
#include <iostream>
#include <map>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <vector>

#include "open_metrics_model.hpp"

namespace open_metrics
{
   namespace internal
   {
      class check_impl
      {
      public:
         void check( const metric_set& metrics )
         {
            for( const auto& entry : metrics.metric_families ) {
               validate_family( entry.second );
            }
            validate_common_labels();
         }

      private:
         std::map< std::string, std::string > sample_names;
         std::vector< const label_set* > metric_labels;

         static void error( const std::string& where, const std::string& message )
         {
            std::cerr << "OpenMetrics validation error: " << where << ": " << message << '\n';
         }

         static void warning( const std::string& where, const std::string& message )
         {
            std::cerr << "OpenMetrics validation warning: " << where << ": " << message << '\n';
         }

         [[nodiscard]] static std::string family_location( const std::string& name )
         {
            return "MetricFamily \"" + name + '"';
         }

         [[nodiscard]] static std::string metric_location( const std::string& family, const std::size_t metric )
         {
            return family_location( family ) + ", Metric " + std::to_string( metric );
         }

         [[nodiscard]] static std::string point_location( const std::string& family, const std::size_t metric, const std::size_t point )
         {
            return metric_location( family, metric ) + ", MetricPoint " + std::to_string( point );
         }

         [[nodiscard]] static std::string number_string( const double value )
         {
            if( std::isnan( value ) ) {
               return "NaN";
            }
            if( std::isinf( value ) ) {
               return std::signbit( value ) ? "-Inf" : "+Inf";
            }
            return std::to_string( value );
         }

         [[nodiscard]] static bool same_label_names( const label_set& lhs, const label_set& rhs )
         {
            if( lhs.size() != rhs.size() ) {
               return false;
            }
            auto l = lhs.begin();
            auto r = rhs.begin();
            while( l != lhs.end() ) {
               if( l->first != r->first ) {
                  return false;
               }
               ++l;
               ++r;
            }
            return true;
         }

         [[nodiscard]] static bool is_snake_case( const std::string& name )
         {
            for( const char c : name ) {  // NOLINT(readability-use-anyofallof)
               if( ( c < 'a' || c > 'z' ) && ( c < '0' || c > '9' ) && ( c != '_' ) ) {
                  return false;
               }
            }
            return true;
         }

         [[nodiscard]] static bool is_non_negative_integer( const double value )
         {
            return std::isfinite( value ) && ( value >= 0.0 ) && ( std::trunc( value ) == value );
         }

         static void validate_labels( const std::string& where, const label_set& labels )
         {
            for( const auto& [ name, value ] : labels ) {
               if( name.starts_with( '_' ) ) {
                  error( where, "reserved label name \"" + name + '"' );
               }
               if( value.empty() ) {
                  warning( where, "empty value for label \"" + name + "\" should be treated as absent" );
               }
            }
         }

         void add_sample_name( const std::string& family, const std::string_view suffix )
         {
            std::string name = family;
            name += suffix;
            const auto [ i, inserted ] = sample_names.emplace( name, family );
            if( !inserted && ( i->second != family ) ) {
               error( family_location( family ), "sample name \"" + name + "\" clashes with MetricFamily \"" + i->second + '"' );
            }
         }

         void validate_sample_names( const std::string& family, const metric_type type )
         {
            switch( type ) {
               case metric_type::unknown:
               case metric_type::gauge:
               case metric_type::stateset:
                  add_sample_name( family, "" );
                  break;
               case metric_type::counter:
                  add_sample_name( family, "_total" );
                  add_sample_name( family, "_created" );
                  break;
               case metric_type::info:
                  add_sample_name( family, "_info" );
                  break;
               case metric_type::histogram:
                  add_sample_name( family, "_bucket" );
                  add_sample_name( family, "_count" );
                  add_sample_name( family, "_sum" );
                  add_sample_name( family, "_created" );
                  break;
               case metric_type::gaugehistogram:
                  add_sample_name( family, "_bucket" );
                  add_sample_name( family, "_gcount" );
                  add_sample_name( family, "_gsum" );
                  break;
               case metric_type::summary:
                  add_sample_name( family, "" );
                  add_sample_name( family, "_count" );
                  add_sample_name( family, "_sum" );
                  add_sample_name( family, "_created" );
                  break;
            }
         }

         void validate_family( const metric_family& family )  // NOLINT(readability-convert-member-functions-to-static)
         {
            const std::string where = family_location( family.name );

            if( family.name.starts_with( '_' ) ) {
               error( where, "MetricFamily names beginning with an underscore are reserved" );
            }
            if( family.name.contains( ':' ) ) {
               warning( where, "colons are reserved for names produced by monitoring systems" );
            }
            if( !is_snake_case( family.name ) ) {
               warning( where, "MetricFamily name is not snake_case" );
            }

            if( family.unit && !family.unit->empty() ) {
               const std::string suffix = '_' + *family.unit;
               if( !family.name.ends_with( suffix ) ) {
                  error( where, "Unit \"" + *family.unit + "\" is not a suffix of the MetricFamily name" );
               }
            }

            std::visit( [ & ]( const auto& data ) { validate_family_data( family, data ); }, family.data );
         }

         template< typename Data >
         void validate_family_data( const metric_family& family, const Data& data )
         {
            const std::string where = family_location( family.name );
            if constexpr( ( Data::type == metric_type::stateset ) || ( Data::type == metric_type::info ) ) {
               if( family.unit && !family.unit->empty() ) {
                  error( where, "StateSet and Info MetricFamilies must have an empty Unit" );
               }
            }
            if constexpr( Data::type == metric_type::unknown ) {
               warning( where, "the Unknown metric type should only be used when the type cannot be determined" );
            }
            validate_sample_names( family.name, Data::type );

            for( std::size_t i = 0; i < data.metrics.size(); ++i ) {
               const auto& metric = data.metrics.at( i );
               const std::string metric_where = metric_location( family.name, i );

               validate_labels( metric_where, metric.labels );
               metric_labels.push_back( &metric.labels );

               if( i != 0 && !same_label_names( data.metrics.front().labels, metric.labels ) ) {
                  warning( metric_where, "label names differ from the first Metric in the MetricFamily" );
               }
               for( std::size_t j = 0; j < i; ++j ) {
                  if( data.metrics.at( j ).labels == metric.labels ) {
                     error( metric_where, "duplicate Metric LabelSet" );
                  }
               }

               if constexpr( ( Data::type == metric_type::histogram ) || ( Data::type == metric_type::gaugehistogram ) ) {
                  if( metric.labels.contains( "le" ) ) {
                     error( metric_where, "Histogram Metric labels must not contain \"le\"" );
                  }
               }
               if constexpr( Data::type == metric_type::summary ) {
                  if( metric.labels.contains( "quantile" ) ) {
                     error( metric_where, "Summary Metric labels must not contain \"quantile\"" );
                  }
               }
               if constexpr( Data::type == metric_type::stateset ) {
                  if( metric.labels.contains( family.name ) ) {
                     error( metric_where, "StateSet Metric labels must not contain the MetricFamily name" );
                  }
               }
               validate_metric( family.name, i, metric );
            }
         }

         template< typename Value >
         void validate_metric( const std::string& family, const std::size_t metric_index, const metric< Value >& metric )
         {
            const std::string where = metric_location( family, metric_index );
            if( metric.metric_points.empty() ) {
               error( where, "Metric must contain at least one MetricPoint" );
               return;
            }

            bool has_explicit_timestamp = false;
            std::optional< double > previous_timestamp;
            for( std::size_t i = 0; i < metric.metric_points.size(); ++i ) {
               const auto& point = metric.metric_points.at( i );
               const std::string point_where = point_location( family, metric_index, i );

               if( point.timestamp ) {
                  has_explicit_timestamp = true;
                  if( !std::isfinite( *point.timestamp ) ) {
                     error( point_where, "timestamp must be finite" );
                  }
                  else {
                     if( previous_timestamp && ( *point.timestamp <= *previous_timestamp ) ) {
                        error( point_where, "MetricPoint timestamps must be monotonically increasing" );
                     }
                     previous_timestamp = point.timestamp;
                  }
               }
               else if( metric.metric_points.size() > 1 ) {
                  error( point_where, "multiple MetricPoints require timestamps" );
               }
               validate_value( point_where, point.value );
            }
            if( has_explicit_timestamp ) {
               warning( where, "MetricPoints should not have explicit timestamps" );
            }

            if constexpr( std::is_same_v< Value, counter_value > ) {
               validate_counter_sequence( family, metric_index, metric );
            }
         }

         static void validate_created( const std::string& where, const std::optional< double >& created, const bool recommended )
         {
            if( created ) {
               if( !std::isfinite( *created ) ) {
                  error( where, "Created timestamp must be finite" );
               }
            }
            else if( recommended ) {
               warning( where, "a Created timestamp is recommended" );
            }
         }

         static void validate_value( const std::string& /*unused*/, const unknown_value& /*unused*/ ) {}

         static void validate_value( const std::string& /*unused*/, const gauge_value& /*unused*/ ) {}

         static void validate_value( const std::string& where, const counter_value& value )
         {
            if( std::isnan( value.total ) || ( value.total < 0.0 ) ) {
               error( where, "Counter Total must be non-NaN and non-negative" );
            }
            validate_created( where, value.created, true );
         }

         static void validate_value( const std::string& where, const state_set_value& value )
         {
            if( value.states.empty() ) {
               error( where, "StateSet MetricPoint must contain at least one state" );
            }
         }

         static void validate_value( const std::string& where, const info_value& value )
         {
            validate_labels( where + ", Info labels", value.labels );
         }

         struct bucket_validation
         {
            bool has_negative_threshold = false;
            std::optional< double > positive_infinity_count;
         };

         template< typename Value >
         [[nodiscard]] static bucket_validation validate_buckets( const std::string& where, const Value& value )
         {
            bucket_validation result;
            if( value.buckets.empty() ) {
               error( where, "MetricPoint must contain at least one bucket" );
            }

            std::optional< double > previous_count;
            for( const auto& [ threshold, count ] : value.buckets ) {
               const std::string bucket_where = where + ", bucket " + number_string( threshold );

               if( std::isnan( threshold ) ) {
                  error( bucket_where, "bucket threshold must not be NaN" );
               }
               else {
                  result.has_negative_threshold = result.has_negative_threshold || ( threshold < 0.0 );
                  if( std::isinf( threshold ) && !std::signbit( threshold ) ) {
                     result.positive_infinity_count = count;
                  }
               }
               if( !is_non_negative_integer( count ) ) {
                  error( bucket_where, "bucket count must be a non-negative integer" );
                  previous_count.reset();
               }
               else {
                  if( previous_count && ( count < *previous_count ) ) {
                     error( bucket_where, "bucket counts must be cumulative" );
                  }
                  previous_count = count;
               }
            }
            if( !result.positive_infinity_count ) {
               error( where, "MetricPoint must contain a +Inf bucket" );
            }
            return result;
         }

         static void validate_count( const std::string& where, const char* name, const std::optional< double >& count )
         {
            if( count && !is_non_negative_integer( *count ) ) {
               error( where, std::string( name ) + " must be a non-negative integer" );
            }
         }

         static void validate_count_relationship( const std::string& where,
                                                  const std::optional< double >& sum,
                                                  const std::optional< double >& count,
                                                  const bucket_validation& buckets )
         {
            if( sum.has_value() != count.has_value() ) {
               error( where, "Sum and Count must either both be present or both be absent" );
            }
            if( count && buckets.positive_infinity_count && ( *count != *buckets.positive_infinity_count ) ) {
               error( where, "Count must equal the +Inf bucket count" );
            }
         }

         static void validate_value( const std::string& where, const histogram_value& value )
         {
            const auto buckets = validate_buckets( where, value );
            validate_count( where, "Count", value.count );
            validate_count_relationship( where, value.sum, value.count, buckets );

            if( value.sum && ( std::isnan( *value.sum ) || ( *value.sum < 0.0 ) ) ) {
               error( where, "Histogram Sum must be non-NaN and non-negative" );
            }
            if( value.sum && buckets.has_negative_threshold ) {
               error( where, "Histogram Sum must be absent when negative bucket thresholds are present" );
            }
            validate_created( where, value.created, true );
         }

         static void validate_value( const std::string& where, const gauge_histogram_value& value )
         {
            const auto buckets = validate_buckets( where, value );
            validate_count( where, "Gcount", value.count );
            validate_count_relationship( where, value.sum, value.count, buckets );

            if( value.sum && std::isnan( *value.sum ) ) {
               error( where, "GaugeHistogram Gsum must not be NaN" );
            }
            if( value.sum && ( *value.sum < 0.0 ) && !buckets.has_negative_threshold ) {
               error( where, "GaugeHistogram Gsum can only be negative when negative bucket thresholds are present" );
            }
         }

         static void validate_value( const std::string& where, const summary_value& value )
         {
            validate_count( where, "Count", value.count );
            if( value.sum && ( std::isnan( *value.sum ) || ( *value.sum < 0.0 ) ) ) {
               error( where, "Summary Sum must be non-NaN and non-negative" );
            }
            validate_created( where, value.created, value.count.has_value() || value.sum.has_value() );

            for( const auto& [ key, quantile ] : value.quantiles ) {
               const std::string quantile_where = where + ", quantile " + number_string( key );
               if( std::isnan( key ) || ( key < 0.0 ) || ( key > 1.0 ) ) {
                  error( quantile_where, "quantile must be between 0 and 1" );
               }
               if( quantile < 0.0 ) {
                  error( quantile_where, "quantile value must not be negative" );
               }
            }
         }

         static void validate_counter_sequence( const std::string& family, const std::size_t metric_index, const metric< counter_value >& metric )
         {
            std::optional< double > previous;
            for( std::size_t i = 0; i < metric.metric_points.size(); ++i ) {
               const auto& point = metric.metric_points.at( i );
               const auto total = point.value.total;
               if( !std::isnan( total ) ) {
                  if( previous && ( total < *previous ) ) {
                     const std::string where = point_location( family, metric_index, i );
                     if( total != 0.0 ) {
                        error( where, "Counter Total decreased without resetting to zero" );
                     }
                     else if( !point.value.created ) {
                        error( where, "Counter reset requires a Created timestamp" );
                     }
                  }
                  previous = total;
               }
            }
         }

         void validate_common_labels() const
         {
            if( metric_labels.size() < 2 ) {
               return;
            }
            label_set common = *metric_labels.front();
            for( std::size_t i = 1; i < metric_labels.size(); ++i ) {
               for( auto j = common.begin(); j != common.end(); ) {
                  const auto found = metric_labels.at( i )->find( j->first );
                  if( found == metric_labels.at( i )->end() || ( found->second != j->second ) ) {
                     j = common.erase( j );
                  }
                  else {
                     ++j;
                  }
               }
            }
            for( const auto& label : common ) {
               warning( "MetricSet", "label \"" + label.first + "\" has the same value on every Metric" );
            }
         }
      };

   }  // namespace internal

   // Validation after construction deliberately does not check the following:
   //
   // - Source ordering and interleaving of MetricFamilies, Metrics, MetricPoints,
   //   metadata, and samples; construction no longer retains this ordering.
   // - Source ordering of buckets, or duplicate buckets, quantiles, states, and
   //   sample components; maps reorder keys and repeated components are overwritten.
   // - Original sample-name suffixes and their compatibility with the family type.
   // - Whether Info sample values were exactly 1 or StateSet sample values were
   //   exactly 0 or 1; those values are discarded or converted while constructing.
   // - The distinction between Info Metric labels and Info MetricPoint data labels.
   // - Whether a Counter Total was absent or explicitly zero.
   // - Exact source number spelling and precision, including canonical label
   //   numbers and integer values that were rounded during conversion to double.
   // - Exemplar contents, which this ingestor intentionally discards.
   // - Properties requiring information from other expositions or the underlying
   //   observations, such as lifetime Info stability, the actual time of a Counter
   //   reset, and whether a Sum is exact.

   inline void check( const metric_set& metrics )
   {
      internal::check_impl().check( metrics );
   }

}  // namespace open_metrics

#endif
