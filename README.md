# ephemera

Provides obfuscated string literals, immediate values, fnv-1a hashes, and wrapped scalar or pointer storage. Constants are transformed during compilation and reconstructed only when used at runtime

## Features

- Compile-time transformation of narrow and wide string literals
- Obfuscated 64-bit immediate values
- Wrapped storage for integral values and pointers
- SSE2-based block reconstruction
- No dynamic allocation
- No dependency on the C++ runtime library

The implementation assumes a 64-bit `uintptr_t` and is not intended for x86 or ARM targets

## Usage

Add `KEYSEED` as a string literal in the project settings:
```text
C/C++ -> Preprocessor -> Preprocessor Definitions -> KEYSEED="Seed"
```

### String literals
```cpp
auto* Module = Utils::GetModuleEntry( _( L"module.exe" ) );
```

The returned string pointer is valid until the end of the current expression. Consume it immediately and do not store it for later use

### Wrapped values

Declare the value in a header:
```cpp
inline CreateWrapped( void*, ModuleBase );
```

Assign and retrieve it normally:
```cpp
ModuleBase = Something;
auto* Base = ModuleBase.Get( );
```

Wrapped values must be assigned before they are read

### Immediate values

```cpp
auto EntrySize = ImmediateValT( size_t, 0x30 );
auto Value = ImmediateVal( 0x12345678ull );
```

## Assembly output

For example, this expression:

```cpp
WrappedState = ImmediateValT( uintptr_t, 0x13579BDF2468ACE0ull );
```

is inlined by MSVC into the following reconstruction sequence:

```asm
mov     rax, 11F06CF1F2481859h
mov     [var_A0], rax
mov     rax, 6526EA2D8DC5FBEDh
mov     [var_A0+8], rax

mov     rax, 9D0F18079668C3CEh
mov     [storage], rax
mov     rax, 898E9EDBDA89A8F2h
mov     [storage+8], rax

movdqu  xmm1, [var_A0]
movdqu  xmm0, [storage]
xorps   xmm1, xmm0
andnps  xmm1, [all_bits]

movdqa  xmm0, xmm1
psrldq  xmm0, 8
movq    rax, xmm0
and     r11, rax
movq    rax, xmm1
and     rax, rcx
shl     rax, 10h
or      r11, rax
```

The original immediate `0x13579BDF2468ACE0` is not stored directly in the generated code. The exact constants and register allocation depend on `KEYSEED`, compiler version and optimization settings.

## User mode and LLVM

To use the library in a user-mode project with clang-cl / llvm:

- Replace `ntddk.h` with the required standard and intrinsic headers
- Use `<type_traits>` and `<utility>` instead of the kernel-side replacements in `namespace std`
- Replace `BitCast` with `std::bit_cast` from `<bit>`
- Use `__m128i_u` for unaligned SIMD loads and stores
- Replace the volatile register barrier with LLVM inline assembly

```cpp
__forceinline uint64_t ForceFromReg( uint64_t Value ) noexcept
{
    asm( "" : "=r"( Value ) : "0"( Value ) );
    return Value;
}
```

Apply the same register barrier to the 32-bit variants. The user-mode build remains x64-only and still requires `KEYSEED`
