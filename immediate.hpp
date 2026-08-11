#pragma once

#include "wrapped.hpp"

#define ImmediateVal( Value ) ( Immediate::CImmediate< Value, \
	String::Key8< static_cast< uint16_t >( HashNonCrypted( #Value ) ) >( ), \
	String::Key8< static_cast< uint16_t >( HashNonCrypted( #Value ) * 2 ) >( ), \
	String::Key8< static_cast< uint16_t >( HashNonCrypted( #Value ) * 3 ) >( ) >( ).Get( ) )
#define ImmediateValT( Type, Value ) ( static_cast< Type >( static_cast< uintptr_t >( ImmediateVal( Value ) ) ) )
#define FnvHash( String ) ImmediateValT( uintptr_t, HashNonCrypted( String ) )
#define FloatVal( Value ) ( String::BitCast< float >( String::ForceFromReg32( String::BitCast< uint32_t >( Value ) ) ) )
#define DoubleVal( Value ) ( String::BitCast< double >( String::ForceFromReg( String::BitCast< uintptr_t >( Value ) ) ) )

namespace Immediate
{
	template< uintptr_t Value, uintptr_t Key1, uintptr_t Key2, uintptr_t Bloat >
	class CImmediate
	{
	private:
		uintptr_t StorageLow;
		uintptr_t StorageHigh;

	public:
		constexpr CImmediate( )
		{
			StorageLow = ( ~( ( Bloat & 0xFFFFFFFF00000000ull ) | ( ( ( Value & 0xFFFFFFFF0000ull ) >> 16 ) & 0xFFFFFFFFull ) ) ) ^ Key1;
			StorageHigh = ( ~( ( Bloat & 0xFFFFFFFF0000ull ) | ( ( Value & 0xFFFF000000000000ull ) | ( Value & 0xFFFFull ) ) ) ) ^ Key2;
		}

		__forceinline uintptr_t Get( )
		{
			struct { uintptr_t Lo; uintptr_t Hi; } Key, AllBits, Stor;

			Key.Lo = String::ForceFromReg( Key1 );
			Key.Hi = String::ForceFromReg( Key2 );
			AllBits.Lo = ~String::ForceFromReg( static_cast< uintptr_t >( 0 ) );
			AllBits.Hi = ~String::ForceFromReg( static_cast< uintptr_t >( 0 ) );
			Stor.Lo = String::ForceFromReg( StorageLow );
			Stor.Hi = String::ForceFromReg( StorageHigh );

			_mm_storeu_si128( reinterpret_cast< __m128i* >( &Stor ), _mm_andnot_si128( _mm_xor_si128(
				_mm_loadu_si128( reinterpret_cast< const __m128i* >( &Key ) ), _mm_loadu_si128( reinterpret_cast< const __m128i* >( &Stor ) ) ),
				_mm_loadu_si128( reinterpret_cast< const __m128i* >( &AllBits ) ) ) );

			return ( Stor.Hi & String::ForceFromReg( static_cast< uintptr_t >( 0xFFFFull ) ) ) |
				( ( Stor.Lo & String::ForceFromReg( static_cast< uintptr_t >( 0xFFFFFFFFull ) ) ) << 16 ) |
				( Stor.Hi & String::ForceFromReg( static_cast< uintptr_t >( 0xFFFF000000000000ull ) ) );
		}
	};
}