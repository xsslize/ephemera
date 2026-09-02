#pragma once

#include "string.hpp"
#include "fnv.hpp"

#ifndef _DEBUG
#define CreateWrapped( Type, Name ) Wrapped::CWrapped< Type, \
	String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) ) >( ), \
	String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 2 ) >( ), \
	String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 3 ) >( ), \
	String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 4 ) >( ) > Name
#else
#define CreateWrapped( Type, Name ) Wrapped::CWrapped< Type, 0, 0, 0, 0 > Name
#endif

namespace Wrapped
{
	template< typename T, uintptr_t Key1, uintptr_t Key2, uintptr_t Bloat1, uintptr_t Bloat2 >
	class CWrapped
	{
	private:
		union U128T
		{
			struct { uintptr_t LongLow; uintptr_t LongHigh; };
			struct
			{
				struct
				{
					uintptr_t B0 : 4;  uintptr_t B1 : 4;  uintptr_t B2 : 4;  uintptr_t B3 : 4;
					uintptr_t B4 : 4;  uintptr_t B5 : 4;  uintptr_t B6 : 4;  uintptr_t B7 : 4;
					uintptr_t B8 : 4;  uintptr_t B9 : 4;  uintptr_t B10 : 4; uintptr_t B11 : 4;
					uintptr_t B12 : 4; uintptr_t B13 : 4; uintptr_t B14 : 4; uintptr_t B15 : 4;
				};
				struct
				{
					uintptr_t H0 : 4;  uintptr_t H1 : 4;  uintptr_t H2 : 4;  uintptr_t H3 : 4;
					uintptr_t H4 : 4;  uintptr_t H5 : 4;  uintptr_t H6 : 4;  uintptr_t H7 : 4;
					uintptr_t H8 : 4;  uintptr_t H9 : 4;  uintptr_t H10 : 4; uintptr_t H11 : 4;
					uintptr_t H12 : 4; uintptr_t H13 : 4; uintptr_t H14 : 4; uintptr_t H15 : 4;
				} Hi;
			};
		};

		union
		{
			__m128i Storage;
			struct { uintptr_t StorageLow; uintptr_t StorageHigh; };
		};

	public:
		CWrapped( ) { }

		template< typename V >
		__forceinline void operator=( const V& Value )
		{
			U128T Init;
			Init.LongLow = ~( uintptr_t )( Value );

			U128T Perm;
			Perm.LongLow = String::ForceFromReg( Bloat1 );
			Perm.LongHigh = String::ForceFromReg( Bloat2 );

			Perm.B1 = Init.B4; Perm.Hi.H12 = Init.B10;
			Perm.B4 = Init.B7; Perm.Hi.H4 = Init.B2;
			Perm.B5 = Init.B9; Perm.Hi.H10 = Init.B12;
			Perm.B15 = Init.B1; Perm.B8 = Init.B11;
			Perm.Hi.H0 = Init.B15; Perm.Hi.H13 = Init.B8;
			Perm.B10 = Init.B0; Perm.Hi.H8 = Init.B3;
			Perm.Hi.H5 = Init.B13; Perm.B12 = Init.B6;
			Perm.B14 = Init.B5; Perm.Hi.H2 = Init.B14;

			struct { uintptr_t Lo; uintptr_t Hi; } Key;
			Key.Lo = String::ForceFromReg( Key1 );
			Key.Hi = String::ForceFromReg( Key2 );

			_mm_storeu_si128( &Storage, _mm_xor_si128( _mm_loadu_si128( reinterpret_cast< const __m128i* >( &Key ) ),
													   _mm_loadu_si128( reinterpret_cast< const __m128i* >( &Perm ) ) ) );
		}

		__forceinline T Get( ) const
		{
			struct { uintptr_t Lo; uintptr_t Hi; } Key;
			Key.Lo = String::ForceFromReg( Key1 );
			Key.Hi = String::ForceFromReg( Key2 );

			U128T Wrapped;
			_mm_storeu_si128( reinterpret_cast< __m128i* >( &Wrapped ), _mm_xor_si128(
				_mm_loadu_si128( reinterpret_cast< const __m128i* >( &Key ) ), _mm_loadu_si128( &Storage ) ) );

			U128T Src;
			Src.B4 = Wrapped.B1; Src.B10 = Wrapped.Hi.H12;
			Src.B7 = Wrapped.B4; Src.B2 = Wrapped.Hi.H4;
			Src.B9 = Wrapped.B5; Src.B12 = Wrapped.Hi.H10;
			Src.B1 = Wrapped.B15; Src.B11 = Wrapped.B8;
			Src.B15 = Wrapped.Hi.H0; Src.B8 = Wrapped.Hi.H13;
			Src.B0 = Wrapped.B10; Src.B3 = Wrapped.Hi.H8;
			Src.B13 = Wrapped.Hi.H5; Src.B6 = Wrapped.B12;
			Src.B5 = Wrapped.B14; Src.B14 = Wrapped.Hi.H2;

			return ( T )( ~Src.LongLow );
		}

		explicit __forceinline operator bool( ) const
		{
			return Get( ) != 0 && StorageLow != 0 && StorageHigh != 0;
		}

		__forceinline operator T( ) const
		{
			return Get( );
		}

		__forceinline T operator->( )
		{
			return Get( );
		}
	};
}