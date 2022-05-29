#pragma once
#include <map>
#include "PortableExecutable.h"
#include "BinaryPacket.h"


class MapHandle {
public:
	MapHandle ( ) {
		ReadBinary ( );
		SetupImports ( );
	};
	~MapHandle ( ) { };

public:

	std::string path = "C:\\Users\\topor\\source\\repos\\TestDll\\Release\\TestDll.dll";

	void ReadBinary ( );


	std::vector<unsigned char> binary;


	void SetupImports ( );
	std::vector< std::pair<std::string, std::string> > m_aImportList = { };

};

class ImageMap  {
public:
	ImageMap ( MapHandle * hdl );
	~ImageMap ( ) {

	
	}

	LIMAGE_NT_HEADERS * ntHd;
	LIMAGE_IMPORT_DESCRIPTOR * impDesc;
	LIMAGE_BASE_RELOCATION * reloc;

	uint64_t  allocatedBuffer;

	std::vector<unsigned char> binary;

	std::vector<unsigned char> cleanedBinary;

	std::vector<std::uint32_t> clientImports = { };

	bool FixImports ( );

	bool FixRelocs ( );

	std::uint64_t GetAllocationSize ( ) {
		return  ntHd->OptionalHeader.SizeOfImage;
	}
	std::uint64_t GetEntryPoint ( ) {
		return  ntHd->OptionalHeader.AddressOfEntryPoint;
	}
	MapHandle * parentHandle;

};

