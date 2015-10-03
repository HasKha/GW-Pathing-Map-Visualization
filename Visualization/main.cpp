#include <stdio.h>
#include <vector>

#include <SDL2/SDL.h>
#include <SDL2/SDL_opengl.h>

#include "../PathingMap.h"
#include "Viewer.h"

SDL_Window* window;
SDL_RendererInfo renderer_info;
std::vector<PathingMapTrapezoid> trapezoids;

int main(int argc, char **argv) {

	// you can set the file hash here
	unsigned int file_hash = 290943;

	wchar_t filename[MAX_PATH];
	wsprintf(filename, L"PMAPs\\MAP %010u.pmap", file_hash);

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
	
	Viewer viewer = Viewer();
	viewer.InitializeWindow();
	viewer.SetPMap(pmap.GetPathingData());
	viewer.Execute();
	viewer.Close();
	
	return 0;
}
