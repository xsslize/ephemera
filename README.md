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