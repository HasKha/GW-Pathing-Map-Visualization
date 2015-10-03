#pragma once

// header of the gw.dat
struct MainHeader
{
	unsigned char ID[4];
	int HeaderSize;
	int SectorSize;
	int CRC1;
	__int64 MFTOffset;
	int MFTSize;
	int Flags;
};

// Master File Table Header
struct MFTHeader
{
	unsigned char ID[4];
	int Unk1;
	int Unk2;
	int EntryCount;
	int Unk4;
	int Unk5;
};

// Master File Table Entry
struct MFTEntry
{
	__int64 Offset;
	__int32 Size;
	unsigned short a;
	unsigned char b;
	unsigned char c;
	__int32 ID;
	__int32 CRC;
	__int32 Hash;
};

// Master File Table expansion
struct MFTExpansion
{
	int FileNumber;
	int FileOffset;
};

void UnpackGWDat(unsigned char *input, int insize, unsigned char *&output, int &outsize);