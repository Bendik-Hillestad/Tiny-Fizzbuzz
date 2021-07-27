/*
   This is free and unencumbered software released into the public domain.

   Anyone is free to copy, modify, publish, use, compile, sell, or
   distribute this software, either in source code form or as a compiled
   binary, for any purpose, commercial or non-commercial, and by any
   means.

   In jurisdictions that recognize copyright laws, the author or authors
   of this software dedicate any and all copyright interest in the
   software to the public domain. We make this dedication for the benefit
   of the public at large and to the detriment of our heirs and
   successors. We intend this dedication to be an overt act of
   relinquishment in perpetuity of all present and future rights to this
   software under copyright law.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
   EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
   MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
   IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
   OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
   ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
   OTHER DEALINGS IN THE SOFTWARE.
*/

/*
┌─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
|                                                                                                                     |
| Author: Bendik Hillestad                                                                                            | 
|                                                                                                                     |
| This is a fairly small (in terms of final executable size) solution to the FizzBuzz problem.                        |
|                                                                                                                     |
| As a refresher, the FizzBuzz problem is as follows:                                                                 |
|   Go through integers from 1 to N, and for each integer print "Fizz" if it is divisible by 3, "Buzz" if it is       |
|   divisible by 5, "FizzBuzz" if it is divisible by both, otherwise just print the number.                           |
|                                                                                                                     |
| The classic implementation looks something like this:                                                               |
|   for (int i = 1; i <= N; i++)                                                                                      |
|   {                                                                                                                 |
|       if ((i % 3 == 0) && (i % 5 == 0))                                                                             |
|       {                                                                                                             |
|           printf("FizzBuzz\n");                                                                                     |
|       }                                                                                                             |
|       else if (i % 3 == 0)                                                                                          |
|       {                                                                                                             |
|           printf("Fizz\n");                                                                                         |
|       }                                                                                                             |
|       else if (i % 5 == 0)                                                                                          |
|       {                                                                                                             |
|           printf("Buzz\n");                                                                                         |
|       }                                                                                                             |
|       else                                                                                                          |
|       {                                                                                                             |
|           printf("%d\n", i);                                                                                        |
|       }                                                                                                             |
|   }                                                                                                                 |
|                                                                                                                     |
| When compiled with no optimizations, this typically results in a massive binary with a size somewhere around 50kb.  |
| When compiled with reasonable optimization flags, this results in a binary with a size somewhere between 9kb - 11kb |
| depending on implementation details. Our implementation combined with some more advanced flags results in a binary  |
| with a size of 800 bytes, which is roughly a 90% size reduction compared to the optimized reference.                |
|                                                                                                                     |
| To achieve our small size compared to the reference, there are a few tricks we use:                                 |
|  - We completely eliminate the use of the C Runtime and any other helper code that the compiler produces for us.    |
|    This means certain features are unavailable to us, unless we implement these features ourselves, but we do not   |
|    really need them. The main thing we need, which is printing something to the console, can be implemented with a  |
|    small amount of code and a call to the win32 function WriteFile.                                                 |
|  - We also provide some flags that allow the compiler to produce a smaller executable. Some of these flags are also |
|    required for our trick above, but many of the flags further reduce the size by removing data we do not need, or  |
|    by allowing the compiler to use smaller instructions to address memory. We also allow the executable to be       |
|    packed better by specifying a section alignment of 16. These two tricks get us down to roughly 1.5kb.            |
|  - The most advanced trick we employ is to eliminate the import table and load functions ourselves using hashes.    |
|    Considering how few functions we need, this trick does not make that big of a difference, but it gets us another |
|    200 bytes or so. It mainly adds to the WTF-factor when reading the code. It should be noted that this trick is   |
|    not necessarily compatible with future versions of Windows.                                                      |
|  - We keep all our data on the stack or embedded as immediate values in the resulting assembly code, which helps    |
|    eliminate some sections in our executable. Combined with our advanced flags, including some undocumented stuff,  |
|    we are left with only the .text section. Avoiding extra sections is mainly useful to get rid of padding.         |
|  - Lastly, we try to write code that translates well into small assembly code, and we take advantage of a repeating |
|    pattern in FizzBuzz to eliminate divisibility calculations. At this point we are fighting the section alignment, |
|    so a small change can be amplified this way.                                                                     |
|                                                                                                                     |
└─────────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
*/

#define _WIN32_WINNT _WIN32_WINNT_WIN7
#define STRICT
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <winternl.h>
#include <intrin.h>

#if !defined(_M_IX86)
#   error 32-bit build only
#endif

#if !defined(CRINKLER)
struct REAL_PEB_LDR_DATA
{
    ULONG      Length;
    BOOLEAN    Initialized;
    PVOID      SsHandle;
    LIST_ENTRY InLoadOrderModuleList;
    LIST_ENTRY InMemoryOrderModuleList;
    LIST_ENTRY InInitializationOrderModuleList;
    PVOID      EntryInProgress;
};
#endif

using u8  = unsigned __int8;
using u32 = unsigned __int32;
using i32 = signed   __int32;

