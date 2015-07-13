#include "FFNA.h"

using namespace std;

FFNA::FFNA( TCHAR* File )
{
	ifstream ffna;
	ffna.open(File,ios::binary);

	int length;
	ffna.seekg (0, ios::end);
	length = ffna.tellg();
	ffna.seekg (0, ios::beg);

	if(length < 4)
	{
		ffna.close();
		throw "FILE_CORRUPT";
	}

	ffna.seekg(4,ios::beg);

	char type;
	ffna.read(&type,1);
	Type = (int)type;
	
	while (ffna.tellg() < length - 4)
	{
		FFNAChunk newChunk;
		unsigned char* buffer = new unsigned char[8];
		ffna.read((char*)buffer,8);
		newChunk.Type = Get<int>(buffer);
		newChunk.Length = Get<int>(buffer + 4);

		delete[] buffer;

		if(length - ffna.tellg() < newChunk.Length)
		{
			ffna.close();
			throw "FILE_CORRUPT";
		}

		buffer = new unsigned char[newChunk.Length];
		ffna.read((char*)buffer,newChunk.Length);
		newChunk.pData = buffer;
		Chunks.push_back(newChunk);
	}
	ffna.close();
}


vector< vector<RawData> > FFNA::ReadTrapazoids()
{
	if(Type != 3)
		throw "UNKNOWN_FORMAT";

	FFNAChunk chunk;
	ZeroMemory(&chunk,sizeof(FFNAChunk));
	for(unsigned int i=0;i<Chunks.size();i++)
	{
		if(Chunks[i].Type == 536870920)
		{
			chunk = Chunks[i];
			break;
		}
	}

	if(chunk.Type != 536870920)
		throw "FILE_CORRUPT";

	vector< vector<RawData> > Trapazoids;
	RawData tmpData;
	int vectorType = Trapazoid;

	if(chunk.Length < 17)
		throw "FILE_CORRUPT";

	int curPos = 13;
	curPos += Get<int>(chunk.pData + curPos) + 5 + 4;

	if(chunk.Length < curPos + 4 || curPos < 0)
		throw "FILE_CORRUPT";

	int sectionCount = Get<int>(chunk.pData + curPos);curPos += 4;
	
	if(chunk.Length < curPos)
		throw "FILE_CORRUPT";

	int section=0;
	int plane = 0;
	while(curPos < chunk.Length)
	{	
		unsigned char sectionHeader = chunk.pData[curPos];
		curPos++;

		int sectionLength = 0;

		if(!(sectionHeader == 0x0B))
		{
			sectionLength = Get<int>(chunk.pData + curPos);
			if(chunk.Length < curPos + 4)
				throw "FILE_CORRUPT";
		}
		else
		{
			sectionLength = Get<int>(chunk.pData + curPos) / 2;
			if(chunk.Length < curPos + 4)
				throw "FILE_CORRUPT";
		}
		curPos +=4;

		if(sectionHeader == 0)
			section++;
		else if(sectionHeader == 2)
		{
			int tmpPos = curPos;
			while(tmpPos < (curPos + sectionLength))
			{
				if(chunk.Length < tmpPos + 44)
					throw "FILE_CORRUPT";
				//tmpPos += 20;


 				unsigned int Adjacent1 = Get<unsigned int>(chunk.pData + tmpPos); tmpPos +=4;
				unsigned int Adjacent2 = Get<unsigned int>(chunk.pData + tmpPos); tmpPos +=4;
				unsigned int Adjacent3 = Get<unsigned int>(chunk.pData + tmpPos); tmpPos +=4;
				unsigned int Adjacent4 = Get<unsigned int>(chunk.pData + tmpPos); tmpPos +=4;

				short Transistion1 = Get<short>(chunk.pData + tmpPos); tmpPos +=2;
				short Transistion2 = Get<short>(chunk.pData + tmpPos); tmpPos +=2;

				DWORD tmpY1 = Get<DWORD>(chunk.pData + tmpPos); tmpPos +=4;
				DWORD tmpY2 = Get<DWORD>(chunk.pData + tmpPos); tmpPos +=4;
				DWORD tmpX1 = Get<DWORD>(chunk.pData + tmpPos); tmpPos +=4;
				DWORD tmpX2 = Get<DWORD>(chunk.pData + tmpPos); tmpPos +=4;
				DWORD tmpX3 = Get<DWORD>(chunk.pData + tmpPos); tmpPos +=4;
				DWORD tmpX4 = Get<DWORD>(chunk.pData + tmpPos); tmpPos +=4;

				if(!(tmpY1 == tmpY2))
				{
					tmpData.Type = vectorType;
					tmpData.Length = 44;
					tmpData.pData = (chunk.pData + tmpPos - 44); //leave as RawData for now
					if(plane >= Trapazoids.size())
						Trapazoids.resize(plane + 1);
					Trapazoids[plane].push_back(tmpData);
				}
			}
			plane++;
		}
		curPos += sectionLength;
	}
	return Trapazoids;
}

FFNA::~FFNA()
{
	for(unsigned int i=0;i<Chunks.size();i++)
		delete[] Chunks[i].pData;
	Chunks.clear();
}

std::vector<RawData> FFNA::ReadTransitionVectors()
{
//20000004
	if(Type != 3)
		throw "UNKNOWN_FORMAT";

	vector<RawData> result;

	FFNAChunk chunk;
	ZeroMemory(&chunk,sizeof(FFNAChunk));
	for(unsigned int i=0;i<Chunks.size();i++)
	{
		if(Chunks[i].Type == 536870916)
		{
			chunk = Chunks[i];
			break;
		}
	}

	if(chunk.Type != 536870916)
		throw "FILE_CORRUPT";
	
	if(chunk.Length < 10)
		throw "FILE_CORRUPT";
	int curPos = 6;
	//{//skip unneded data
		long Sec1Len = Get<long>(chunk.pData + curPos);
		curPos = 11 + Sec1Len;
		if(chunk.Length < curPos + 4)
			throw "FILE_CORRUPT";

		long Sec2Len = Get<long>(chunk.pData + curPos);
		curPos = 20 + Sec1Len + Sec2Len;
		if(chunk.Length < curPos + 4)
			throw "FILE_CORRUPT";

		long Sec3Len = 2 * Get<long>(chunk.pData + curPos);
		curPos = 25 + Sec1Len + Sec2Len + Sec3Len;
		if(chunk.Length < curPos + 4)
			throw "FILE_CORRUPT";
	//}
	long Sec4Len = Get<long>(chunk.pData + curPos); curPos +=4;
	if(chunk.Length < curPos + 4)
		throw "FILE_CORRUPT";
// 	long VectorCount = Get<long>(chunk.pData + curPos); curPos +=4;
// 	if(chunk.Length < curPos + 4)
// 		throw "FILE_CORRUPT";
	
	curPos = 33 + Sec1Len + Sec2Len + Sec3Len; 
	while(curPos<(29 + Sec1Len + Sec2Len + Sec3Len + Sec4Len))
	{
		if(chunk.Length < curPos + 16)
			throw "FILE_CORRUPT";
		RawData tmpData;
		tmpData.Type = TransitionVectors;
		tmpData.Length = 16;
		tmpData.pData = chunk.pData + curPos;

		curPos += 16;
	
		result.push_back(tmpData);	
	}

	return result;
}

