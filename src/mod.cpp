#include "SigScan.h"

SIG_SCAN (sigSetCameraPosition, 0x1404CCD10, "\xF2\x0F\x10\x02\xF2\x0F\x11\x01\x8B\x42\x08\x89\x41\x08\xC3", "xxxxxxxxxxxxxxx");
SIG_SCAN (sigSetCameraFocus, 0x1404CCD30, "\xF2\x0F\x10\x02\xF2\x0F\x11\x41\x0C", "xxxxxxxxx");
SIG_SCAN (sigSetCameraRotation, 0x1404CCD20, "\xF2\x0F\x10\x02\xF2\x0F\x11\x41\x18", "xxxxxxxxx");
SIG_SCAN (sigSetCameraHorizontalFov, 0x1404CCD70, "\xF3\x0F\x11\x49\x24\xC3", "xxxxxx");
SIG_SCAN (sigSetCameraVerticalFov, 0x1404CCD50, "\xF3\x0F\x11\x49\x28\xC3", "xxxxxx");
SIG_SCAN (sigGetButtonPressed, 0x1402AB1A0, "\x40\x55\x41\x56\x48\x83\xEC\x38", "xxxxxxxx");
SIG_SCAN (sigUpdateCamera, 0x1402FA9F0, "\x48\x8B\xC4\x55\x48\x8D\x68\xE8", "xxxxxxxx");
SIG_SCAN (sigGetCamera, 0x1404D7C48, "\xE8\xCC\xCC\xCC\xCC\x48\x8D\x1D\xCC\xCC\xCC\xCC\x48\x8B\xCB\xBA\x03\x00\x00\x00", "x????xxx????xxxxxxxx");

#pragma pack(8)
struct Vec2 {
	f32 x;
	f32 y;

	Vec2 () {
		this->x = 0;
		this->y = 0;
	}

	Vec2 (f32 x, f32 y) {
		this->x = x;
		this->y = y;
	}
};

struct Vec3 {
	f32 x;
	f32 y;
	f32 z;

	Vec3 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
	}

	Vec3 (f32 x, f32 y, f32 z) {
		this->x = x;
		this->y = y;
		this->z = z;
	}

	void operator+= (const Vec2 &value) {
		x += value.x;
		z += value.y;
	}
};

struct Camera {
	Vec3 position;
	Vec3 focus;
	f32 rotation;
	f32 unk_1C;
	f32 unk_20;
	f32 horizontalFov;
	f32 verticalFov;
	f32 depth;
};

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

u64
readOffset (u64 loc) {
	u64 next   = loc + 4;
	u8 *p      = (u8 *)loc;
	i32 offset = p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
	return next + offset;
}

f32
ToRadians (f32 degrees) {
	return (degrees * M_PI) / 180.0f;
}

Vec2
PointFromAngle (f32 degrees, f32 distance) {
	f32 radians = ToRadians (degrees + 90.0f);
	return Vec2 (-1 * std::cos (radians) * distance, -1 * std::sin (radians) * distance);
}

FUNCTION_PTR (Camera *, GetCamera, readOffset ((u64)sigGetCamera () + 1));
bool cameraOverwrite   = false;
bool f11Held           = false;
f32 verticalRotation   = 0.0f;
f32 horizontalRotation = 0.0f;

POINT mouseCurrent;
POINT mousePrevious;

extern "C" {
HOOK (void, SetCameraPosition, sigSetCameraPosition (), Camera *cam, Vec3 *pos);
void
realSetCameraPosition (Camera *cam, Vec3 *pos) {
	if (!cameraOverwrite) originalSetCameraPosition (cam, pos);
}

HOOK (void, SetCameraFocus, sigSetCameraFocus (), Camera *cam, Vec3 *focus);
void
realSetCameraFocus (Camera *cam, Vec3 *focus) {
	if (!cameraOverwrite) originalSetCameraFocus (cam, focus);
}

HOOK (void, SetCameraRotation, sigSetCameraRotation (), Camera *cam, Vec3 *a2);
void
realSetCameraRotation (Camera *cam, Vec3 *a2) {
	if (!cameraOverwrite) originalSetCameraRotation (cam, a2);
}

HOOK (void, SetCameraHorizontalFov, sigSetCameraHorizontalFov (), Camera *cam, f32 fov);
void
realSetCameraHorizontalFov (Camera *cam, f32 fov) {
	if (!cameraOverwrite) originalSetCameraHorizontalFov (cam, fov);
}

HOOK (void, SetCameraVerticalFov, sigSetCameraVerticalFov (), Camera *cam, f32 fov);
void
realSetCameraVerticalFov (Camera *cam, f32 fov) {
	if (!cameraOverwrite) originalSetCameraVerticalFov (cam, fov);
}
}

