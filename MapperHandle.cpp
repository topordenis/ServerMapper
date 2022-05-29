#pragma once


#include "MapperHandle.h"
#include <map>
#include <fstream>

#include <Windows.h>
#include "Utils.h"

#define MakePtr( cast, ptr, addValue ) (cast)( (DWORD_PTR)(ptr) + (DWORD_PTR)(addValue))

//   This one is mine, but obviously..."adapted" from matt's original idea =p
#define MakeDelta(cast, x, y) (cast) ( (DWORD_PTR)(x) - (DWORD_PTR)(y))

std::uint64_t * GetPtrFromRVA ( std::uint64_t m_dwRVA, LIMAGE_NT_HEADERS * m_pNtHeaders, uint8_t * m_aImage ) {
	auto GetSectionHeader = [ m_dwRVA, m_pNtHeaders ] ( ) -> LIMAGE_SECTION_HEADER * {
		LIMAGE_SECTION_HEADER * m_pSection = LIMAGE_FIRST_SECTION ( m_pNtHeaders );
		for ( int i = 0; i < m_pNtHeaders->FileHeader.NumberOfSections; i++, m_pSection++ ) {
			std::uint64_t m_dwSize = m_pSection->Misc.VirtualSize;
			if ( !m_dwSize )
				m_dwSize = m_pSection->SizeOfRawData;

			if ( ( m_dwRVA >= m_pSection->VirtualAddress ) && ( m_dwRVA < ( m_pSection->VirtualAddress + m_dwSize ) ) )
				return m_pSection;
		}

		return nullptr;
	};

	LIMAGE_SECTION_HEADER * m_pSectionHeader = GetSectionHeader ( );
	if ( !m_pSectionHeader )
		return nullptr;

	auto m_dwDelta = ( std::uint64_t ) ( m_pSectionHeader->VirtualAddress - m_pSectionHeader->PointerToRawData );
	return ( std::uint64_t * ) ( m_aImage + m_dwRVA - m_dwDelta );
}

ImageMap::ImageMap ( MapHandle * hdl ) {
	parentHandle = hdl;

	binary = hdl->binary;

	auto dosHeaders = reinterpret_cast< LIMAGE_DOS_HEADER * > ( binary.data ( ) );
	ntHd = reinterpret_cast< LIMAGE_NT_HEADERS * > ( binary.data ( ) + dosHeaders->e_lfanew );
	impDesc = reinterpret_cast< LIMAGE_IMPORT_DESCRIPTOR * >( GetPtrFromRVA
	(
		ntHd->OptionalHeader.DataDirectory [ /*IMAGE_DIRECTORY_ENTRY_IMPORT*/ 1 ].VirtualAddress,
		ntHd,
		binary.data ( )
	) );


	reloc = ( LIMAGE_BASE_RELOCATION * ) GetPtrFromRVA (
		( DWORD ) ( ntHd->OptionalHeader.DataDirectory [ /*IMAGE_DIRECTORY_ENTRY_BASERELOC*/ 5 ].VirtualAddress ),
		ntHd,
		binary.data ( ) );


}
bool ImageMap::FixImports ( ) {
	if ( clientImports.empty ( ) ) {
		std::cout << "clientImports is empty. This means client didnt sent his imports addresses." << std::endl;

		return false;
	}

	char * mod;

	int i = 0;


	//   Loop through all the required modules
	while ( ( mod = ( char * ) GetPtrFromRVA ( ( DWORD ) ( impDesc->Name ), ntHd, binary.data ( ) ) ) ) {

		//HMODULE localMod = LoadLibrary ( mod );


		LIMAGE_THUNK_DATA * itd =
			( LIMAGE_THUNK_DATA * ) GetPtrFromRVA ( ( DWORD ) ( impDesc->FirstThunk ), ntHd, binary.data ( ) );

		while ( itd->u1.AddressOfData ) {

			LIMAGE_IMPORT_BY_NAME * iibn;
			iibn = ( LIMAGE_IMPORT_BY_NAME * ) GetPtrFromRVA ( ( DWORD ) ( itd->u1.AddressOfData ), ntHd, binary.data ( ) );
			std::cout << " Fixing import for " << iibn->Name << std::endl;

			itd->u1.Function = clientImports.at ( i );


			itd++; i++;
		}
		impDesc++;
	}

	/*copy cleaned image without pe header*/

	cleanedBinary = binary;

	for ( size_t i = 0; i < 128; i++ ) {
		cleanedBinary.at ( i ) = 0x0;
	}
	

	return true;
}

