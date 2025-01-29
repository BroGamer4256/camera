#include "SigScan.h"

SIG_SCAN (sigSetCameraData, 0x1402FAE00, "\x48\x83\xEC\x28\x4C\x8B\xC1\x48\x83\xC1\x04\xE8\x50\x09\x00\x00\x49\x8D\x48\x10\xE8\x67\x09\x00\x00\xF3\x41\x0F\x10\x40\x1C\xF3\x0F\x59\x05\x95\x26\x9D\x00",
          "xxxxxxxxxxxx????xxxxx????xxxxxxxxxx????");
SIG_SCAN (sigGetButtonPressed, 0x1402AB1A0, "\x40\x55\x41\x56\x48\x83\xEC\x38", "xxxxxxxx");
SIG_SCAN (sigUpdateCamera, 0x1402FB0F0,
          "\x48\x81\xEC\x98\x00\x00\x00\x48\x8B\x05\x6A\x12\xAA\x00\x48\x33\xC4\x48\x89\x84\x24\x80\x00\x00\x00\xE8\x02\x03\x00\x00\xE8\x8D\x12\x00\x00\x0F\xB6\x05\xA0\x06\x93\x0C",
          "xxxxxxxxxx????xxxxxxxxxxxx????x????xxx????");

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

struct Vec4 {
	f32 x;
	f32 y;
	f32 z;
	f32 w;

	Vec4 () {
		this->x = 0;
		this->y = 0;
		this->z = 0;
		this->w = 0;
	}

	Vec4 (f32 x, f32 y, f32 z, f32 w) {
		this->x = x;
		this->y = y;
		this->z = z;
		this->w = w;
	}
};

struct CameraData {
	Vec3 viewPoint;
	Vec3 interest;
	f32 roll;
	f32 fov;
	f32 aetFov;
	f64 aspect;
	f32 minDistance;
	f32 maxDistance;
	f32 projLeftOffset;
	f32 projRightOffset;
	f32 projBottomOffset;
	f32 projTopOffset;
	u8 useUp;
	Vec3 up;
	u8 ignoreFov;
	u8 ignoreMinDist;
	Vec4 viewMatrix[4];
	Vec4 invViewMatrix[4];
	Vec4 projectionMatrix[4];
	Vec4 viewProjectionMatrix[4];
	Vec4 viewProjectionAet2dMatrix[4];
	Vec4 viewProjectionAet3dMatrix[4];
	f32 fovCorrectHeight;
	f32 aetDepth;
	Vec3 unk_1E4;
	Vec3 unk_1F0;
	Vec3 unk_1FC;
	Vec3 unk_208;
	f32 distance;
	Vec3 rotation;
	f32 fovHorizontalRad;
	u8 unk_228;
	u8 fastChange;
	u8 fastChangeHist0;
	u8 fastChangeHist1;
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

bool cameraOverwrite   = false;
bool f11Held           = false;
f32 verticalRotation   = 0.0f;
f32 horizontalRotation = 0.0f;

POINT mouseCurrent;
POINT mousePrevious;

extern "C" {
HOOK (void, SetCameraData, sigSetCameraData (), CameraData *, Vec3 *);
void
realSetCameraData (CameraData *cam, Vec3 *pos) {
	if (!cameraOverwrite) originalSetCameraData (cam, pos);
}
}

HOOK (bool, GetButtonPressed, sigGetButtonPressed (), void *inputState, int button, void *checkFunc) {
	if (!cameraOverwrite) return originalGetButtonPressed (inputState, button, checkFunc);
	return false;
}

HOOK (void *, UpdateCamera, sigUpdateCamera ()) {
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
	if (!cameraOverwrite) return originalUpdateCamera ();
	auto camera = (CameraData *)(readOffset ((u64)whereUpdateCamera + 0x26) - 0x22A);

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

	f32 speed = fast ? 0.5f : 0.1f;

	if (forward || backward) camera->viewPoint += PointFromAngle (verticalRotation + (forward ? +0.0f : -180.0f), speed);

	if (left || right) camera->viewPoint += PointFromAngle (verticalRotation + (right ? +90.0f : -90.0f), speed);

	if (up || down) camera->viewPoint.y += speed * (up ? +0.25f : -0.25f);

	if (clockwise || counterClockwise) camera->rotation.x += speed / 5.0 * (clockwise ? -1.0f : +1.0f);

	if (zoomIn || zoomOut) {
		camera->fov += speed * (zoomIn ? -1.0f : +1.0f);
		camera->fov = std::clamp (camera->fov, +1.0f, +200.0f);
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

	Vec2 interest      = PointFromAngle (verticalRotation, 1.0f);
	camera->interest.x = camera->viewPoint.x + interest.x;
	camera->interest.z = camera->viewPoint.z + interest.y;

	camera->interest.y = camera->viewPoint.y + PointFromAngle (horizontalRotation, 5.0f).x;

	return originalUpdateCamera ();
}

extern "C" {
__declspec (dllexport) void
init () {
	INSTALL_HOOK (SetCameraData);
	INSTALL_HOOK (GetButtonPressed);
	INSTALL_HOOK (UpdateCamera);
}
}
