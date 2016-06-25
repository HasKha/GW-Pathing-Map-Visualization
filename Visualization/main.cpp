#include <stdio.h>
#include <vector>

#include "../PathingMap.h"
#include "Viewer.h"


int main(int argc, char **argv) {

	unsigned int file_hash;
	if (argc < 2) {
		file_hash = 219215;
	} else {
		file_hash = atoi(argv[1]);
	}


	wchar_t filename[MAX_PATH];
	wsprintf(filename, L"..\\PMAPs\\MAP %010u.pmap", file_hash);

	PathingMap pmap(file_hash);
	bool loaded = pmap.Open(filename);
	if (loaded) {
		printf("loaded pmap!\n");
	} else {
		printf("failed to load pmap!\n");
		return 1;
	}
	printf("loaded %d\n", loaded);
	printf("number of trapzeoids %d\n", pmap.GetPathingData().size());
	printf("index: [plane] YT YB XTL XTR XBL XBR\n");
	//for (int i = 0; i < pmap.GetPathingData().size(); ++i) {
	//	printf("%d: ", i);
	//	printf("[%d] ", pmap.GetPathingData()[i].Plane);
	//	printf("%.2f ", pmap.GetPathingData()[i].YT);
	//	printf("%.2f ", pmap.GetPathingData()[i].YB);
	//	printf("%.2f ", pmap.GetPathingData()[i].XTL);
	//	printf("%.2f ", pmap.GetPathingData()[i].XTR);
	//	printf("%.2f ", pmap.GetPathingData()[i].XBL);
	//	printf("%.2f\n", pmap.GetPathingData()[i].XBR);
	//}
	
	Viewer viewer = Viewer();
	viewer.InitializeWindow();
	viewer.SetPMap(pmap.GetPathingData());
	viewer.Execute();
	viewer.Close();
	
	return 0;
}
