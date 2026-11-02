# Input Reference

The reference documentation for all (contiguous memory) inputs.
The [stream](Stream-Parsing.md) inputs are [documented here](Stream-Parsing.md#inputs).


## Contents

* [Preamble](#preamble)
* [Inputs](#inputs)
  * [Argv Input](#argv-input)
  * [Base Input](#base-input)
  * [View Input](#view-input)
  * [Copy Input](#copy-input)
  * [File Input](#file-input)
  * [Read Input](#read-input)
  * [Mmap Input](#mmap-input)
  * [Text View Input](#text-view-input)
  * [Text Copy Input](#text-copy-input)
  * [Text File Input](#text-file-input)
  * [Text Read Input](#text-read-input)
  * [Text Mmap Input](#text-mmap-input)
* [Input Adapters](#input-adapters)
  * [Input with Depth](#input-with-depth)
  * [Input with Offset](#input-with-offset)
* [Index](#index)


## Preamble

All inputs documented on this page are defined in `tao/pegtl/inputs.hpp` which is included from `tao/pegtl.hpp`.

All inputs documented on this page implement the common input functions consisting of

* the [input interface](Inputs-and-Parsing.md#input-interface),
* the applicable [convenience functions](Inputs-and-Parsing.md#input-convenience),
* the [stream compatibility](Inputs-and-Parsing.md#stream-compatibility).

#### Namespaces

All inputs and the type alias `default_eol` reside in namespace `tao::pegtl`.
This default can be changed via the macro `TAO_PEGTL_NAMESPACE` in `tao/pegtl/config.hpp`.
The namespace `tao::pegtl` is generally omitted on this page.

#### Containers

Some deduction guides for `copy_input` and `text_copy_input` use `internal::container_for_data_t< Data >` to choose a container for copied input data.
This applies when an input is constructed from raw data, an array, or an initializer list, where the constructor receives elements instead of an already chosen container.

* When `std::decay_t< Data >` is `char`, the selected container is `std::string`.
* For all other `Data` types, the selected container is `std::vector< Data >`.

When `copy_input` or `text_copy_input` is constructed from a container directly, its deduction guides use `internal::container_for_container_t< Container >` instead.
That helper keeps the supplied container type, except that data referenced by a `std::string_view` is copied into a `std::string`.

#### Spans

The non-owning `base_input`, `view_input` and `text_view_input` classes also accept `std::span< const Data, Extent >`.
The extent can be fixed or dynamic.
All elements of the span become input, including a null terminator if one is part of the span.
The underlying objects must outlive the input; the span object itself need not.
There are no deduction guides for span arguments, so specify the input template arguments when constructing from a span.

For the `span()` accessor on inputs and action inputs, see [Input Convenience](Inputs-and-Parsing.md#span-views).

#### Text Inputs

The text inputs use a `text_position` that tracks column and line numbers in addition to the count of parsed objects.

> [!NOTE]
> Both column and line numbers are one-based, i.e. column 1 in line 1 corresponds to the first input object.

> [!IMPORTANT]
> The column corresponds to the number of input objects consumed since the last end-of-line, not the number of code points or extended grapheme clusters.

The non-text inputs also have an `Eol` template parameter, however they only use it for the [`eol`](Rule-Reference.md#eol) and [`eolf`](Rule-Reference.md#eolf) rules, **not** for column and line numbers in the position.


## Inputs

By [default](Introduction.md#namespaces) all inputs reside in namespace `tao::pegtl`.

### Argv Input

* Used to parse a single command line argument `argv[ n ]`.
* Sets the source to `"argv[ n ]"` when passed `argv` and `n`.
* [Restartable](Inputs-and-Parsing.md#inputs-with-start).
* [Without lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [With source](Inputs-and-Parsing.md#inputs-with-source) (by default).

#### Exposition

```c++
template< typename Eol = void, typename Source = std::string >
struct argv_input
{
   using data_t = char;
   using error_position_t = count_position;   // When Source is void, or
   using error_position_t = position_with_source< Source, count_position >;  // when Source is not void.
   using offset_position_t = count_position;
   using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = Source;  // Only when not void.
   using error_source_t = Source;  // Only when not void.
```

#### Construction

```c++
   argv_input( char** argv, const int argn );
```

#### Deduction Guides

```c++
argv_input( char**, const int ) -> argv_input<>;
```

### Base input

* Does **not** copy the input data!
* [Without start](Inputs-and-Parsing.md#inputs-with-start)
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [Without source](Inputs-and-Parsing.md#inputs-with-source).

This is the most light-weight of all input classes.
It only keeps two pointers, the one returned by `current()` and the one returned by `end()`.

#### Exposition

```c++
template< typename Eol = default_eol, typename Data = char >
struct base_input
{
   using data_t = Data;
   using error_position_t = pointer_position< data_t >;
   using offset_position_t = void;
   using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.
```

#### Construction

```c++
   base_input( const Data* begin, const Data* end ) noexcept;
   base_input( const Data* begin, const std::size_t size ) noexcept;

   template< std::size_t Extent >
   explicit base_input( const std::span< const Data, Extent > data ) noexcept;

   explicit base_input( std::vector< Data >& data ) noexcept;
   explicit base_input( const std::vector< Data >& data ) noexcept;

   template< std::size_t Size >
   explicit base_input( const std::array< Data, Size >& a ) noexcept;

   // Only available when Data is (compatible with) char:

   explicit base_input( std::string& data ) noexcept;
   explicit base_input( const std::string& data ) noexcept;
   explicit base_input( const std::string_view data ) noexcept;

   template< std::size_t N >
   explicit base_input( const char ( &data )[ N ] ) noexcept;

   // To prevent accidents with temporaries:

   base_input( std::string&& ) = delete;
   base_input( const std::string&& ) = delete;

   base_input( std::vector< Data >&& ) = delete;
   base_input( const std::vector< Data >&& ) = delete;
```

#### Deduction Guides

```c++
template< typename Data >
base_input( const Data*, const Data* ) -> base_input< default_eol, Data >;

template< typename Data >
base_input( const Data*, const std::size_t ) -> base_input< default_eol, Data >;

base_input( std::string& ) -> base_input< default_eol, char >;
base_input( const std::string& ) -> base_input< default_eol, char >;
base_input( const std::string_view ) -> base_input< default_eol, char >;

template< typename Data, typename... Params >
base_input( std::vector< Data, Params... >& ) -> base_input< default_eol, Data >;

template< typename Data, typename... Params >
base_input( const std::vector< Data, Params... >& ) -> base_input< default_eol, Data >;

template< std::size_t Size >
base_input( const char ( & )[ Size ] ) -> base_input< default_eol, char >;

template< typename Data, std::size_t Size >
base_input( const std::array< Data, Size >& ) -> base_input< default_eol, Data >;
```

### View Input

* Does **not** copy the input data.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [Without source](Inputs-and-Parsing.md#inputs-with-source) (by default).
* Like [`base_input`](#base-input) but with start.
* Like [`text_view_input`](#text-view-input) but without lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol, typename Data = char, typename InputSource = void, typename ErrorSource = InputSource >
struct view_input
{
   using data_t = Data;
   using error_position_t = count_position;   // When ErrorSource is void, or
   using error_position_t = position_with_source< ErrorSource, count_position >;  // when ErrorSource is not void.
   using offset_position_t = count_position;
   using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = InputSource;  // Only when not void.
   using error_source_t = ErrorSource;  // Only when not void.
```

#### Construction

When `InputSource` and `ErrorSource` are both `void`.

```c++
   view_input( const Data* begin, const Data* end ) noexcept;
   view_input( const Data* begin, const std::size_t size ) noexcept;

   template< std::size_t Extent >
   explicit view_input( const std::span< const Data, Extent > data ) noexcept;

   explicit view_input( std::vector< Data >& data ) noexcept;
   explicit view_input( const std::vector< Data >& data ) noexcept;

   // To prevent accidents with temporaries:

   view_input( std::string&& ) = delete;
   view_input( const std::string&& ) = delete;

   view_input( std::vector< Data >&& ) = delete;
   view_input( const std::vector< Data >&& ) = delete;

   // Only available when Data is (compatible with) char:

   explicit view_input( std::string& data ) noexcept;
   explicit view_input( const std::string& data ) noexcept;
   explicit view_input( const std::string_view data ) noexcept;
```

#### Deduction Guides

Guides that leave `InputSource` and `ErrorSource` as `void`.

```c++
template< typename Data >
view_input( const Data*, const Data* ) -> view_input< default_eol, Data, void, void >;

template< typename Data >
view_input( const Data*, const std::size_t ) -> view_input< default_eol, Data, void, void >;

view_input( std::string& ) -> view_input< default_eol, char, void, void >;
view_input( const std::string& ) -> view_input< default_eol, char, void, void >;
view_input( const std::string_view ) -> view_input< default_eol, char, void, void >;

template< typename Data, typename... Params >
view_input( std::vector< Data, Params... >& ) -> view_input< default_eol, Data, void, void >;

template< typename Data, typename... Params >
view_input( const std::vector< Data, Params... >& ) -> view_input< default_eol, Data, void, void >;

template< std::size_t Size >
view_input( const char ( & )[ Size ] ) -> view_input< default_eol, char, void, void >;

template< typename Data, std::size_t Size >
view_input( const std::array< Data, Size >& ) -> view_input< default_eol, Data, void, void >;
```

#### Construction

When `InputSource` and `ErrorSource` are both **not** `void`.

The argument `source` is forwarded to construct a data member of type `InputSource`.

```c++
   template< typename Source >
   view_input( Source&& source, const Data* begin, const Data* end );

   template< typename Source >
   view_input( Source&& source, const Data* begin, const std::size_t size );

   template< typename Source, std::size_t Extent >
   explicit view_input( Source&& source, const std::span< const Data, Extent > data );

   template< typename Source >
   explicit view_input( Source&& source, std::vector< Data >& data );

   template< typename Source >
   explicit view_input( Source&& source, const std::vector< Data >& data );

   // To prevent accidents with temporaries:

   template< typename Source >
   view_input( Source&& source, std::string&& ) = delete;

   template< typename Source >
   view_input( Source&& source, const std::string&& ) = delete;

   template< typename Source >
   view_input( Source&& source, std::vector< Data >&& ) = delete;

   template< typename Source >
   view_input( Source&& source, const std::vector< Data >&& ) = delete;

   // Only available when Data is compatible with char:

   template< typename Source >
   explicit view_input( Source&& source, std::string& data );

   template< typename Source >
   explicit view_input( Source&& source, const std::string& data );

   template< typename Source >
   explicit view_input( Source&& source, const std::string_view data );
```

#### Deduction Guides

With `source` argument set `InputSource` and `ErrorSource` to `std::string`.

```c++
template< typename String, typename Data >
view_input( String&&, const Data*, const Data* ) -> view_input< default_eol, Data, std::string, std::string >;

template< typename String, typename Data >
view_input( String&&, const Data*, const std::size_t ) -> view_input< default_eol, Data, std::string, std::string >;

template< typename String >
view_input( String&&, std::string& ) -> view_input< default_eol, char, std::string, std::string >;

template< typename String >
view_input( String&&, const std::string& ) -> view_input< default_eol, char, std::string, std::string >;

template< typename String >
view_input( String&&, const std::string_view ) -> view_input< default_eol, char, std::string, std::string >;

template< typename String, typename Data, typename... Params >
view_input( String&&, std::vector< Data, Params... >& ) -> view_input< default_eol, Data, std::string, std::string >;

template< typename String, typename Data, typename... Params >
view_input( String&&, const std::vector< Data, Params... >& ) -> view_input< default_eol, Data, std::string, std::string >;

template< typename String, std::size_t Size >
view_input( String&&, const char ( & )[ Size ] ) -> view_input< default_eol, char, std::string, std::string >;

template< typename String, typename Data, std::size_t Size >
view_input( String&&, const std::array< Data, Size >& ) -> view_input< default_eol, Data, std::string, std::string >;
```

### Copy Input

* Copies the input data to a container data member.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [Without source](Inputs-and-Parsing.md#inputs-with-source) (by default).
* Like [`text_copy_input`](#text-copy-input) but without lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol, typename Container = std::string, typename InputSource = void, typename ErrorSource = InputSource >
struct copy_input
{
   using data_t = typename Container::value_type;
   using error_position_t = count_position;   // When ErrorSource is void, or
   using error_position_t = position_with_source< ErrorSource, count_position >;  // when ErrorSource is not void.
   using offset_position_t = count_position;
   using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = InputSource;  // Only when not void.
   using error_source_t = ErrorSource;  // Only when not void.
```

#### Construction

When `InputSource` and `ErrorSource` are both `void`.

```c++
   copy_input( const data_t* begin, const data_t* end );
   copy_input( const data_t* begin, const std::size_t size );

   explicit copy_input( Container&& data ) noexcept;
   explicit copy_input( const Container& data );

   template< std::size_t Size >
   explicit copy_input( const std::array< data_t, Size >& data );

   explicit copy_input( const std::initializer_list< data_t >& init );
```

#### Deduction Guides

Guides that leave `InputSource` and `ErrorSource` as `void`.

```c++
template< typename data_t >
copy_input( const data_t*, const data_t* ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, void, void >;

template< typename data_t >
copy_input( const data_t*, const std::size_t ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, void, void >;

template< typename Container >
copy_input( Container&& ) -> copy_input< default_eol, internal::container_for_container_t< Container >, void, void >;

template< typename Container >
copy_input( const Container& ) -> copy_input< default_eol, internal::container_for_container_t< Container >, void, void >;

template< typename data_t, std::size_t Size >
copy_input( const std::array< data_t, Size >& ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, void, void >;

template< typename data_t >
copy_input( const std::initializer_list< data_t >& ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, void, void >;
```

#### Construction

When `InputSource` and `ErrorSource` are both **not** `void`.

The argument `source` is forwarded to construct a data member of type `InputSource`.

```c++
   template< typename Source >
   copy_input( Source&& source, const data_t* begin, const data_t* end );

   template< typename Source >
   copy_input( Source&& source, const data_t* begin, const std::size_t size );

   template< typename Source >
   explicit copy_input( Source&& source, Container&& data ) noexcept;

   template< typename Source >
   explicit copy_input( Source&& source, const Container& data );

   template< typename Source, std::size_t Size >
   explicit copy_input( Source&& source, const std::array< data_t, Size >& data );

   template< typename Source >
   explicit copy_input( Source&& source, const std::initializer_list< data_t >& data );
```

#### Deduction Guides

With `source` argument set `InputSource` and `ErrorSource` to `std::string`.

```c++
template< typename String, typename data_t >
copy_input( String&&, const data_t*, const data_t* ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, std::string, std::string >;

template< typename String, typename data_t >
copy_input( String&&, const data_t*, const std::size_t ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, std::string, std::string >;

template< typename String, typename Container >
copy_input( String&&, Container&& ) -> copy_input< default_eol, internal::container_for_container_t< Container >, std::string, std::string >;

template< typename String, typename Container >
copy_input( String&&, const Container& ) -> copy_input< default_eol, internal::container_for_container_t< Container >, std::string, std::string >;

template< typename String, typename data_t, std::size_t Size >
copy_input( String&&, const std::array< data_t, Size >& ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, std::string, std::string >;

template< typename String, typename data_t >
copy_input( String&&, const std::initializer_list< data_t >& ) -> copy_input< default_eol, internal::container_for_data_t< data_t >, std::string, std::string >;
```

### File Input

* Implemented with `mmap_input` when available, and `read_input` as fallback.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [With source](Inputs-and-Parsing.md#inputs-with-source) types `std::filesystem::path`.
* Like [`text_file_input`](#text-file-input) but without lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol >
struct file_input
{
   // Inherits everything from either mmap_input< Eol, char > or read_input< Eol >.
};

template< typename... Args >
file_input( Args...&& ) -> file_input< default_eol >;
```

### Read Input

* Uses `std::fopen()` and `std::fread()`.
* Reads the whole file into a `std::string`.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [With source](Inputs-and-Parsing.md#inputs-with-source) types `std::filesystem::path`.
* Like [`text_read_input`](#text-read-input) but without lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol >
struct read_input
{
   using data_t = char;
   using error_position_t = position_with_source< std::filesystem::path, count_position >;
   using offset_position_t = count_position;
   using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = std::filesystem::path;
   using error_source_t = std::filesystem::path;
```

#### Construction

```c++
   explicit read_input( const std::filesystem::path& path );
   read_input( FILE* file, const std::filesystem::path& path );
```

#### Deduction Guides

```c++
template< typename... Args >
read_input( Args...&& ) -> read_input< default_eol >;
```

### Mmap Input

* Mmaps the file into memory.
* Only available on systems with Posix *mmap* and Windows.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [With source](Inputs-and-Parsing.md#inputs-with-source) types `std::filesystem::path`.
* Like [`text_mmap_input`](#text-mmap-input) but without lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol, typename Data = char >
struct mmap_input
{
   using data_t = Data;
   using error_position_t = position_with_source< std::filesystem::path, count_position >;
   using offset_position_t = count_position;
   using rewind_position_t = pointer_position< data_t >;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = std::filesystem::path;
   using error_source_t = std::filesystem::path;
```

#### Construction

```c++
   explicit mmap_input( const std::filesystem::path& path );
```

#### Deduction Guides

```c++
template< typename... Args >
mmap_input( Args...&& ) -> mmap_input< default_eol, char >;
```

### Text View Input

* Does **not** copy the input data.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [Without source](Inputs-and-Parsing.md#inputs-with-source) (by default).
* Like [`view_input`](#view-input) but with lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol, typename Data = char, typename InputSource = void, typename ErrorSource = InputSource >
struct text_view_input
{
   using data_t = Data;
   using error_position_t = text_position;  // When ErrorSource is void, or
   using error_position_t = position_with_source< ErrorSource, text_position >;  // when ErrorSource is not void.
   using offset_position_t = text_position;
   using rewind_position_t = text_position;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = InputSource;  // Only when not void.
   using error_source_t = ErrorSource;  // Only when not void.
```

#### Construction

When `InputSource` and `ErrorSource` are both `void`.

```c++
   text_view_input( const Data* begin, const Data* end ) noexcept;
   text_view_input( const Data* begin, const std::size_t size ) noexcept;

   template< std::size_t Extent >
   explicit text_view_input( const std::span< const Data, Extent > data ) noexcept;

   explicit text_view_input( std::vector< Data >& data ) noexcept;
   explicit text_view_input( const std::vector< Data >& data ) noexcept;

   text_view_input( std::string&& ) = delete;
   text_view_input( const std::string&& ) = delete;

   text_view_input( std::vector< Data >&& ) = delete;
   text_view_input( const std::vector< Data >&& ) = delete;

   // Only available when Data is (compatible with) char:

   explicit text_view_input( std::string& data ) noexcept;
   explicit text_view_input( const std::string& data ) noexcept;
   explicit text_view_input( const std::string_view data ) noexcept;
```

#### Deduction Guides

Guides that leave `InputSource` and `ErrorSource` as `void`.

```c++
template< typename Data >
text_view_input( const Data*, const Data* ) -> text_view_input< default_eol, Data, void, void >;

template< typename Data >
text_view_input( const Data*, const std::size_t ) -> text_view_input< default_eol, Data, void, void >;

text_view_input( std::string& ) -> text_view_input< default_eol, char, void, void >;
text_view_input( const std::string& ) -> text_view_input< default_eol, char, void, void >;
text_view_input( const std::string_view ) -> text_view_input< default_eol, char, void, void >;

template< typename Data, typename... Params >
text_view_input( std::vector< Data, Params... >& ) -> text_view_input< default_eol, Data, void, void >;

template< typename Data, typename... Params >
text_view_input( const std::vector< Data, Params... >& ) -> text_view_input< default_eol, Data, void, void >;

template< std::size_t Size >
text_view_input( const char ( & )[ Size ] ) -> text_view_input< default_eol, char, void, void >;

template< typename Data, std::size_t Size >
text_view_input( const std::array< Data, Size >& ) -> text_view_input< default_eol, Data, void, void >;
```

#### Construction

When `InputSource` and `ErrorSource` are both **not** `void`.

The argument `source` is forwarded to construct a data member of type `InputSource`.

```c++
   template< typename Source >
   text_view_input( Source&& source, const Data* begin, const Data* end );

   template< typename Source >
   text_view_input( Source&& source, const Data* begin, const std::size_t size );

   template< typename Source, std::size_t Extent >
   explicit text_view_input( Source&& source, const std::span< const Data, Extent > data );

   template< typename Source >
   explicit text_view_input( Source&& source, std::vector< Data >& data );

   template< typename Source >
   explicit text_view_input( Source&& source, const std::vector< Data >& data );

   template< typename Source >
   text_view_input( Source&& source, std::string&& ) = delete;

   template< typename Source >
   text_view_input( Source&& source, const std::string&& ) = delete;

   template< typename Source >
   text_view_input( Source&& source, std::vector< Data >&& ) = delete;

   template< typename Source >
   text_view_input( Source&& source, const std::vector< Data >&& ) = delete;

   // Only available when Data is compatible with char:

   template< typename Source >
   explicit text_view_input( Source&& source, std::string& data );

   template< typename Source >
   explicit text_view_input( Source&& source, const std::string& data );

   template< typename Source >
   explicit text_view_input( Source&& source, const std::string_view data );
```

#### Deduction Guides

With `source` argument set `InputSource` and `ErrorSource` to `std::string`.

```c++
template< typename String, typename Data >
text_view_input( String&&, const Data*, const Data* ) -> text_view_input< default_eol, Data, std::string, std::string >;

template< typename String, typename Data >
text_view_input( String&&, const Data*, const std::size_t ) -> text_view_input< default_eol, Data, std::string, std::string >;

template< typename String >
text_view_input( String&&, std::string& ) -> text_view_input< default_eol, char, std::string, std::string >;

template< typename String >
text_view_input( String&&, const std::string& ) -> text_view_input< default_eol, char, std::string, std::string >;

template< typename String >
text_view_input( String&&, const std::string_view ) -> text_view_input< default_eol, char, std::string, std::string >;

template< typename String, typename Data, typename... Params >
text_view_input( String&&, std::vector< Data, Params... >& ) -> text_view_input< default_eol, Data, std::string, std::string >;

template< typename String, typename Data, typename... Params >
text_view_input( String&&, const std::vector< Data, Params... >& ) -> text_view_input< default_eol, Data, std::string, std::string >;

template< typename String, std::size_t Size >
text_view_input( String&&, const char ( & )[ Size ] ) -> text_view_input< default_eol, char, std::string, std::string >;

template< typename String, typename Data, std::size_t Size >
text_view_input( String&&, const std::array< Data, Size >& ) -> text_view_input< default_eol, Data, std::string, std::string >;
```

### Text Copy Input

* Copies the input data to a container data member.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [Without source](Inputs-and-Parsing.md#inputs-with-source) (by default).
* Like [`copy_input`](#copy-input) but with lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol, typename Container = std::string, typename InputSource = void, typename ErrorSource = InputSource >
struct text_copy_input
{
   using data_t = typename Container::value_type;
   using error_position_t = text_position;  // When ErrorSource is void, or
   using error_position_t = position_with_source< ErrorSource, text_position >;  // when ErrorSource is not void.
   using offset_position_t = text_position;
   using rewind_position_t = text_position;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = InputSource;  // Only when not void.
   using error_source_t = ErrorSource;  // Only when not void.
```

#### Construction

When `InputSource` and `ErrorSource` are both `void`.

```c++
   text_copy_input( const Data* begin, const Data* end );
   text_copy_input( const Data* begin, const std::size_t size );

   explicit text_copy_input( Container&& data ) noexcept;
   explicit text_copy_input( const Container& data );

   using Data = typename Container::value_type;

   template< std::size_t Size >
   explicit text_copy_input( const std::array< Data, Size >& data );

   explicit text_copy_input( const std::initializer_list< Data >& init );
```

#### Deduction Guides

Guides that leave `InputSource` and `ErrorSource` as `void`.

```c++
template< typename Data >
text_copy_input( const Data*, const Data* ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, void, void >;

template< typename Data >
text_copy_input( const Data*, const std::size_t ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, void, void >;

template< typename Container >
text_copy_input( Container&& ) -> text_copy_input< default_eol, internal::container_for_container_t< Container >, void, void >;

template< typename Container >
text_copy_input( const Container& ) -> text_copy_input< default_eol, internal::container_for_container_t< Container >, void, void >;

template< typename Data, std::size_t Size >
text_copy_input( const std::array< Data, Size >& ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, void, void >;

template< typename Data >
text_copy_input( const std::initializer_list< Data >& ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, void, void >;
```

#### Construction

When `InputSource` and `ErrorSource` are both **not** `void`.

The argument `source` is forwarded to construct a data member of type `InputSource`.

```c++
   template< typename Source >
   text_copy_input( Source&& source, const Data* begin, const Data* end );

   template< typename Source >
   text_copy_input( Source&& source, const Data* begin, const std::size_t size );

   template< typename Source >
   explicit text_copy_input( Source&& source, Container&& data ) noexcept;

   template< typename Source >
   explicit text_copy_input( Source&& source, const Container& data );

   template< typename Source, std::size_t Size >
   explicit text_copy_input( Source&& source, const std::array< Data, Size >& data );

   template< typename Source >
   explicit text_copy_input( Source&& source, const std::initializer_list< Data >& data );
```

#### Deduction Guides

With `source` argument set `InputSource` and `ErrorSource` to `std::string`.

```c++
template< typename String, typename Data >
text_copy_input( String&&, const Data*, const Data* ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, std::string, std::string >;

template< typename String, typename Data >
text_copy_input( String&&, const Data*, const std::size_t ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, std::string, std::string >;

template< typename String, typename Container >
text_copy_input( String&&, Container&& ) -> text_copy_input< default_eol, internal::container_for_container_t< Container >, std::string, std::string >;

template< typename String, typename Container >
text_copy_input( String&&, const Container& ) -> text_copy_input< default_eol, internal::container_for_container_t< Container >, std::string, std::string >;

template< typename String, typename Data, std::size_t Size >
text_copy_input( String&&, const std::array< Data, Size >& ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, std::string, std::string >;

template< typename String, typename Data >
text_copy_input( String&&, const std::initializer_list< Data >& ) -> text_copy_input< default_eol, internal::container_for_data_t< Data >, std::string, std::string >;
```

### Text File Input

* Implemented with `text_mmap_input` when available, and `text_read_input` as fallback.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [With source](Inputs-and-Parsing.md#inputs-with-source) types `std::filesystem::path`.
* Like [`file_input`](#file-input) but with lines and columns in the position.

```c++
template< typename Eol = default_eol >
struct text_file_input
{
   // Same construction as either text_mmap_input< Eol, char > or text_read_input< Eol >.
};

// Deduction guide.

template< typename... Args >
text_file_input( Args...&& ) -> text_file_input< default_eol >;
```

### Text Read Input

* Uses `std::fopen()` and `std::fread()`.
* Reads the whole file into a `std::string`.
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [With source](Inputs-and-Parsing.md#inputs-with-source) types `std::filesystem::path`.
* Like [`read_input`](#read-input) but with lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol >
struct text_read_input
{
   using data_t = Data;
   using error_position_t = position_with_source< std::filesystem::path, text_position >;
   using offset_position_t = text_position;
   using rewind_position_t = text_position;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = std::filesystem::path;
   using error_source_t = std::filesystem::path;
```

#### Construction

```c++
   explicit text_read_input( const std::filesystem::path& path );
   text_read_input( FILE* file, const std::filesystem::path& path );
```

#### Deduction Guides

```c++
template< typename... Args >
text_read_input( Args...&& ) -> text_read_input< default_eol >;
```

### Text Mmap Input

* Mmaps the file into memory.
* Only available on systems with Posix `mmap(2)` (and Windows).
* [With start](Inputs-and-Parsing.md#inputs-with-start).
* [With lines](Inputs-and-Parsing.md#inputs-with-lines) (by default).
* [With source](Inputs-and-Parsing.md#inputs-with-source) types `std::filesystem::path`.
* Like [`mmap_input`](#mmap-input) but with lines and columns in the position.

#### Exposition

```c++
template< typename Eol = default_eol, typename Data = char >
struct text_mmap_input
{
   using data_t = char;
   using error_position_t = position_with_source< std::filesystem::path, text_position >;
   using offset_position_t = text_position;
   using rewind_position_t = text_position;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   using eol_rule = Eol;  // Only when not void.

   using input_source_t = std::filesystem::path;
   using error_source_t = std::filesystem::path;
```

#### Construction

```c++
   explicit text_mmap_input( const std::filesystem::path& path );
```

#### Deduction Guides

```c++
template< typename... Args >
text_mmap_input( Args...&& ) -> text_mmap_input< default_eol, char >;
```


## Input Adapters

### Input with Depth

An input adapter that adds a depth counter for use with [`check_depth`](Action-Reference.md#check_depth-n-) to limit the rule recursion depth during a parsing run.

```c++
template< typename Input >
class input_with_depth
   : public Input
{
public:
   using Input::Input;

   [[nodiscard]] internal::depth_guard make_depth_guard() noexcept;
   [[nodiscard]] std::size_t current_depth() const noexcept;
};
```

### Input with Offset

An input adapter that keeps an offset position that is added to all positions reported by the input.
Both `current_position()` and `previous_position()` return the same position type as `Input`'s functions.

```c++
template< typename Input >
class input_with_offset
   : public Input
{
public:
   using data_t = typename Input::data_t;
   using error_position_t = typename Input::error_position_t;
   using offset_position_t = typename Input::offset_position_t;
   using rewind_position_t = typename Input::rewind_position_t;
#if defined( __cpp_exceptions )
   using parse_error_t = parse_error< error_position_t >;
#endif

   template< typename... Ts >
   explicit input_with_offset( offset_position_t&& s, Ts&&... ts );

   template< typename... Ts >
   explicit input_with_offset( const offset_position_t& s, Ts&&... ts );

   [[nodiscard]] auto current_position() const;
   [[nodiscard]] auto previous_position( const rewind_position_t& saved ) const;
   [[nodiscard]] const offset_position_t& direct_offset() const noexcept;

   void direct_position() const = delete;
};
```


## Index

* [Argv Input](#argv-input) <sup>[(input)](#inputs)</sup>
* [Base Input](#base-input) <sup>[(input)](#inputs)</sup>
* [Copy Input](#copy-input) <sup>[(input)](#inputs)</sup>
* [File Input](#file-input) <sup>[(input)](#inputs)</sup>
* [Input with Depth](#input-with-depth) <sup>[(input adapters)](#input-adapters)</sup>
* [Input with Offset](#input-with-offset) <sup>[(input adapters)](#input-adapters)</sup>
* [Mmap Input](#mmap-input) <sup>[(input)](#inputs)</sup>
* [Read Input](#read-input) <sup>[(input)](#inputs)</sup>
* [Text Copy Input](#text-copy-input) <sup>[(input)](#inputs)</sup>
* [Text File Input](#text-file-input) <sup>[(input)](#inputs)</sup>
* [Text Mmap Input](#text-mmap-input) <sup>[(input)](#inputs)</sup>
* [Text Read Input](#text-read-input) <sup>[(input)](#inputs)</sup>
* [Text View Input](#text-view-input) <sup>[(input)](#inputs)</sup>
* [View Input](#view-input) <sup>[(input)](#inputs)</sup>


---

This page is part of the [PEGTL](https://github.com/taocpp/PEGTL) and its [documentation](README.md).

Copyright (c) 2024-2026 Dr. Colin Hirsch and Daniel Frey<br>
Distributed under the Boost Software License, Version 1.0<br>
See accompanying file [LICENSE_1_0.txt](../LICENSE_1_0.txt) or copy at https://www.boost.org/LICENSE_1_0.txt
