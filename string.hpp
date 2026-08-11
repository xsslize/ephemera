#pragma once

#include <ntddk.h>
#include <cstdint>
#include <emmintrin.h>
#include <intrin.h>

#define _( Text ) String::CString( [ ]( ) { return Text; }, \
		std::integral_constant< size_t, ( sizeof( Text ) / 8 ) + ( sizeof( Text ) % 8 != 0 ) >{ }, \
		std::integral_constant< size_t, sizeof( Text ) / sizeof( Text[ 0 ] ) >{ }, \
		std::make_index_sequence< ( sizeof( Text ) / 8 ) + ( sizeof( Text ) % 8 != 0 ) >{ } ).Get( )

namespace std
{
	template< class T > struct remove_reference { using type = T; };
	template< class T > struct remove_reference< T& > { using type = T; };
	template< class T > struct remove_reference< T&& > { using type = T; };
	template< class T > using  remove_reference_t = typename remove_reference< T >::type;

	template< class T > struct remove_const { using type = T; };
	template< class T > struct remove_const< const T > { using type = T; };
	template< class T > using  remove_const_t = typename remove_const< T >::type;

	template< class T, T V >
	struct integral_constant
	{
		static constexpr T value = V;
		using value_type = T;
		using type = integral_constant;
		constexpr operator value_type( ) const noexcept { return value; }
	};

	template< class T, T... Vals >
	struct integer_sequence
	{
		static constexpr size_t size( ) noexcept { return sizeof...( Vals ); }
	};

	template <size_t... Vals>
	using index_sequence = integer_sequence< size_t, Vals... >;

	namespace detail
	{
		template <size_t N, size_t... Is>
		struct make_index_seq : make_index_seq< N - 1, N - 1, Is... > { };

		template <size_t... Is>
		struct make_index_seq< 0, Is... > { using type = index_sequence< Is... >; };
	}

	template <size_t N>
	using make_index_sequence = typename detail::make_index_seq< N >::type;
}

namespace String
{
	template< class T >
	using CleanType = std::remove_const_t< std::remove_reference_t< T > >;

	__forceinline uintptr_t ForceFromReg( uintptr_t Value ) noexcept
	{
		volatile uintptr_t Reg = Value;
		return Reg;
	}

	__forceinline uint32_t ForceFromReg32( uint32_t Value ) noexcept
	{
		volatile uint32_t Reg = Value;
		return Reg;
	}

	__forceinline int32_t ForceFromReg32S( int32_t Value ) noexcept
	{
		volatile int32_t Reg = Value;
		return Reg;
	}

	template< typename To, typename From >
	__forceinline To BitCast( From F )
	{
		static_assert( sizeof( To ) == sizeof( From ) );

		To Result;
		__movsb( reinterpret_cast< unsigned char* >( &Result ), reinterpret_cast< const unsigned char* >( &F ), sizeof( To ) );

		return Result;
	}

	template< uint32_t Seed >
	__forceinline constexpr uint32_t Key4( ) noexcept
	{
		uint32_t Value = Seed;
		for ( char Char : KEYSEED )
			Value = static_cast< uint32_t >( ( Value ^ Char ) * 16777619ull );

		return Value;
	}

	template< size_t Seed >
	__forceinline constexpr uintptr_t Key8( )
	{
		constexpr auto First = Key4< 2166136261u + Seed >( );
		constexpr auto Second = Key4< First >( );
		return ( static_cast< uintptr_t >( First ) << 32 ) | Second;
	}

	template< size_t Size, typename CharT >
	__forceinline constexpr uintptr_t LoadLowStorage( uintptr_t Key, uintptr_t Bloat, size_t Idx, const CharT* Str ) noexcept
	{
		constexpr auto ValueSize = sizeof( CharT );
		constexpr auto IdxOffset = 8 / ValueSize;

		uintptr_t Value = 0;
		for ( size_t i = 0; i < IdxOffset && i + ( Idx * IdxOffset ) < Size; ++i )
		{
			Value |= static_cast< uintptr_t >( Str[ i + ( Idx * IdxOffset ) ] ) << ( ( i % IdxOffset ) * 8 * ValueSize );
		}

		return ( ~( ( Bloat & 0xFFFFFFFF00000000ull ) | ( ( ( Value & 0xFFFFFFFF0000ull ) >> 16 ) & 0xFFFFFFFFull ) ) ) ^ Key;
	}

	template< size_t Size, typename CharT >
	__forceinline constexpr uintptr_t LoadHighStorage( uintptr_t Key, uintptr_t Bloat, size_t Idx, const CharT* Str ) noexcept
	{
		constexpr auto ValueSize = sizeof( CharT );
		constexpr auto IdxOffset = 8 / ValueSize;

		uintptr_t Value = 0;
		for ( size_t i = 0; i < IdxOffset && i + ( Idx * IdxOffset ) < Size; ++i )
		{
			Value |= static_cast< uintptr_t >( Str[ i + ( Idx * IdxOffset ) ] ) << ( ( i % IdxOffset ) * 8 * ValueSize );
		}

		return ( ~( ( Bloat & 0xFFFFFFFF0000ull ) | ( ( Value & 0xFFFF000000000000ull ) | ( Value & 0xFFFFull ) ) ) ) ^ Key;
	}

