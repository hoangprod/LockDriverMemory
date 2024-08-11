#pragma once
#include <ntifs.h>
#include <windef.h>

#include <intrin.h>
#include "Typedef.h"
#include "Global.h"


#pragma region MacroDefinitions

#	define DBP( Fmt, ... )											\
			{																\
				DbgPrintEx(0, 0, "[<%d %s>] "##Fmt "\n", __LINE__, __FUNCTION__, __VA_ARGS__);					\
			}

#	define DBE( Fmt, ... )											\
			{																\
				DbgPrintEx(0, 0, "[!ERROR! <%d %s>] "##Fmt "\n", __LINE__, __FUNCTION__, __VA_ARGS__);				\
			}

#	define DBEXC( Fmt, ... )											\
			{																\
				DbgPrintEx(0, 0, "[!ERROR! <%d %s>] "##Fmt "\n", __LINE__, __FUNCTION__, __VA_ARGS__);				\
			}

#define ReCa  reinterpret_cast
#define RePV  reinterpret_cast<PVOID>
#define ReI64 reinterpret_cast<INT_PTR>
#define ReU64 reinterpret_cast<UINT_PTR>
#define Re32  reinterpret_cast<DWORD>

#define IOCTL(Code, Method)			(CTL_CODE(0x8000, (Code), Method, FILE_ANY_ACCESS))
#define EXTRACT_CTL_CODE(Ioctl)		((unsigned short)(((Ioctl) & 0b0011111111111100) >> 2))
#define EXTRACT_CTL_METHOD(Ioctl)	((unsigned short)((Ioctl) & 0b11))

#define BYTEn(x, n)   (*((BYTE*)&(x)+n))

#define IMAGE_FIRST_SECTION( ntheader ) ((PIMAGE_SECTION_HEADER)  \
    ((ULONG_PTR)(ntheader) +                                      \
     FIELD_OFFSET( IMAGE_NT_HEADERS64, OptionalHeader ) +           \
     ((ntheader))->FileHeader.SizeOfOptionalHeader   \
    ))


#define IMAGE32(hdr) (hdr->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR32_MAGIC)
#define IMAGE64(hdr) (hdr->OptionalHeader.Magic == IMAGE_NT_OPTIONAL_HDR64_MAGIC)

#define HEADER_VAL_T(hdr, val) (IMAGE64(hdr) ? ((PIMAGE_NT_HEADERS64)hdr)->OptionalHeader.val : ((PIMAGE_NT_HEADERS32)hdr)->OptionalHeader.val)
#define THUNK_VAL_T(hdr, ptr, val) (IMAGE64(hdr) ? ((PIMAGE_THUNK_DATA64)ptr)->val : ((PIMAGE_THUNK_DATA32)ptr)->val)
#define TLS_VAL_T(hdr, ptr, val) (IMAGE64(hdr) ? ((PIMAGE_TLS_DIRECTORY64)ptr)->val : ((PIMAGE_TLS_DIRECTORY32)ptr)->val)
#define CFG_DIR_VAL_T(hdr, dir, val) (IMAGE64(hdr) ? ((PIMAGE_LOAD_CONFIG_DIRECTORY64)dir)->val : ((PIMAGE_LOAD_CONFIG_DIRECTORY32)dir)->val)

// Signatures and magic numbers
//
#define IMAGE_DOS_SIGNATURE         0x5A4D //MZ
#define IMAGE_NT_SIGNATURE          0x00004550 //PE00
#define IMAGE_SCN_MEM_DISCARDABLE   0x02000000
#define IMAGE_SCN_MEM_WRITE         0x80000000
#define IMAGE_SCN_MEM_READ          0x40000000
#define IMAGE_SCN_MEM_EXECUTE       0x20000000

// Directory Entries
//
#define IMAGE_DIRECTORY_ENTRY_EXPORT          0   // Export Directory
#define IMAGE_DIRECTORY_ENTRY_IMPORT          1   // Import Directory
#define IMAGE_DIRECTORY_ENTRY_RESOURCE        2   // Resource Directory
#define IMAGE_DIRECTORY_ENTRY_EXCEPTION       3   // Exception Directory
#define IMAGE_DIRECTORY_ENTRY_SECURITY        4   // Security Directory
#define IMAGE_DIRECTORY_ENTRY_BASERELOC       5   // Base Relocation Table
#define IMAGE_DIRECTORY_ENTRY_DEBUG           6   // Debug Directory
#define IMAGE_DIRECTORY_ENTRY_ARCHITECTURE    7   // Architecture Specific Data
#define IMAGE_DIRECTORY_ENTRY_GLOBALPTR       8   // RVA of GP
#define IMAGE_DIRECTORY_ENTRY_TLS             9   // TLS Directory
#define IMAGE_DIRECTORY_ENTRY_LOAD_CONFIG    10   // Load Configuration Directory
#define IMAGE_DIRECTORY_ENTRY_BOUND_IMPORT   11   // Bound Import Directory in headers
#define IMAGE_DIRECTORY_ENTRY_IAT            12   // Import Address Table
#define IMAGE_DIRECTORY_ENTRY_DELAY_IMPORT   13   // Delay Load Import Descriptors
#define IMAGE_DIRECTORY_ENTRY_COM_DESCRIPTOR 14   // COM Runtime descriptor

#define IMAGE_ORDINAL_FLAG64 0x8000000000000000
#define IMAGE_ORDINAL_FLAG32 0x80000000
#define IMAGE_ORDINAL64(Ordinal) (Ordinal & 0xffff)
#define IMAGE_ORDINAL32(Ordinal) (Ordinal & 0xffff)
#define IMAGE_SNAP_BY_ORDINAL64(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG64) != 0)
#define IMAGE_SNAP_BY_ORDINAL32(Ordinal) ((Ordinal & IMAGE_ORDINAL_FLAG32) != 0)

#define IMAGE_NT_OPTIONAL_HDR32_MAGIC           0x10b
#define IMAGE_NT_OPTIONAL_HDR64_MAGIC           0x20b

#define LDRP_RELOCATION_INCREMENT               0x1
#define LDRP_RELOCATION_FINAL                   0x2

#define IMAGE_REL_BASED_ABSOLUTE                0
#define IMAGE_REL_BASED_HIGH                    1
#define IMAGE_REL_BASED_LOW                     2
#define IMAGE_REL_BASED_HIGHLOW                 3
#define IMAGE_REL_BASED_HIGHADJ                 4
#define IMAGE_REL_BASED_MIPS_JMPADDR            5
#define IMAGE_REL_BASED_SECTION                 6
#define IMAGE_REL_BASED_REL32                   7
#define IMAGE_REL_BASED_MIPS_JMPADDR16          9
#define IMAGE_REL_BASED_IA64_IMM64              9
#define IMAGE_REL_BASED_DIR64                   10

#define IMAGE_SIZEOF_BASE_RELOCATION            8



#define GUARD_TRY()		__try {

#define GUARD_END()		} \
						__except (EXCEPTION_EXECUTE_HANDLER) { \
						}

#define SEH_START()		ULONG ExceptionCode = 0; \
						PEXCEPTION_POINTERS ExceptionPointers = 0;\
                        __try {


#define SEH_END()	} \
						__except (ExceptionCode = GetExceptionCode(), ExceptionPointers = GetExceptionInformation(), EXCEPTION_EXECUTE_HANDLER ) { \
							if (ExceptionPointers) \
							{ \
								DBEXC("Exception (%lx) caught in %s @ (%p : %llx : %llx)",\
								ExceptionCode, __FUNCTION__, \
								ExceptionPointers->ExceptionRecord->ExceptionAddress, \
								G::DriverImageBase, \
								(UINT_PTR)ExceptionPointers->ExceptionRecord->ExceptionAddress - G::DriverImageBase \
								); \
							}\
                        }

#pragma endregion