bool ImageMap::FixRelocs ( ) {
	unsigned long ImageBase = ntHd->OptionalHeader.ImageBase;
	unsigned int nBytes = 0;
  
	auto size = ntHd->OptionalHeader.DataDirectory [ IMAGE_DIRECTORY_ENTRY_BASERELOC ].Size;

	unsigned long delta = MakeDelta ( unsigned long, (PVOID)allocatedBuffer, ImageBase );
	std::cout << "Delta SERVER  " << delta << std::endl;
	while ( 1 ) {
		unsigned long * locBase =
			( unsigned long * ) GetPtrFromRVA ( ( DWORD ) ( reloc->VirtualAddress ), ntHd, binary.data() );
		unsigned int numRelocs = ( reloc->SizeOfBlock - sizeof ( LIMAGE_BASE_RELOCATION ) ) / sizeof ( WORD );

		if ( nBytes >= size ) break;

		unsigned short * locData = MakePtr ( unsigned short *, reloc, sizeof ( LIMAGE_BASE_RELOCATION ) );
		for ( unsigned int i = 0; i < numRelocs; i++ ) {
			if ( ( ( *locData >> 12 ) & IMAGE_REL_BASED_HIGHLOW ) )
				*MakePtr ( unsigned long *, locBase, ( *locData & 0x0FFF ) ) += delta;

			locData++;
		}
		std::cout << "Fixed block " << locBase << std::endl;
		nBytes += reloc->SizeOfBlock;
		reloc = ( LIMAGE_BASE_RELOCATION * ) locData;
	}

	return true;
}

void MapHandle::ReadBinary ( ) {
	std::ifstream m_strFile( path, std::ios::binary );
	m_strFile.unsetf ( std::ios::skipws );
	m_strFile.seekg ( 0, std::ios::end );

	const auto m_iSize = m_strFile.tellg ( );

	m_strFile.seekg ( 0, std::ios::beg );
	binary.reserve ( static_cast< uint32_t >( m_iSize ) );
	binary.insert ( binary.begin ( ), std::istream_iterator< std::uint8_t > ( m_strFile ), std::istream_iterator< std::uint8_t > ( ) );
}

/// <summary>
/// Gets called only on MapHandle Initialization
/// </summary>
/// <returns>empty</returns>
void MapHandle::SetupImports ( ) {
	if ( binary.empty ( ) ) {

	//	throw_line ( "ERROR!!! binary is empty. MapHandle Line 149" );
		std::cout << "binary is empty " << std::endl;
		return;
	}

	char * mod;
	auto dosHeaders = reinterpret_cast< LIMAGE_DOS_HEADER * > ( binary.data ( ) );
	auto ntHd = reinterpret_cast< LIMAGE_NT_HEADERS * > ( binary.data ( ) + dosHeaders->e_lfanew );
	auto impDesc = reinterpret_cast< LIMAGE_IMPORT_DESCRIPTOR * >( GetPtrFromRVA
	(
		ntHd->OptionalHeader.DataDirectory [ /*IMAGE_DIRECTORY_ENTRY_IMPORT*/ 1 ].VirtualAddress,
		ntHd,
		binary.data ( )
	) );
	
	int count = 0;
	//   Loop through all the required modules
	while ( ( mod = ( char * ) GetPtrFromRVA ( ( DWORD ) ( impDesc->Name ), ntHd, binary.data ( ) )) ) {

		
	


		LIMAGE_THUNK_DATA * itd =
			( LIMAGE_THUNK_DATA * ) GetPtrFromRVA ( ( DWORD ) ( impDesc->FirstThunk ), ntHd, binary.data ( ) );

		while ( itd->u1.AddressOfData ) {
			LIMAGE_IMPORT_BY_NAME * iibn;
			iibn = ( LIMAGE_IMPORT_BY_NAME * ) GetPtrFromRVA ( ( DWORD ) ( itd->u1.AddressOfData ), ntHd, binary.data() );
			std::cout << "Read import " << iibn->Name << std::endl;

			
			m_aImportList.push_back ( std::make_pair ( std::string ( iibn->Name ), std::string ( mod ) ) );
			count++;
			itd++;
		}


		impDesc++;
	}

	std::cout << "Parsed " << m_aImportList.size() << " imports. " << std::endl;

}

