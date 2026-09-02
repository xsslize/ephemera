#include "../invoker/invoker.hpp"

namespace Modules
{
	CreateWrapped( ModuleT*, Module );
	CreateWrapped( ModuleT*, Ntdll );
	CreateWrapped( ModuleT*, Win32u );
	CreateWrapped( ModuleT*, Kernel32 );
	CreateWrapped( ModuleT*, Kernelbase );
	CreateWrapped( ModuleT*, User32 );

	uintptr_t GetModule( uint64_t ModuleHash )
	{
		const PebT* Peb = PebT::Current( );
		const PebLdrDataT* Ldr;
		if ( !Peb || !( Ldr = Peb->Ldr ) )
			return 0;

		for ( const ListEntryT* ListEntry = Ldr->InMemoryOrderModuleList.Flink; ListEntry != &Ldr->InMemoryOrderModuleList; ListEntry = ListEntry->Flink )
		{
			const LdrDataTableEntryT* Entry = CONTAINING_RECORD( ListEntry, const LdrDataTableEntryT, InMemoryOrderLinks );

			if ( HashRuntimeCaseInsensetive( Entry->BaseDllName.Buffer ) == ModuleHash )
				return Entry->DllBase;
		}

		return 0;
	}

	uintptr_t ModuleT::GetExport( uint64_t ExportHash )
	{
		const IMAGE_EXPORT_DIRECTORY* ExportDir = Offset< IMAGE_EXPORT_DIRECTORY* >( GetNt( )->OptionalHeader.DataDirectory[ IMAGE_DIRECTORY_ENTRY_EXPORT ].VirtualAddress );
		const uint32_t* FunctionTable = Offset< uint32_t* >( ExportDir->AddressOfFunctions );
		const uint32_t* NamesTable = Offset< uint32_t* >( ExportDir->AddressOfNames );
		const uint16_t* OrdinalsTable = Offset< uint16_t* >( ExportDir->AddressOfNameOrdinals );

		for ( uint32_t i = 0; i < ExportDir->NumberOfNames; ++i )
			if ( HashRuntime( Offset< char* >( NamesTable[ i ] ) ) == ExportHash )
				return Offset( FunctionTable[ OrdinalsTable[ i ] ] );

		return 0;
	}

	bool Setup( uint64_t OwnModule )
	{
		Module = GetModule( OwnModule );
		Ntdll = GetModule( FnvHash( "ntdll.dll" ) );
		Win32u = GetModule( FnvHash( "win32u.dll" ) );
		Kernel32 = GetModule( FnvHash( "kernel32.dll" ) );
		Kernelbase = GetModule( FnvHash( "kernelbase.dll" ) );
		User32 = GetModule( FnvHash( "user32.dll" ) );

		return
			Module &&
			Ntdll &&
			Win32u &&
			Kernel32 &&
			Kernelbase &&
			User32;
	}
}