HOOK (bool, GetButtonPressed, sigGetButtonPressed (), void *inputState, int button, void *checkFunc) {
	if (!cameraOverwrite) return originalGetButtonPressed (inputState, button, checkFunc);
	return false;
}

HOOK (void *, UpdateCamera, sigUpdateCamera (), void *a1, f32 verticalFov, f32 a3) {
	mousePrevious = mouseCurrent;
	GetCursorPos (&mouseCurrent);
	if (GetAsyncKeyState (VK_F11) & 0x8000 && !f11Held) {
		cameraOverwrite = !cameraOverwrite;
		f11Held         = true;
		RECT windowRect;
		GetWindowRect (GetActiveWindow (), &windowRect);

		int centerX        = windowRect.left + (windowRect.right - windowRect.left) / 2;
		int centerY        = windowRect.top + (windowRect.bottom - windowRect.top) / 2;
		mouseCurrent.x     = centerX;
		mouseCurrent.y     = centerY;
		mousePrevious      = mouseCurrent;
		verticalRotation   = 0.0f;
		horizontalRotation = 0.0f;
	}
	if (!(GetAsyncKeyState (VK_F11) & 0x8000) && f11Held) f11Held = false;
	if (!cameraOverwrite) return originalUpdateCamera (a1, verticalFov, a3);
	auto camera = GetCamera ();

	bool forward  = GetAsyncKeyState ('W') & 0x8000;
	bool backward = GetAsyncKeyState ('S') & 0x8000;
	bool left     = GetAsyncKeyState ('A') & 0x8000;
	bool right    = GetAsyncKeyState ('D') & 0x8000;

	bool up   = GetAsyncKeyState (VK_SPACE) & 0x8000;
	bool down = GetAsyncKeyState (VK_CONTROL) & 0x8000;

	bool clockwise        = GetAsyncKeyState ('Q') & 0x8000;
	bool counterClockwise = GetAsyncKeyState ('E') & 0x8000;

	bool zoomIn  = GetAsyncKeyState ('R') & 0x8000;
	bool zoomOut = GetAsyncKeyState ('F') & 0x8000;

	bool fast = GetAsyncKeyState (VK_SHIFT) & 0x8000;

	f32 speed = fast ? 0.05f : 0.01f;

	if (forward || backward) camera->position += PointFromAngle (verticalRotation + (forward ? +0.0f : -180.0f), speed);

	if (left || right) camera->position += PointFromAngle (verticalRotation + (right ? +90.0f : -90.0f), speed);

	if (up || down) camera->position.y += speed * (up ? +0.25f : -0.25f);

	if (clockwise || counterClockwise) camera->rotation += speed * (clockwise ? -1.0f : +1.0f);

	if (zoomIn || zoomOut) {
		camera->horizontalFov += speed * (zoomIn ? -1.0f : +1.0f);
		camera->horizontalFov = std::clamp (camera->horizontalFov, +1.0f, +200.0f);
	}

	RECT windowRect;
	GetWindowRect (GetActiveWindow (), &windowRect);

	int centerX = windowRect.left + (windowRect.right - windowRect.left) / 2;
	int centerY = windowRect.top + (windowRect.bottom - windowRect.top) / 2;

	mousePrevious.x = centerX;
	mousePrevious.y = centerY;
	SetCursorPos (centerX, centerY);

	auto deltaX = mouseCurrent.x - mousePrevious.x;
	auto deltaY = mouseCurrent.y - mousePrevious.y;

	verticalRotation += deltaX * 0.1f;
	horizontalRotation -= deltaY * (0.1f / 5.0f);

	horizontalRotation = std::clamp (horizontalRotation, -75.0f, +75.0f);

	Vec2 focus      = PointFromAngle (verticalRotation, 1.0f);
	camera->focus.x = camera->position.x + focus.x;
	camera->focus.z = camera->position.z + focus.y;

	camera->focus.y = camera->position.y + PointFromAngle (horizontalRotation, 5.0f).x;

	return originalUpdateCamera (a1, verticalFov, a3);
}

extern "C" {
__declspec (dllexport) void init () {
	INSTALL_HOOK (SetCameraPosition);
	INSTALL_HOOK (SetCameraFocus);
	INSTALL_HOOK (SetCameraRotation);
	INSTALL_HOOK (SetCameraHorizontalFov);
	INSTALL_HOOK (SetCameraVerticalFov);
	INSTALL_HOOK (GetButtonPressed);
	INSTALL_HOOK (UpdateCamera);
}
}
