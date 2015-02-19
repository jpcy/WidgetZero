/*
The MIT License (MIT)

Copyright (c) 2014 Jonathan Young

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/
#include <tigr.h>
#include <wz.h>

#define WINDOW_WIDTH 320
#define WINDOW_HEIGHT 200

class TigrRenderer : public wz::IRenderer
{
public:
	TigrRenderer(Tigr *screen) : screen(screen) {}

	virtual void drawButton(wz::Button *button, wz::Rect clip)
	{
		const wz::Rect rect = button->getAbsoluteRect();

		// Change color when hovering.
		const TPixel color = button->getHover() ? tigrRGB(128, 255, 128) : tigrRGB(255, 255, 255);

		// Draw background and border.
		tigrFill(screen, rect.x, rect.y, rect.w, rect.h, tigrRGB(64, 64, 64));
		tigrRect(screen, rect.x, rect.y, rect.w, rect.h, color);

		// Draw centered text.
		wz::Size textSize;
		textSize.w = tigrTextWidth(tfont, button->getLabel());
		textSize.h = tigrTextHeight(tfont, button->getLabel());
		tigrPrint(screen, tfont, rect.x + rect.w / 2 - textSize.w / 2, rect.y + rect.h / 2 - textSize.h / 2, color, button->getLabel());
	}
	
	virtual wz::Size measureButton(wz::Button *button)
	{
		// Measure text.
		wz::Size size;
		size.w = tigrTextWidth(tfont, button->getLabel());
		size.h = tigrTextHeight(tfont, button->getLabel());

		// Add padding.
		const wz::Border padding = button->getPadding();
		size.w += padding.left + padding.right;
		size.h += padding.top + padding.bottom;
		return size;
	}

private:
	Tigr *screen;
};

int main(int argc, char *argv[])
{
	// Create the tigr window.
	Tigr *screen = tigrWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "WidgetZero Example - Custom Renderer", 0);

	// Create the custom renderer.
	TigrRenderer *renderer = new TigrRenderer(screen);

	// Create the main window.
	wz::MainWindow *mainWindow = new wz::MainWindow(renderer);
	mainWindow->setSize(WINDOW_WIDTH, WINDOW_HEIGHT);

	// Create a button.
	wz::Button *button = new wz::Button("Click me!");
	button->setAlign(wz::Align::Center | wz::Align::Middle);
	mainWindow->add(button);

	// For tracking input state changes.
	int lastMouseX = 0, lastMouseY = 0, lastMouseButtons = 0;

	while (!tigrClosed(screen))
	{
		// Handle mouse movement.
		int mouseX, mouseY, mouseButtons;
		tigrMouse(screen, &mouseX, &mouseY, &mouseButtons);

		if (mouseX != lastMouseX || mouseY != lastMouseY)
		{
			mainWindow->mouseMove(mouseX, mouseY, mouseX - lastMouseX, mouseY - lastMouseY);
		}

		lastMouseX = mouseX;
		lastMouseY = mouseY;
		lastMouseButtons = mouseButtons;

		// Draw.
		tigrClear(screen, tigrRGB(192, 192, 192));
		mainWindow->draw();
		tigrUpdate(screen);
	}

	tigrFree(screen);
	return 0;
}
