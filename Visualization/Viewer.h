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
	bool wireframe_;

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

	void HandleMouseDownEvent(SDL_MouseButtonEvent);
	void HandleMouseUpEvent(SDL_MouseButtonEvent);
	void HandleMouseMoveEvent(SDL_MouseMotionEvent);
	void HandleMouseWheelEvent(SDL_MouseWheelEvent);
	void HandleKeyDownEvent(SDL_KeyboardEvent);
	void HandleKeyUpEvent(SDL_KeyboardEvent);

	void HandleWindowEvent(SDL_WindowEvent);
};
