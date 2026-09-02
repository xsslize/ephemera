#pragma once

#include "../modules/modules.hpp"

namespace Utils
{
	template< typename DestinationType = uintptr_t >
	__forceinline void ZeroMem( DestinationType Destination, size_t Size )
	{
		if ( !( Size % 16 ) )
		{
			auto Zero = _mm_setzero_si128( );
			for ( size_t i = 0; i < Size; i += 16 )
				_mm_storeu_si128( reinterpret_cast< __m128i_u* >( ( uintptr_t ) Destination + i ), Zero );

			return;
		}

		if ( !( Size % 8 ) )
		{
			Size /= 8;
			asm volatile( "rep stosq" : "+D"( Destination ), "+c"( Size ) : "a"( 0 ) : "memory" );

			return;
		}

		if ( !( Size % 4 ) )
		{
			Size /= 4;
			asm volatile( "rep stos{l|d}" : "+D"( Destination ), "+c"( Size ) : "a"( 0 ) : "memory" );

			return;
		}

		asm volatile( "rep stosb" : "+D"( Destination ), "+c"( Size ) : "a"( 0 ) : "memory" );
	}

	template< typename DestinationType = uintptr_t, typename SourceType = uintptr_t >
	__forceinline void CopyMem( DestinationType Destination, SourceType Source, size_t Size )
	{
		if ( !( Size % 16 ) )
		{
			for ( size_t i = 0; i < Size; i += 16 )
			{
				_mm_storeu_si128( reinterpret_cast< __m128i_u* >( ( uintptr_t ) Destination + i ),
								  _mm_loadu_si128( reinterpret_cast< const __m128i_u* >( ( uintptr_t ) Source + i ) ) );
			}

			return;
		}

		if ( !( Size % 8 ) )
		{
			__movsq( reinterpret_cast< uint64_t* >( Destination ), reinterpret_cast< const uint64_t* >( Source ), Size / 8 );
			return;
		}

		if ( !( Size % 4 ) )
		{
			__movsd( reinterpret_cast< unsigned long* >( Destination ), reinterpret_cast< const unsigned long* >( Source ), Size / 4 );
			return;
		}

		__movsb( reinterpret_cast< uint8_t* >( Destination ), reinterpret_cast< const uint8_t* >( Source ), Size );
	}

	template< typename T = char >
	_FORCEINLINE size_t Strlen( const T* String )
	{
		size_t Length = 0;
		while ( String[ Length++ ] );
		return Length - 1;
	}
}