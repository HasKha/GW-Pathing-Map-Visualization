#include "stdafx.h"
#include <Windows.h>
#include <vector>
#include <algorithm>
#include <iostream>
#include <fstream>
#include "Xentax.h"
#include "FFNA.h"
#include "PathingMap.h"

using namespace std;

HANDLE GWDat;
MainHeader GWHead;
MFTHeader MFTH;

vector<MFTExpansion> MFTX;
vector<MFTEntry> MFT;

int PathingMapCount = 0;

void _nfseek(HANDLE f, __int64 offset, int origin)
{
	LARGE_INTEGER i;
	i.QuadPart=offset;
	SetFilePointerEx(f,i,NULL,FILE_BEGIN);
}

void _nfread(void *buffer, int size, int count, HANDLE f)
{
	DWORD bread;
	int errcde=ReadFile(f,buffer,size*count,&bread,NULL);
	if (!errcde)
	{
		TCHAR text[2048];
		FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM,0,GetLastError(),0,text,1024,NULL);
		printf("%s\n",text);
	}
}

void PrintUsage()
{
	printf("Usage:n"
		"pmap.exe (option)\n"
		"Options: \n"
		"		-e [GW.dat] - Extracts all Pathingmaps from GW.dat\n"
		/*"		-f [FFNA file] - Extract Pathingmap from specified FFNA file\n"*/);
}

void ProcessFile(char* file,DWORD hash)
{
	try
	{
		FFNA ffna(file);
	}
	catch (...)
	{
		return;
	}
	FFNA ffna(file);
	
	if(!(ffna.GetType() == 3))
		return;

 	vector< vector<RawData> > Trapazoids;
	vector< RawData > Transitions;
	try
	{
		Trapazoids = ffna.ReadTrapazoids();
		Transitions = ffna.ReadTransitionVectors();
	}
	catch (...)
	{
		//printf("Error: %s",error);
		return;
 	}

	if(Trapazoids.size() > 0)
	{
		PathingMap pmap(hash);
		for(unsigned int i=0;i<Trapazoids.size();i++)
		{
			for(unsigned int j=0;j<Trapazoids[i].size();j++)
			{
				GWPathingTrapezoid* trapez = reinterpret_cast<GWPathingTrapezoid*>(Trapazoids[i][j].pData);
				pmap.AddPathingTrapazoid(i,*trapez);
			}
		}

		for(unsigned int i=0;i<Transitions.size();i++)
		{
			Transition* trans = reinterpret_cast<Transition*>(Transitions[i].pData);
			pmap.AddTransition(i,*trans);
		}

		CreateDirectory(TEXT("PMAPs"),NULL);
		char newname[1024];
		sprintf_s(newname,1024,".\\PMAPs\\MAP %010u.pmap",hash);
		pmap.Save(newname);
		PathingMapCount++;
	}
	for(unsigned int i=0;i<Trapazoids.size();i++)
		Trapazoids[i].clear();
	Trapazoids.clear();	
}


void ExtractFile(MFTEntry* m)
{
	if(!m->b)
	{
		return;
	}
	
	_nfseek(GWDat,m->Offset,0);
	unsigned char *Input=new unsigned char[m->Size];
	_nfread(Input,m->Size,1,GWDat);
	unsigned char *Output=NULL;
	int OutSize=0;

	if (m->a) UnpackGWDat(Input,m->Size,Output,OutSize);        
	else
	{
		Output=new unsigned char[m->Size];
		memcpy(Output,Input,m->Size);
		OutSize=m->Size;
	}
	if (Output)
	{
		unsigned int i=((unsigned int*)Output)[0];
		unsigned int k=((unsigned int*)Output)[1];
		int i2=i&0xffff;
		int i3=i&0xffffff;

		if(i == 'anff')
		{
			char Name[256];
			sprintf_s(Name,256,"[MAP]%08X",m->CRC);
			char newname[1024];
			sprintf_s(newname,1024,".\\temp\\%s.ffna",Name);
			CreateDirectory("temp",NULL);
			ofstream of;
			of.open(newname,ios::binary);
			of.write((char*)Output,OutSize);
			of.close();
			ProcessFile(newname,(DWORD)m->Hash);
			DeleteFile(newname);
		}
		else
		{
			//printf("\nnot an ffna file - skipping\n");
		}
	delete[] Output;
 	}
	delete[] Input;
}

