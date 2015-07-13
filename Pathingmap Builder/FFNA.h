/**
* FFNA Format handler Class
* Credits to _rusty
**/
#pragma once
#include <Windows.h>
#include <vector>
#include <iostream>
#include <fstream>

enum DataTypes
{
	UnknownVectors,
	PlaneVectors,
	TransitionVectors,
	Trapazoid,
};

struct RawData
{
	int Type;
	int Length;
	unsigned char* pData;
};

struct FFNAChunk
{
	int Type;
	int Length;
	unsigned char* pData;
};

class FFNA
{
public:
	FFNA(TCHAR* File);
	~FFNA();

	std::vector<std::vector<RawData>> ReadTrapazoids();
	std::vector<RawData> ReadTransitionVectors(); //obsolete atm

	inline int GetType() const { return Type; }
private:
	template <typename T> T Get(unsigned char* pos )
	{
		T value;
		memcpy(&value,pos,sizeof(T));
		return value;
	}
private:
	int Type;
	std::vector<FFNAChunk> Chunks;
};
