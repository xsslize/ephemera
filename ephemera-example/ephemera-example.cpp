#include "../invoker/invoker.hpp"

#ifdef _DEBUG
int DllMain( HMODULE Base, uint32_t Reason, void* )
{
    if ( Reason != DLL_PROCESS_ATTACH )
        return FALSE;

    if ( Modules::Setup( FnvHash( "App.exe" ) ) && Invoker::Setup( ImmediateVal( 0x13E67 ) ) )
    {
        Invoker::Call( Modules::User32->GetExport( FnvHash( "MessageBoxA" ) ), nullptr, _( "Hello world!" ), _( "Information" ), MB_ICONINFORMATION );
    }
}
#else
int Entry( HMODULE Base, uint32_t Reason, void* )
{
    if ( Reason != DLL_PROCESS_ATTACH )
        return FALSE;

    if ( Modules::Setup( FnvHash( "App.exe" ) ) && Invoker::Setup( ImmediateVal( 0x13E67 ) ) )
    {
        Invoker::Call( Modules::User32->GetExport( FnvHash( "MessageBoxA" ) ), nullptr, _( "Hello world!" ), _( "Information" ), MB_ICONINFORMATION );
    }
}
#endif