bool compareH(MFTExpansion& a, MFTExpansion b)
{
	return a.FileOffset < b.FileOffset;
}

void ExtractGWDat(char* gwdat)
{
	FILE* g=fopen(gwdat,"rb");
	if(!g)
	{
		printf("Error opening %s\n",gwdat);
		return;
	}
	fclose(g);
	GWDat=CreateFile(gwdat,GENERIC_READ,FILE_SHARE_READ,NULL,OPEN_EXISTING,FILE_ATTRIBUTE_NORMAL,NULL);
	_nfread(&GWHead,sizeof(GWHead),1,GWDat);
	printf("Header data:\n");
	printf("ID          = %c%c%c%c\n",GWHead.ID[0],GWHead.ID[1],GWHead.ID[2],GWHead.ID[3]);
	if (!(GWHead.ID[0]==0x33 && GWHead.ID[1]==0x41 && GWHead.ID[2]==0x4e && GWHead.ID[3]==0x1a))
	{
		printf("The input file is not a Guild Wars datafile!\n");
		return;
	}
	printf("Header Size = %10d\n",GWHead.HeaderSize);
	printf("Sector Size = %10d\n",GWHead.SectorSize);
	printf("CRC1        = %10x\n",GWHead.CRC1);
	printf("MFT Offset  = %10I64d\n",GWHead.MFTOffset);
	printf("MFT Size    = %10d\n",GWHead.MFTSize);

	printf("\nReading MFT\n");
	_nfseek(GWDat,GWHead.MFTOffset,SEEK_SET);
	_nfread(&MFTH,sizeof(MFTH),1,GWDat);
	printf("EntryCount  = %10d\n",MFTH.EntryCount);

	//read reserved MFT entries
	for (int x=0; x<15; x++)
	{
		MFTEntry ME;
		_nfread(&ME,0x18,1,GWDat);
		ME.Hash = 0;
		MFT.push_back(ME);
	}

	//read Hashlist
	_nfseek(GWDat,MFT[1].Offset,SEEK_SET);
	int mftxsize=MFT[1].Size/sizeof(MFTExpansion);
	for (unsigned int x=0; x<MFT[1].Size/sizeof(MFTExpansion); x++)
	{
		MFTExpansion _MFTX;
		_nfread(&_MFTX,sizeof(MFTExpansion),1,GWDat);
		MFTX.push_back(_MFTX);
	}
	
	std::sort(MFTX.begin(), MFTX.end(), compareH);

	//read MFT entries
	unsigned int hashcounter = 0;
	while (MFTX[hashcounter].FileOffset < 16)
		++hashcounter;

	_nfseek(GWDat,GWHead.MFTOffset + 24 * 16,SEEK_SET);
	for (int x = 16; x < MFTH.EntryCount - 1; ++x)
	{
		MFTEntry ME;
		_nfread(&ME,0x18,1,GWDat);

		if (hashcounter < MFTX.size() && x == MFTX[hashcounter].FileOffset)
		{
			ME.Hash = MFTX[hashcounter].FileNumber;
			MFT.push_back(ME);

			while (hashcounter + 1 < MFTX.size() && MFTX[hashcounter].FileOffset == MFTX[hashcounter + 1].FileOffset)
			{
				++hashcounter;
				ME.Hash = MFTX[hashcounter].FileNumber;
				MFT.push_back(ME);
			}

			++hashcounter;
		}
		else
		{
			ME.Hash = 0;
			MFT.push_back(ME);
		}		
	}



	printf("\n\n");
	for(unsigned int i=0;i<MFT.size();i++)
	{
		printf("\rProcessing file %6d",i);
		ExtractFile(&(MFT[i]));
		//printf("\n");
	}
	RemoveDirectory(TEXT("temp"));
}

int _tmain(int argc, char* argv[])
{
	SetConsoleTitle(TEXT("GW Pathingmap Builder by ACB"));
	
	if(argc<3)
	{
		PrintUsage();
		return -1;
	}
	
	char option = argv[1][1];
	switch(option)
	{
	case 'e':
		ExtractGWDat(argv[2]);
		break;
// 	case 'f':
// 		ProcessFile(argv[2]);
// 		break;
	default:
		PrintUsage();
		return -1;
	}
	
	printf("\rCreated %6d Pathingmaps",PathingMapCount);
	Sleep(10000);
	return 0;
}

