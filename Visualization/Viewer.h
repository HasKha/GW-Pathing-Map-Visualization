#pragma once

#include <vector>

#include <SDL2\SDL.h>
#include <SDL2\SDL_opengl.h>

#include "../PathingMap.h"
#include "Point2d.h"


class Viewer {
private:
	static const int DEFAULT_WIDTH = 800;
	static const int DEFAULT_HEIGHT = 500;

	int width_;
	int height_;

	SDL_Window* window;
	std::vector<PathingMapTrapezoid> trapezoids_;
	int max_plane_;

	bool mouse_down_;

	bool refresh_;

	double scale_;
	Point2d translate_;
	double ratio_;

public:
	Viewer();

	void InitializeWindow();

	void Resize(int width, int height);

	void Execute();

	void SetPMap(std::vector<PathingMapTrapezoid> trapezoids);

	void RenderPMap();

	void Close();

	void handleMouseDownEvent(SDL_MouseButtonEvent);
	void handleMouseUpEvent(SDL_MouseButtonEvent);
	void handleMouseMoveEvent(SDL_MouseMotionEvent);
	void handleMouseWheelEvent(SDL_MouseWheelEvent);

	void handleWindowEvent(SDL_WindowEvent);
};