static constexpr u32 hash(char const* str)
{
    u32 hash{ 0 };
    while (*str)
    {
        hash ^= static_cast<u8>(*str);
        hash = (hash >> 26) | (hash << 6);
        ++str;
    }
    return hash;
}

extern "C" __declspec(noreturn) void __stdcall _main()
{
    #if !defined(CRINKLER)
    // Prepare loading the functions we need from kernel32.
    constexpr u32 function_hashes[3]
    {
        hash("WriteFile"),
        hash("GetStdHandle"),
        hash("VirtualAlloc")
    };
    void(*function_table[3])();

    #define WriteFile ((BOOL(WINAPI*)(HANDLE, LPCVOID, DWORD, LPDWORD, LPOVERLAPPED))(function_table[0]))
    #define GetStdHandle ((HANDLE(WINAPI*)(DWORD))(function_table[1]))
    #define VirtualAlloc ((LPVOID(WINAPI*)(LPVOID, SIZE_T, DWORD, DWORD))(function_table[2]))

    // Find where kernel32.dll (or kernelbase.dll) is located in memory.
    DWORD const module = reinterpret_cast<DWORD>(
        reinterpret_cast<LDR_DATA_TABLE_ENTRY const*>(
            reinterpret_cast<DWORD>(
                reinterpret_cast<REAL_PEB_LDR_DATA const*>(
                    reinterpret_cast<PEB const*>(
                        __readfsdword(0x30)
                    )->Ldr
                )->InInitializationOrderModuleList.Flink->Flink
            ) - (2 * sizeof(LIST_ENTRY))
        )->DllBase
    );

    // Find the export header so we can iterate through the functions.
    IMAGE_EXPORT_DIRECTORY const* const export_header = reinterpret_cast<IMAGE_EXPORT_DIRECTORY const*>(
        reinterpret_cast<IMAGE_NT_HEADERS32 const*>(
            reinterpret_cast<IMAGE_DOS_HEADER const*>(module)->e_lfanew + module
        )->OptionalHeader.DataDirectory[0].VirtualAddress + module
    );

    // Go through each function.
    for (i32 i{ static_cast<i32>(export_header->NumberOfFunctions) }; --i >= 0;)
    {
        // Get the name of the function.
        char const* function_name = reinterpret_cast<char const*>(
            reinterpret_cast<DWORD*>(
                export_header->AddressOfNames + module
            )[i] + module
        );

        // Hash the function name.
        u32 hash{ 0 };
        while (*function_name != 0)
        {
            hash ^= static_cast<u8>(*function_name);
            hash  = _rotl(hash, 6);
            ++function_name;
        }

        // Go through the hashes of the functions we want to load.
        for (i32 j{ sizeof(function_table) / sizeof(void(*)()) }; --j >= 0;)
        {
            // Check if the hash matches.
            if (hash == function_hashes[j])
            {
                // Get the actual index we need to get the address of the function (in some cases the same as 'i', but not always).
                WORD const index{ reinterpret_cast<WORD*>(export_header->AddressOfNameOrdinals + module)[i] };

                // Get the address of the function.
                function_table[j] = reinterpret_cast<void(*)()>(
                    reinterpret_cast<DWORD*>(
                        export_header->AddressOfFunctions + module
                    )[index] + module
                );
            }
        }
    }
    #endif

    // Configure the number of integers we will go through.
    constexpr u32 N{ 100 };

    // Allocate a buffer that will hold our output so that we can perform just one call to WriteFile.
    char* buf = static_cast<char*>(VirtualAlloc(nullptr, (11 * N), MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE));
    __analysis_assume(buf != nullptr);
    char* p = buf++;

    // FizzBuzz follows a pattern that repeats every 15 iterations, so we can skip the divisibility calculations
    // and instead encode the pattern in this bitstring.
    u32 pattern{ 0b000001001001000001100001000011U };

    // Go through all the integers from 1 to N.
    i32 i{ 0 };
    do
    {
        // Get the next integer and move through the pattern.
        ++i; pattern = (pattern << 28) | (pattern >> 2);

        // Push a new line.
        *p++ = '\n';

        // Check the pattern and determine if we need to push Fizz and/or Buzz.
        if (pattern & 1)
        {
            *reinterpret_cast<i32*>(p) = 'zziF';
            p += 4;
        }
        if (pattern & 2)
        {
            *reinterpret_cast<i32*>(p) = 'zzuB';
            p += 4;
        }

        // Check if we need to push the number.
        if ((pattern & 3) == 0)
        {
            // Go through each digit in the number and push it.
            i32 div{ 1'000'000'000 };
            while (div != 0)
            {
                if ((i / div) != 0)
                {
                    *p++ = '0' + ((i / div) % 10);
                }

                div /= 10;
            }
        }
    } while (i < N);

    // Write our output to the console.
    WriteFile(GetStdHandle(STD_OUTPUT_HANDLE), buf, (p - buf), nullptr, nullptr);
    
    // Exit.
    __fastfail(FAST_FAIL_FATAL_APP_EXIT);
    __assume(0);
}
