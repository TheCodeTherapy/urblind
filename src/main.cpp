#include <algorithm>
#include <cmath>
#include <functional>
#include <iostream>
#include <vector>

#include "raylib.h"

// X11 headers with a #define namespace conflict avoidance hack (Xlib's Font conflicts with Raylib's Font)
#define Font XFont
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#undef Font

// TODO: Implement a method to print monitors layout on the terminal with ASCII art so users can select a monitor.
// TODO: Allow the user to select the monitor by index as a command line argument.
// TODO: Implement a shader-based paint brush to highlight parts of the texture.
// TODO: Implement a way to save the painted texture to a file with a bindkey (stb_image_write.h).

/**
 * ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │ Uses Raylib functions to gather information about the available monitors in a spatial manner.                    │
 * │ This will allow us to know how the monitors are positioned so we can index them left to right, facilitating so   │
 * │ the user can easily pick a monitor index to be the target of the application window.                             │
 * └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
class MonitorState {
 public:
  int totalWidth = 0;
  int totalHeight = 0;
  std::vector<int> spatialMonitorIndexes;
  std::vector<Vector2> resolutions;
  std::vector<Vector2> positions;
  int mainMonitor = 0;

  MonitorState() { retrieveMonitorData(); }

  void retrieveMonitorData() {
    int monitorCount = GetMonitorCount();
    if (monitorCount == 0) {
      std::cerr << "No monitors detected!" << std::endl;
      return;
    }

    std::vector<float> monitorPositions(monitorCount);
    resolutions.resize(monitorCount);

    for (int i = 0; i < monitorCount; i++) {
      Vector2 position = GetMonitorPosition(i);
      int width = GetMonitorWidth(i);
      int height = GetMonitorHeight(i);

      resolutions[i] = {static_cast<float>(width), static_cast<float>(height)};
      monitorPositions[i] = position.x;

      std::cout << "Monitor " << i << ": " << width << "x" << height << " | Position: (" << position.x << ","
                << position.y << ")\n";
    }

    // Sort monitor indexes by X position
    spatialMonitorIndexes.resize(monitorCount);
    for (int i = 0; i < monitorCount; i++) {
      spatialMonitorIndexes[i] = i;
    }
    std::sort(spatialMonitorIndexes.begin(), spatialMonitorIndexes.end(),
              [&monitorPositions](int a, int b) { return monitorPositions[a] < monitorPositions[b]; });

    for (int i = 0; i < monitorCount; i++) {
      positions.push_back(GetMonitorPosition(spatialMonitorIndexes[i]));
    }

    std::cout << "Monitor Order (Left to Right): ";
    for (int index : spatialMonitorIndexes) {
      std::cout << index << " ";
    }
    std::cout << std::endl;

    // Compute total width
    totalWidth = 0;
    for (int index : spatialMonitorIndexes) {
      totalWidth += resolutions[index].x;
    }

    // Compute total height (handling vertical stacking)
    totalHeight = 0;
    float maxY = 0;
    for (int index : spatialMonitorIndexes) {
      float y = GetMonitorPosition(index).y;
      if (y > maxY) {
        totalHeight += resolutions[index].y;
        maxY = y;
      } else {
        totalHeight = std::max(totalHeight, static_cast<int>(resolutions[index].y));
      }
    }

    // Get the main monitor
    mainMonitor = GetCurrentMonitor();
    std::cout << "Main Monitor: " << mainMonitor << std::endl;
  }
};

/**
 * ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │ This is the screenshot method. It uses Xlib.h to get X's display and root window and capture its content,        │
 * │ and Xutil only to call XDestroyImage and free the resources we used to grab the screenshot. XGetImage returns    │
 * │ image data using BGRX, so we must manually convert the image into RGBA to compose the image data to be used by   │
 * │ Raylib with the pixel format we need for our texture (uncompressed R8G8B8A8).                                    │
 * └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
Image CaptureScreenX11(int x, int y, int width, int height) {
  Display* display = XOpenDisplay(nullptr);
  if (!display) {
    std::cerr << "Cannot open X11 display!" << std::endl;
    return {0};
  }
  Window root = DefaultRootWindow(display);
  std::cout << "Root window: " << root << std::endl;

  XSync(display, True);
  XImage* img = XGetImage(display, root, x, y, width, height, AllPlanes, ZPixmap);

  if (!img) {
    std::cerr << "Failed to capture screen!" << std::endl;
    return {0};
  }

  // Allocate memory for the RGBA image
  unsigned char* rgbaData = new unsigned char[width * height * 4];

  // Convert BGRX to RGBA
  for (int i = 0; i < width * height; i++) {
    int pixelIndex = i * 4;
    rgbaData[pixelIndex + 0] = img->data[pixelIndex + 2];  // R
    rgbaData[pixelIndex + 1] = img->data[pixelIndex + 1];  // G
    rgbaData[pixelIndex + 2] = img->data[pixelIndex + 0];  // B
    rgbaData[pixelIndex + 3] = 255;                        // Alpha (fully opaque)
  }

  Image screenshot = {
      .data = rgbaData, .width = width, .height = height, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};

  XDestroyImage(img);  // Free X11 image memory
  return screenshot;
}

/**
 * ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │ This function helps us to control the "virtual camera" in regards to the pan, zoom, and the texture size limits. │
 * │ It prevents the camera from going beyond the texture limits when zoomed in (when the viewport is smaller than    │
 * │ the texture) and centers the texture when zoomed out (when the viewport is larger than the texture).             │
 * └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
void ClampPan(Vector2& pan, float zoom, Vector2 textureSize, Vector2 screenSize) {
  float visibleWidth = screenSize.x / zoom;
  float visibleHeight = screenSize.y / zoom;

  // center the zoomed out texture if smaller than the viewport
  if (visibleWidth >= textureSize.x) {
    pan.x = (textureSize.x - visibleWidth) / 2.0f;
  } else {
    pan.x = std::max(0.0f, std::min(pan.x, textureSize.x - visibleWidth));
  }

  if (visibleHeight >= textureSize.y) {
    pan.y = (textureSize.y - visibleHeight) / 2.0f;
  } else {
    pan.y = std::max(0.0f, std::min(pan.y, textureSize.y - visibleHeight));
  }
}

/**
 * ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │ This is a helper function that returns the mouse position coordinates in texture space. It considers our current │
 * │ pan and zoom to return exactly the X and Y coordinates of the texture pixel we have the mouse cursor on.         │
 * └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
Vector2 GetMousePositionOnTexture(Vector2 mouseViewport, Vector2 pan, float zoom) {
  return {(mouseViewport.x / zoom) + pan.x, (mouseViewport.y / zoom) + pan.y};
}

/**
 * ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │ This is a helper function that helps zooming with an aim. It takes our mouse position coordinates in texture     │
 * │ space, our previous zoom level (before triggering the mouse wheel event to zoom), our next zoom level (after     │
 * │ triggering the mouse wheel event), and our current pan coordinates. Then it computes the pan coordinates         │
 * │ so the texture pixel under our mouse cursor ends up being the same after zooming. Maybe there's a better way to  │
 * │ achieve the same, but that kinda works.                                                                          │
 * └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
Vector2 ComputeTargetPan(Vector2 mouseOnTexture, float previousZoom, float nextZoom, Vector2 currentPan) {
  Vector2 targetPan;
  targetPan.x = mouseOnTexture.x - ((mouseOnTexture.x - currentPan.x) * (previousZoom / nextZoom));
  targetPan.y = mouseOnTexture.y - ((mouseOnTexture.y - currentPan.y) * (previousZoom / nextZoom));
  return targetPan;
}

struct DebugInfo {
  std::string label;
  std::function<std::string()> valueFunc;
};

/**
 * ┌──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
 * │ This is just a small class I'll use as a debug panel to show some values in real-time because I didn't want to   │
 * │ bloat such a minimal and simple app with any overkill stuff like Dear ImGui or any other sophisticated GUI       │
 * │ library. I don't want the debug info to be ugly tho, so I'm writing this.                                        │
 * └──────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
 */
