#include "hooks.hpp"

namespace Hooks
{
	bool SetupVmt( Modules::ModuleT* Image, void*** Instance, void**& Cave, size_t& TableSize )
	{
		void** Table = *Instance;

		while ( true )
		{
			if ( !Image->IsExecutable( *Table ) )
				break;

			++TableSize;
			++Table;
		}

		const IMAGE_NT_HEADERS64* Nt = Image->GetNt( );
		const IMAGE_SECTION_HEADER* Section = IMAGE_FIRST_SECTION( Nt );

		for ( uint16_t i = 0; i < Nt->FileHeader.NumberOfSections; ++i, ++Section )
		{
			if ( Section->Characteristics & IMAGE_SCN_MEM_WRITE )
			{
				Cave = const_cast< void** >( Image->Offset< void** >( Section->VirtualAddress + ( ( Section + 1 )->VirtualAddress - Section->VirtualAddress ) - 8 ) );

				for ( size_t i = 0; i < TableSize && Cave > Image->Offset< void** >( Section->VirtualAddress ); )
				{
					if ( !*Cave )
					{
						++i;
						--Cave;
					}
					else
					{
						--Cave;
						i = 0;
					}
				}

				if ( Cave <= Image->Offset< void** >( Section->VirtualAddress ) )
					Cave = nullptr;
				else
					break;
			}
		}

		if ( !Cave )
			return false; // @dev: failed to find data cave in image

		++Cave;

		return true;
	}
}