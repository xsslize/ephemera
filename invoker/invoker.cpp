#include "invoker.hpp"

namespace Invoker
{
	CreateWrapped( uintptr_t, CallRbp );
	CreateWrapped( uintptr_t, BaseThreadInitThunkRet );
	CreateWrapped( uintptr_t, RtlUserThreadStartRet );

	bool Setup( size_t Offset )
	{
		CallRbp = Modules::Module->Offset( Offset );

		if ( *reinterpret_cast< uint16_t* >( CallRbp.Get( ) ) != ImmediateValT( uint16_t, 0xD5FF ) )
			return false; // @dev: is not call rbp

		uintptr_t RtlExitUserThread = 0;

		const IMAGE_NT_HEADERS64* Nt = Modules::Kernel32->GetNt( );
		const IMAGE_DATA_DIRECTORY* ImportDataDir = &Nt->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_IMPORT ];
		const IMAGE_IMPORT_DESCRIPTOR* ImportDir = Modules::Kernel32->Offset< IMAGE_IMPORT_DESCRIPTOR* >( ImportDataDir->VirtualAddress );

		for ( ; ImportDir->Name; ++ImportDir )
		{
			const IMAGE_THUNK_DATA64* FuncData = Modules::Kernel32->Offset< IMAGE_THUNK_DATA64* >( ImportDir->FirstThunk );
			const IMAGE_THUNK_DATA64* ThunkData = FuncData;

			if ( ImportDir->OriginalFirstThunk )
				ThunkData = Modules::Kernel32->Offset< IMAGE_THUNK_DATA64* >( ImportDir->OriginalFirstThunk );

			if ( !FuncData || !ThunkData )
				continue;

			if ( HashRuntimeCaseInsensetive( Modules::Kernel32->Offset< char* >( ImportDir->Name ) ) == FnvHash( "ntdll.dll" ) )
			{
				for ( ; ThunkData->u1.AddressOfData; ++ThunkData, ++FuncData )
				{
					if ( IMAGE_SNAP_BY_ORDINAL64( ThunkData->u1.Ordinal ) )
						continue;

					const IMAGE_IMPORT_BY_NAME* NamedImport = Modules::Kernel32->Offset< IMAGE_IMPORT_BY_NAME* >( ThunkData->u1.AddressOfData );
					if ( HashRuntime( NamedImport->Name ) == FnvHash( "RtlExitUserThread" ) )
					{
						RtlExitUserThread = reinterpret_cast< uintptr_t >( &FuncData->u1.AddressOfData );
						break;
					}
				}
			}
		}

		if ( !RtlExitUserThread )
			return false; // @dev: failed to find RtlExitUserThread address in kernel32.dll imports

		uintptr_t BaseThreadInitThunk = Modules::Kernel32->GetExport( FnvHash( "BaseThreadInitThunk" ) );

		for ( size_t i = 0; i < 0x100; ++i )
		{
			if ( *reinterpret_cast< uint16_t* >( BaseThreadInitThunk + i ) == ImmediateValT( uint16_t, 0xC88B ) )
			{
				bool RexW = *reinterpret_cast< uint8_t* >( BaseThreadInitThunk + i + 2 ) == 0x48;

				if ( ( RexW && *reinterpret_cast< uint16_t* >( BaseThreadInitThunk + i + 3 ) == ImmediateValT( uint16_t, 0x15FF ) ) ||
					   ( !RexW && *reinterpret_cast< uint16_t* >( BaseThreadInitThunk + i + 2 ) == ImmediateValT( uint16_t, 0x15FF ) ) )
				{
					size_t InstSize = RexW ? 7 : 6;
					size_t ImmOffset = RexW ? 3 : 2;

					uintptr_t callee = BaseThreadInitThunk + i + 2 + InstSize + *reinterpret_cast< int32_t* >( BaseThreadInitThunk + i + 2 + ImmOffset );
					if ( callee == RtlExitUserThread )
					{
						BaseThreadInitThunkRet = BaseThreadInitThunk + i;
						break;
					}
				}
			}
		}

		if ( !BaseThreadInitThunkRet )
			return false; // @dev: failed to find return address for BaseThreadInitThunk

		uintptr_t RtlUserThreadStart = Modules::Ntdll->GetExport( FnvHash( "RtlUserThreadStart" ) );

		for ( size_t i = 0; i < 0x100; ++i )
		{
			if ( *reinterpret_cast< uint16_t* >( RtlUserThreadStart + i ) == ImmediateValT( uint16_t, 0x15FF )
				 && *reinterpret_cast< uint8_t* >( RtlUserThreadStart + i + 6 ) == 0xEB )
			{
				RtlUserThreadStartRet = RtlUserThreadStart + i;
				break;
			}

			// @dev: differences between windows versions

			if ( *reinterpret_cast< uint8_t* >( RtlUserThreadStart + i ) == 0xE8
				 && *reinterpret_cast< uint8_t* >( RtlUserThreadStart + i + 5 ) == 0xEB )
			{
				RtlUserThreadStartRet = RtlUserThreadStart + i;
				break;
			}
		}

		if ( !RtlUserThreadStartRet )
			return false; // @dev: failed to find return address for RtlUserThreadStart

		return true;
	}
}