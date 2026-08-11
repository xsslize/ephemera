#pragma once

template< typename T, T Value >
struct ConstantHolderT
{
	enum class EValueHolder : T { _Value = Value };
};

#define ConstantH( Value ) ( static_cast< decltype( Value ) >( ConstantHolderT< decltype( Value ), Value >::EValueHolder::_Value ) )

namespace Fnv1a
{
	constexpr auto FnvBasis = 14695981039346656037ull;
	constexpr auto FnvPrime = 1099511628211ull;

	template< bool IgnoreCase = false, typename T = char >
	__forceinline unsigned long long Runtime( const T* String )
	{
		auto Hash = FnvBasis;

		size_t Len = 0;
		while ( String[ Len ] ) ++Len;

		for ( unsigned i = 0; i < Len; i++ )
		{
			char C = static_cast< char >( static_cast< unsigned char >( String[ i ] ) );
			if constexpr ( IgnoreCase )
				if ( C >= 'A' && C <= 'Z' ) C += 32;
			Hash ^= static_cast< unsigned char >( C );
			Hash *= FnvPrime;
		}

		return Hash;
	}

	template< typename T >
	constexpr unsigned long long Constant( const T* String, unsigned long long Value = FnvBasis )
	{
		return !*String
			? Value
			: Constant( String + 1,
				static_cast< unsigned long long >(
					1ull * ( Value ^ static_cast< unsigned char >( *String ) ) * FnvPrime ) );
	}
}

#define HashNonCrypted( String ) ConstantH( Fnv1a::Constant( String ) )
#define HashRuntime( String ) Fnv1a::Runtime( String )
#define HashRuntimeCaseInsensetive( String ) Fnv1a::Runtime< true >( String )