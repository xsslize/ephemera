#pragma once

#include "../pe.hpp"

namespace Modules
{
	struct ModuleT
	{
		uintptr_t GetExport( uint64_t ExportHash );

		template< typename T = uintptr_t >
		_FORCEINLINE T As( )
		{
			return ( T ) this;
		}

		template< typename T = uintptr_t >
		_FORCEINLINE const std::remove_const_t< T > Offset( size_t Offset )
		{
			return ( const T ) ( reinterpret_cast< uintptr_t >( this ) + Offset );
		}

		_FORCEINLINE const IMAGE_NT_HEADERS64* GetNt( )
		{
			return Offset< IMAGE_NT_HEADERS* >( As< IMAGE_DOS_HEADER* >( )->e_lfanew );
		}

		template< typename T = uintptr_t >
		_FORCEINLINE bool IsInside( T Address )
		{
			return Address >= As< T >( ) && Address < Offset< T >( GetNt( )->OptionalHeader.SizeOfImage );
		}

		template< typename T = uintptr_t >
		_FORCEINLINE bool IsExecutable( T Address )
		{
			if ( !Address || !IsInside( Address ) )
				return false;

			const IMAGE_NT_HEADERS64* Nt = GetNt( );
			const IMAGE_SECTION_HEADER* Section = IMAGE_FIRST_SECTION( Nt );
			for ( uint16_t i = 0; i < Nt->FileHeader.NumberOfSections; ++i, ++Section )
				if ( Address >= Offset< T >( Section->VirtualAddress ) && Address < Offset< T >( Section->VirtualAddress + Section->Misc.VirtualSize ) )
					return Section->Characteristics & IMAGE_SCN_MEM_EXECUTE;

			return false;
		}
	};

	extern CreateWrapped( ModuleT*, Module );
	extern CreateWrapped( ModuleT*, Ntdll );
	extern CreateWrapped( ModuleT*, Win32u );
	extern CreateWrapped( ModuleT*, Kernel32 );
	extern CreateWrapped( ModuleT*, Kernelbase );
	extern CreateWrapped( ModuleT*, User32 );

	bool Setup( uint64_t Module );
}