#include "memory.hpp"

#include "../importer/importer.hpp"
#include "../../globals/globals.hpp"

namespace memory {
	bool is_valid( const uintptr_t address ) {
		if ( !address )
			return false;

		const auto result = call( MmIsAddressValid, reinterpret_cast< void* >( address ) );

		if ( !result )
			return false;

		return true;
	}

	bool is_valid( const void* address ) {
		if ( !address )
			return false;

		const auto result = call( MmIsAddressValid, const_cast< void* >( address ) );

		if ( !result )
			return false;

		return true;
	}

	bool hook( const wchar_t* name, uintptr_t function, PDRIVER_DISPATCH& original ) {
		const auto type = take( IoDriverObjectType );

		if ( !type )
			return false;

		UNICODE_STRING object_name{ };
		winapi::init_unicode_string( &object_name, name );

		PDRIVER_OBJECT driver_object{ };
		call( ObReferenceObjectByName, &object_name, OBJ_CASE_INSENSITIVE, 0, 0, *type, KernelMode, 0, &driver_object );

		PHYSICAL_ADDRESS address{ .QuadPart = -1 };

		const auto allocated = reinterpret_cast< uintptr_t >( call( MmAllocateContiguousMemory, 0x1000, address ) );

		if ( !allocated )
			return false;

		original = driver_object->MajorFunction[ 14 ];

		*( DWORD* )allocated = 0x8C48348;
		*( DWORD* )( allocated + 0x4 ) = 0x25FF;
		*( WORD* )( allocated + 0x8 ) = 0;
		*( uintptr_t* )( allocated + 0xA ) = function;

		driver_object->HardwareDatabase = ( PUNICODE_STRING )allocated;

		InterlockedExchange64( ( volatile __int64* )&driver_object->MajorFunction[ 14 ], globals::trampoline );

		return true;
	}
}