	__forceinline void Decrypt( uintptr_t* LowBlock, uintptr_t* HighBlock, size_t Index, uintptr_t* KeyLow, uintptr_t* KeyHigh )
	{
		struct { uintptr_t Lo; uintptr_t Hi; } Key, AllBits, Storage;

		Key.Lo = ForceFromReg( KeyLow[ Index ] );
		Key.Hi = ForceFromReg( KeyHigh[ Index ] );
		AllBits.Lo = ~ForceFromReg( static_cast< uintptr_t >( 0 ) );
		AllBits.Hi = ~ForceFromReg( static_cast< uintptr_t >( 0 ) );
		Storage.Lo = ForceFromReg( LowBlock[ Index ] );
		Storage.Hi = ForceFromReg( HighBlock[ Index ] );

		_mm_storeu_si128( reinterpret_cast< __m128i* >( &Storage ), _mm_andnot_si128( _mm_xor_si128(
			_mm_loadu_si128( reinterpret_cast< const __m128i* >( &Key ) ), _mm_loadu_si128(
				reinterpret_cast< const __m128i* >( &Storage ) ) ), _mm_loadu_si128( reinterpret_cast< const __m128i* >( &AllBits ) ) ) );

		LowBlock[ Index ] =
			( Storage.Hi & ForceFromReg( static_cast< uintptr_t >( 0xFFFFull ) ) ) |
			( ( Storage.Lo & ForceFromReg( static_cast< uintptr_t >( 0xFFFFFFFFull ) ) ) << 16 ) |
			( Storage.Hi & ForceFromReg( static_cast< uintptr_t >( 0xFFFF000000000000ull ) ) );
	}

	template< typename T, size_t BlocksCount, size_t Size, typename KeysLow, typename KeysHigh, typename Bloat, typename Indices >
	class CString;

	template< typename T, size_t BlocksCount, size_t Size, uintptr_t... KeysLow, uintptr_t... KeysHigh, uintptr_t... Bloat, size_t... Indices >
	class CString< T, BlocksCount, Size, std::integer_sequence< uintptr_t, KeysLow... >, std::integer_sequence< uintptr_t, KeysHigh... >,
		std::integer_sequence< uintptr_t, Bloat... >, std::index_sequence< Indices... > >
	{
	private:
		uintptr_t LowBlocks[ sizeof...( KeysLow ) ];
		uintptr_t HighBlocks[ sizeof...( KeysLow ) ];
	public:
		template< typename L >
		__forceinline constexpr CString( L Fn, std::integral_constant< size_t, BlocksCount >, std::integral_constant< size_t, Size >, std::index_sequence< Indices... > )
			: LowBlocks{ ForceFromReg( ( std::integral_constant< uintptr_t, LoadLowStorage< Size >( KeysLow, Bloat, Indices, Fn( ) ) >::value ) )... },
			HighBlocks{ ForceFromReg( ( std::integral_constant< uintptr_t, LoadHighStorage< Size >( KeysHigh, Bloat, Indices, Fn( ) ) >::value ) )... }
		{

		}

		__forceinline T* Get( )
		{
			uintptr_t ArrLow[ ]{ ForceFromReg( KeysLow )... };
			uintptr_t ArrHigh[ ]{ ForceFromReg( KeysHigh )... };

			auto* pKeyLow = reinterpret_cast< uintptr_t* >( ForceFromReg( reinterpret_cast< uintptr_t >( ArrLow ) ) );
			auto* pKeyHigh = reinterpret_cast< uintptr_t* >( ForceFromReg( reinterpret_cast< uintptr_t >( ArrHigh ) ) );

			( ( Decrypt( &LowBlocks[ 0 ], &HighBlocks[ 0 ], Indices, &pKeyLow[ 0 ], &pKeyHigh[ 0 ] ) ), ... );

			return reinterpret_cast< T* >( LowBlocks );
		}

		__forceinline ~CString( )
		{
			__stosb( reinterpret_cast< uint8_t* >( LowBlocks ), 0, sizeof( LowBlocks ) );
			__stosb( reinterpret_cast< uint8_t* >( HighBlocks ), 0, sizeof( HighBlocks ) );
		}
	};

	template <typename L, size_t BlocksCount, size_t Size, size_t... Indices>
	CString( L, std::integral_constant< size_t, BlocksCount >, std::integral_constant< size_t, Size >, std::index_sequence< Indices... > ) -> CString
		< std::remove_const_t < std::remove_reference_t< decltype( L{}( )[ 0 ] ) > >, BlocksCount, Size,
		std::integer_sequence< uintptr_t, Key8< Indices + 1 >( )... >,
		std::integer_sequence< uintptr_t, Key8< ( Indices + 2 ) * 4 >( )... >,
		std::integer_sequence< uintptr_t, Key8< ( Indices + 4 ) * 16 >( )... >,
		std::index_sequence< Indices... > >;
}