#pragma once

#include "../utils.hpp"

namespace Invoker
{
	extern CreateWrapped( uintptr_t, CallRbp );
	extern CreateWrapped( uintptr_t, BaseThreadInitThunkRet );
	extern CreateWrapped( uintptr_t, RtlUserThreadStartRet );

	constexpr size_t ShadowStackSize = 0x198;

	bool Setup( size_t Offset );

	template< typename T >
	__forceinline uint64_t ZeroExtend( T Value )
	{
		return ( uint64_t ) Value;
	}

	template< typename T = uintptr_t, typename FunctionType = uintptr_t, typename... Args >
	_FORCEINLINE T Call( FunctionType Function, Args... Arguments )
	{
		#ifdef _DEBUG
		return reinterpret_cast< T( * )( Args... ) >( Function )( Arguments... );
		#else

		volatile struct
		{
			uintptr_t CallRbp;							  // 0x0
			uintptr_t BaseThreadInitThunkRet;	          // 0x8
			uintptr_t RtlUserThreadStartRet;		      // 0x10
			uintptr_t StackPointer;						  // 0x18
			uint64_t  Rcx;								  // 0x20
			uint64_t  Rdx;								  // 0x28
			uint64_t  R8;							      // 0x30
			uint64_t  R9;								  // 0x38
			uint64_t  RbpBackup;						  // 0x40
			uint64_t  R15Backup;						  // 0x48
			uintptr_t Function;							  // 0x50
			uint64_t  OutValue;							  // 0x58
			uintptr_t RspBackup;						  // 0x60
		} AsmParams;

		Utils::ZeroMem( ( uintptr_t ) &AsmParams, sizeof( AsmParams ) );

		AsmParams.CallRbp = CallRbp.Get( );
		AsmParams.BaseThreadInitThunkRet = BaseThreadInitThunkRet.Get( );
		AsmParams.RtlUserThreadStartRet = RtlUserThreadStartRet.Get( );
		AsmParams.Function = ( uintptr_t ) Function;

		uintptr_t StackPointer = 0;
		asm volatile( "movq %%rsp, %0" :"=r"( StackPointer ) : : "memory" );
		uintptr_t StackBackup = StackPointer;
		StackPointer -= ShadowStackSize;
		StackPointer &= 0xFFFFFFFFFFFFFFE0;
		StackPointer += 8;
		size_t StackSize = StackBackup - StackPointer;
		Utils::ZeroMem( StackPointer, StackSize );

		AsmParams.StackPointer = StackPointer - 0x20;

		if ( constexpr uint64_t ArgsCount = sizeof...( Arguments ) )
		{
			uint64_t ArgsArray[ ArgsCount ] = { ZeroExtend( Arguments )... };
			if constexpr ( constexpr size_t arguments_on_stack = ArgsCount > 4 ? ArgsCount - 4 : 0 )
				Utils::CopyMem( StackPointer + 0x28, &ArgsArray[ 4 ], arguments_on_stack * 8 );

			AsmParams.Rcx = ArgsArray[ 0 ];

			if constexpr ( ArgsCount > 1 )
				AsmParams.Rdx = ArgsArray[ 1 ];
			if constexpr ( ArgsCount > 2 )
				AsmParams.R8 = ArgsArray[ 2 ];
			if constexpr ( ArgsCount > 3 )
				AsmParams.R9 = ArgsArray[ 3 ];
		}

		asm volatile(
			  "movq %[input_params], %%rax\n\t"
			  "movq %%r15, 0x48(%%rax)\n\t"
			  "movq %%rax, %%r15\n\t"
			  "movq %%rsp, 0x60(%%r15)\n\t"
			  "movq %%rbp, 0x40(%%r15)\n\t"
			  "movq 0x20(%%r15), %%rcx\n\t"
			  "movq 0x28(%%r15), %%rdx\n\t"
			  "movq 0x30(%%r15), %%r8\n\t"
			  "movq 0x38(%%r15), %%r9\n\t"
			  "movq 0x18(%%r15), %%rsp\n\t"
			  "addq $0x20, %%rsp\n\t"
			  "movq (%%r15), %%rax\n\t"
			  "movq %%rax, (%%rsp)\n\t"

			  "movq 0x8(%%r15), %%rax\n\t"
			  "movq %%rax, 0xA0(%%rsp)\n\t"

			  "movq 0x10(%%r15), %%rax\n\t"
			  "movq %%rax, 0xD0(%%rsp)\n\t"

			  "leaq 0x5(%%rip), %%rbp\n\t"
			  "pushq 0x50(%%r15)\n\t"
			  "ret\n\t"
			  "movq 0x60(%%r15), %%rsp\n\t"
			  "movq %%rax, 0x58(%%r15)\n\t"
			  "movq 0x40(%%r15), %%rbp\n\t"
			  "movq 0x48(%%r15), %%r15"
			  :
		: [ input_params ] "r"( &AsmParams )
			: "memory", "cc", "rax", "rcx", "rdx", "r8", "r9", "r10", "r11", "xmm0", "xmm1", "xmm2", "xmm3", "xmm4", "xmm5"
			);

		T OutValue = ( T ) AsmParams.OutValue;

		Utils::ZeroMem( ( uintptr_t ) &AsmParams, sizeof( AsmParams ) );
		Utils::ZeroMem( StackPointer, StackSize );

		return OutValue;
		#endif
	}
}