class DebugPanel {
 public:
  int x, y;
  int fontSize;
  float padding;
  bool visible = true;
  std::vector<DebugInfo> entries;
  int longestEntryEver = 0;

  DebugPanel(int startX, int startY, int textSize = 20, float spacingMultiplier = 1.25f)
      : x(startX), y(startY), fontSize(textSize), padding(textSize * spacingMultiplier) {}

  void AddEntry(const std::string& label, std::function<std::string()> valueFunc) {
    entries.push_back({label, valueFunc});
  }

  void Draw() {
    if (!visible) return;

    int entriesSize = entries.size();
    int longestEntryWidth = 0;
    for (const auto& entry : entries) {
      std::string text = entry.label + ": " + entry.valueFunc();
      int width = MeasureText(text.c_str(), fontSize);
      longestEntryWidth = std::max(longestEntryWidth, width);
      longestEntryEver = std::max(longestEntryEver, width);
    }

    int left = x - 10;
    int top = y - 10;
    int right = x + longestEntryEver + 10;
    int bottom = y + padding * entriesSize + 10;

    DrawRectangle(left, top, longestEntryEver + 20, padding * entries.size() + 20, Fade(BLACK, 0.8f));
    DrawLine(left, top, right, top, WHITE);        // top
    DrawLine(left, top, left, bottom, WHITE);      // left
    DrawLine(right, top, right, bottom, WHITE);    // right
    DrawLine(left, bottom, right, bottom, WHITE);  // bottom

    float yOffset = y;
    for (const auto& entry : entries) {
      std::string text = entry.label + ": " + entry.valueFunc();
      DrawText(text.c_str(), x, yOffset, fontSize, WHITE);
      yOffset += padding;
    }
  }
};

