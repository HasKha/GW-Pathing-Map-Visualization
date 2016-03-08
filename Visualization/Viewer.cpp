#include "Viewer.h"

Viewer::Viewer() {
	window = nullptr;
	trapezoids_ = std::vector<PathingMapTrapezoid>();
	max_plane_ = 1;
	mouse_down_ = false;
	refresh_ = false;
	scale_ = 0.0001;
	translate_ = Point2d();
	wireframe_ = false;
	circles_ = false;

	int n_vertices = 50;
	for (int i = 0; i < 50; ++i) {
		float angle = i * (2 * static_cast<float>(M_PI) / n_vertices);
		float x = std::cos(angle);
		float y = std::sin(angle);
		circle_vertices_.push_back(Point2d(x, y));
	}
	circle_vertices_.push_back(circle_vertices_[0]);

	circle_sizes_.push_back(300); // nearby aka hos
	circle_sizes_.push_back(322);
	circle_sizes_.push_back(366);
	circle_sizes_.push_back(1085); // spellcast
	circle_sizes_.push_back(2500); // spirit range
	circle_sizes_.push_back(5000); // compass
}

void Viewer::InitializeWindow() {
	SDL_Init(SDL_INIT_VIDEO);
	window = SDL_CreateWindow("Pathing visualization",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		DEFAULT_WIDTH, DEFAULT_HEIGHT, 
		SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

	SDL_GL_CreateContext(window);
	SDL_GL_SetSwapInterval(1);

	glDisable(GL_LIGHTING);
	glDisable(GL_DEPTH_TEST);

	Resize(DEFAULT_WIDTH, DEFAULT_HEIGHT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
}

void Viewer::Resize(int width, int height) {
	width_ = width;
	height_ = height;
	printf("resizing to %d %d\n", width, height);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	ratio_ = (double)width / height;
	glScaled(1, ratio_, 1);

	refresh_ = true;
}

void Viewer::Execute() {
	bool quit = false;
	while (!quit) {
		SDL_Delay(1);
		// event handling
		SDL_Event e;
		while (SDL_PollEvent(&e) != 0) {
			switch (e.type) {
			case SDL_QUIT:
				quit = true;
				break;
			case SDL_MOUSEBUTTONDOWN:
				HandleMouseDownEvent(e.button);
				break;
			case SDL_MOUSEBUTTONUP:
				HandleMouseUpEvent(e.button);
				break;
			case SDL_MOUSEMOTION:
				HandleMouseMoveEvent(e.motion);
				break;
			case SDL_MOUSEWHEEL:
				HandleMouseWheelEvent(e.wheel);
				break;
			case SDL_WINDOWEVENT:
				HandleWindowEvent(e.window);
				break;
			case SDL_KEYDOWN:
				HandleKeyDownEvent(e.key);
				break;
			case SDL_KEYUP:
				HandleKeyUpEvent(e.key);
				break;
			default:
				break;
			}
		}

		if (refresh_) {
			RenderPMap();
			refresh_ = false;
		}
	}
}

void Viewer::SetPMap(std::vector<PathingMapTrapezoid> trapezoids) {
	trapezoids_ = trapezoids;
	max_plane_ = 1;
	for (size_t i = 0; i < trapezoids_.size(); ++i) {
		if (max_plane_ < trapezoids_[i].Plane) {
			max_plane_ = trapezoids_[i].Plane;
		}
	}
	refresh_ = true;
}

void Viewer::RenderPMap() {
	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	glViewport(0, 0, width_, height_);
	glScaled(scale_, scale_, 1);
	glPushMatrix();
	glTranslated(translate_.x(), translate_.y(), 0);

	if (wireframe_) {
		glPolygonMode(GL_FRONT_AND_BACK, GL_LINE); // wireframe
	} else {
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // full quads
	}

	glColor3f(1, 0, 0);
	glBegin(GL_QUADS);
	for (size_t i = 0; i < trapezoids_.size(); ++i) {
		float c = (float)trapezoids_[i].Plane / max_plane_;
		glColor3f(c, 1 - c, 0);
		glVertex2f(trapezoids_[i].XTL, trapezoids_[i].YT);
		glVertex2f(trapezoids_[i].XTR, trapezoids_[i].YT);
		glVertex2f(trapezoids_[i].XBR, trapezoids_[i].YB);
		glVertex2f(trapezoids_[i].XBL, trapezoids_[i].YB);
	}
	glEnd();

	if (circles_) {
		glPopMatrix();
		glTranslated(center_.x(), center_.y(), 0.0);
		glColor3f(1.0f, 1.0f, 1.0f);
		glPolygonMode(GL_FRONT_AND_BACK, GL_FILL); // full quads

		glBegin(GL_POINT);
		glVertex2f(0.0f, 0.0f);
		glEnd();

		for (size_t j = 0; j < circle_sizes_.size(); ++j) {
			glPushMatrix();
			glScaled(circle_sizes_[j], circle_sizes_[j], 0.0);

			glBegin(GL_LINE_STRIP);
			for (size_t i = 0; i < circle_vertices_.size(); ++i) {
				glVertex2d(circle_vertices_[i].x(), circle_vertices_[i].y());
			}
			glEnd();
			glPopMatrix();
		}

		
		//glBegin(GL_TRIANGLES);
		//glVertex2f(center_.x(), center_.y());
		//glVertex2f(center_.x() + 500, center_.y() - 500);
		//glVertex2f(center_.x() - 500, center_.y() - 500);
		//glEnd();
	}


	SDL_GL_SwapWindow(window);
}

void Viewer::Close() {
	SDL_DestroyWindow(window);
	SDL_Quit();
}

void Viewer::HandleMouseDownEvent(SDL_MouseButtonEvent button) {
	if (button.button == SDL_BUTTON_LEFT) {
		mouse_down_ = true;
	}
}

void Viewer::HandleMouseUpEvent(SDL_MouseButtonEvent button) {
	if (button.button == SDL_BUTTON_LEFT) {
		mouse_down_ = false;
	}
}

void Viewer::HandleMouseMoveEvent(SDL_MouseMotionEvent motion) {
	if (mouse_down_) {
		Point2d diff = Point2d(motion.xrel, -motion.yrel);
		diff.x() /= width_; // remap from [0, WIDTH] to [0, 1]
		diff.y() /= height_; // remap from [0, HEIGHT] to [0, 1]
		diff.y() /= ratio_; // adjust for window aspect ratio
		diff *= 2; // remap from [0, 1]^2 to [0, 2]^2 (screen space is [-1, 1] so range has to be 2
		diff /= scale_; // remap for scale

		translate_ += diff;
		
		refresh_ = true;
	}

	{
		center_ = Point2d(motion.x, motion.y);
		center_.x() /= width_; // remap from [0, Width] to [0, 1]
		center_.y() /= -height_; // remap from [0, Height] to [0, 1]
		center_.y() += 0.5f;
		center_.x() -= 0.5f;
		center_.y() /= ratio_; // adjust for window aspect ratio
		center_ *= 2; // remap from [0, 1]^2 to [0, 2]^2
		center_ /= scale_;

		refresh_ = true;
	}
}

void Viewer::HandleMouseWheelEvent(SDL_MouseWheelEvent wheel) {
	if (wheel.y > 0) {
		scale_ *= 1.25;
	} else {
		scale_ *= 0.8;
	}
	refresh_ = true;
}

void Viewer::HandleWindowEvent(SDL_WindowEvent window) {
	switch (window.event) {
	case SDL_WINDOWEVENT_RESIZED:
		Resize(window.data1, window.data2);
		break;
	default:
		break;
	}
}

void Viewer::HandleKeyDownEvent(SDL_KeyboardEvent keyboard) {
	switch (keyboard.keysym.sym) {
	default:
		break;
	}
}

void Viewer::HandleKeyUpEvent(SDL_KeyboardEvent keyboard) {
	switch (keyboard.keysym.sym) {
	case SDLK_SPACE:
		wireframe_ = !wireframe_;
		refresh_ = true;
		break;
	case SDLK_c:
		circles_ = !circles_;
		refresh_ = true;
		break;
	default:
		break;
	}
}
