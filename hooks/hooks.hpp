#pragma once

#include "../utils.hpp"

#include "../invoker/invoker.hpp"

#ifndef _DEBUG
#define CreateVmtHook( Name ) Hooks::Vmt< String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) ) >( ), \
String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 2 ) >( ), String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 3 ) >( ), \
String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 4 ) >( ), String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 5 ) >( ), \
String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 6 ) >( ), String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 7 ) >( ), \
String::Key8< static_cast< uint16_t >( HashNonCrypted( #Name ) * 8 ) >( ) > Name
#else
#define CreateVmtHook( Name ) Hooks::Vmt< 0, 0, 0, 0, 0, 0, 0, 0 > Name
#endif

namespace Hooks
{
	bool SetupVmt( Modules::ModuleT* Image, void*** Instance, void**& Cave, size_t& TableSize );

	template< uint64_t OwnerKey1, uint64_t OwnerKey2, uint64_t OwnerBloat1, uint64_t OwnerBloat2,
		uint64_t OrigTableKey1, uint64_t OrigTableKey2, uint64_t OrigTableBloat1, uint64_t OrigTableBloat2 >
	class Vmt
	{
	private:
		Wrapped::CWrapped< void***, OwnerKey1, OwnerKey2, OwnerBloat1, OwnerBloat2 > Owner;
		Wrapped::CWrapped< void**, OrigTableKey1, OrigTableKey2, OrigTableBloat1, OrigTableBloat2 > OrigTable;
		size_t TableSize;

	public:
		__forceinline bool Setup( Modules::ModuleT* Image, void*** Instance )
		{
			size_t TableSz = 0;
			void** Cave = nullptr;
			if ( !SetupVmt( Image, Instance, Cave, TableSz ) )
				return false;

			Owner = Instance;
			OrigTable = *Instance;
			Utils::CopyMem( Cave, OrigTable.Get( ), TableSz * 8 );
			*Instance = Cave;
			TableSize = TableSz;

			return true;
		}

		__forceinline void Restore( )
		{
			void** ShadowTable = *Owner;
			*Owner = OrigTable.Get( );
			Utils::ZeroMem( ShadowTable, TableSize * 8 );
		}

		__forceinline void Hook( size_t Index, void* Function )
		{
			( *Owner )[ Index ] = Function;
		}

		template< typename T, typename... Args >
		__forceinline T Original( size_t Index, Args... Arguments )
		{
			return Invoker::Call< T >( OrigTable[ Index ], Arguments... );
		}
	};
}