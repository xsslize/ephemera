#include "../hooks/hooks.hpp"

// @dev: hooks example
[[gnu::naked]] void HookTestRttiProxy( )
{
	asm volatile(
		  "subq $0x38, %rsp\n\t"
		  "movq %rcx, 0x20(%rsp)\n\t"
		  "movq %rdx, 0x28(%rsp)\n\t"
		  "movq %rbp, %rcx\n\t"
		  "movq %r15, %rdx\n\t"
		  "movq %rsi, %r8\n\t"
		  "movq %r14, %r9\n\t"
		  "call HookTestRtti\n\t"
		  "addq $0x38, %rsp\n\t"
		  "ret"
		  );
}

namespace Engine
{
	CreateWrapped( uintptr_t*, InputDevice );

	void Setup( )
	{
		InputDevice = *Modules::Module->Offset< uintptr_t** >( 0x34DA88 );
	}
}

CreateVmtHook( InputDeviceHook );

bool SetupHooks( )
{
	if ( !InputDeviceHook.Setup( Modules::Module, reinterpret_cast< void*** >( Engine::InputDevice.Get( ) ) ) )
		return false;

	InputDeviceHook.Hook( 0, reinterpret_cast< void* >( HookTestRttiProxy ) );

	return true;
}

extern "C" uint8_t HookTestRtti( HWND Window, uint32_t Message, uint64_t WParam, int64_t LParam, uintptr_t This, uintptr_t Rtti )
{
	uint8_t Result = InputDeviceHook.Original< uint8_t >( 0, This, Rtti );
	return Result;
}

#ifdef _DEBUG
int DllMain( HMODULE Base, uint32_t Reason, void* )
{
    if ( Reason != DLL_PROCESS_ATTACH )
        return FALSE;

    if ( Modules::Setup( FnvHash( "App.exe" ) ) && Invoker::Setup( ImmediateVal( 0x13E67 ) ) )
    {
        Invoker::Call( Modules::User32->GetExport( FnvHash( "MessageBoxA" ) ), nullptr, _( "Hello world!" ), _( "Information" ), MB_ICONINFORMATION );

		if ( !SetupHooks( ) )
			return FALSE;
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

		if ( !SetupHooks( ) )
			return FALSE;
    }
}
#endif