int main() {
  const int fontSize = 20;

  int screenWidth = 640;
  int screenHeight = 480;

  float zoom = 1.0f;
  float targetZoom = 1.0f;
  const float minZoom = 0.1f;
  const float maxZoom = 300.0f;
  const float smoothingFactor = 7.5f;

  bool dragging = false;
  Vector2 pan = {0, 0};
  Vector2 targetPan = {0, 0};

  Vector2 mousePosition = {0, 0};
  Vector2 previousMousePosition = {0, 0};
  Vector2 mouseOnTexture = {0, 0};

  float deltaTime = 0.0f;
  int fps = 0.0f;

  InitWindow(screenWidth, screenHeight, "raylib");

  DebugPanel debugPanel(12, 12, fontSize, 1.1f);
  debugPanel.AddEntry("fps    ", [&]() { return TextFormat("%d", fps); });
  debugPanel.AddEntry("mouse  ", [&]() { return TextFormat("%04.0f, %04.0f", mousePosition.x, mousePosition.y); });
  debugPanel.AddEntry("texure ", [&]() { return TextFormat("%04.0f, %04.0f", mouseOnTexture.x, mouseOnTexture.y); });
  debugPanel.AddEntry("pan    ", [&]() { return TextFormat("%04.0f, %04.0f", pan.x, pan.y); });
  debugPanel.AddEntry("zoom   ", [&]() { return TextFormat("%.2f", zoom); });

  MonitorState monitorState;
  screenWidth = static_cast<int>(monitorState.resolutions[2].x);
  screenHeight = static_cast<int>(monitorState.resolutions[2].y);

  SetWindowSize(screenWidth, screenHeight);
  SetWindowPosition(static_cast<int>(monitorState.positions[2].x), static_cast<int>(monitorState.positions[2].y));

  pan.x = monitorState.positions[2].x;
  pan.y = monitorState.positions[2].y;
  targetPan.x = monitorState.positions[2].x;
  targetPan.y = monitorState.positions[2].y;

  SetConfigFlags(FLAG_WINDOW_HIDDEN);
  // WARNING: This is a hack that forces the window to be hidden before the screenshot. Not sure if it's consistent
  // because maybe the X server and/or the compositor need more time. On the CaptureScreenX11 function, I'm using
  // XSync to flush the output buffer and wait for X to process all requests in queue. Maybe that's enough?
  Image screenshot = CaptureScreenX11(0, 0, monitorState.totalWidth, monitorState.totalHeight);
  ClearWindowState(FLAG_WINDOW_HIDDEN);

  if (screenshot.data == nullptr) {
    std::cerr << "Failed to capture screen!" << std::endl;
    return -1;
  }

  Texture2D texture = LoadTextureFromImage(screenshot);
  SetTextureWrap(texture, TEXTURE_WRAP_MIRROR_REPEAT);
  SetTextureFilter(texture, TEXTURE_FILTER_POINT);

  bool shouldClose = false;

  /**
   * ┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
   * │ Main loop                                                                                                      │
   * └────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
   */
  while (!shouldClose) {
    deltaTime = GetFrameTime();
    fps = GetFPS();

    if (IsKeyPressed(KEY_ESCAPE)) shouldClose = true;
    if (IsKeyPressed(KEY_F11)) ToggleFullscreen();
    if (IsKeyPressed(KEY_TAB)) debugPanel.visible = !debugPanel.visible;

    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
      dragging = true;
      previousMousePosition = GetMousePosition();
    }

    if (IsMouseButtonReleased(MOUSE_LEFT_BUTTON)) dragging = false;

    mousePosition = GetMousePosition();
    if (dragging) {
      targetPan.x -= (mousePosition.x - previousMousePosition.x) / zoom;
      targetPan.y -= (mousePosition.y - previousMousePosition.y) / zoom;
      previousMousePosition = mousePosition;
    }

    float wheel = GetMouseWheelMove();
    float previousZoom = zoom;
    mouseOnTexture = GetMousePositionOnTexture(mousePosition, pan, zoom);
    if (wheel != 0) {
      float zoomFactor = 1.05f;
      if (wheel > 0) {
        targetZoom *= zoomFactor;
      } else {
        targetZoom /= zoomFactor;
      }

      targetZoom = std::max(minZoom, std::min(targetZoom, maxZoom));
      targetPan = ComputeTargetPan(mouseOnTexture, previousZoom, targetZoom, pan);
    }

    const float smoothing = 1.0f - exp(-smoothingFactor * deltaTime);
    zoom += (targetZoom - zoom) * smoothing;
    pan.x += (targetPan.x - pan.x) * smoothing;
    pan.y += (targetPan.y - pan.y) * smoothing;

    ClampPan(pan, zoom, {static_cast<float>(texture.width), static_cast<float>(texture.height)},
             {static_cast<float>(screenWidth), static_cast<float>(screenHeight)});

    Rectangle source = {pan.x, pan.y, monitorState.resolutions[2].x / zoom, monitorState.resolutions[2].y / zoom};

    Rectangle dest = {0, 0, monitorState.resolutions[monitorState.mainMonitor].x,
                      monitorState.resolutions[monitorState.mainMonitor].y};

    // TODO: MAYBE adjust the dest rectangle to clamp the texture when it is zoomed out and smaller than the viewport?
    // I kinda like the mirrored repeat texture wrapping though. It feels unpolished but it looks cool.

    BeginDrawing();
    ClearBackground(BLACK);
    DrawTexturePro(texture, source, dest, {0, 0}, 0, WHITE);

    debugPanel.Draw();

    EndDrawing();
  }
  /**
   * ┌────────────────────────────────────────────────────────────────────────────────────────────────────────────────┐
   * └────────────────────────────────────────────────────────────────────────────────────────────────────────────────┘
   */

  UnloadTexture(texture);
  UnloadImage(screenshot);
  CloseWindow();
  return 0;
}
