#include "View.h"
#include "openvr.h"
#include <d3d11.h>
#include "struct.h"
#include "Eye.h"

View::View() {
	this->context = {};
	this->lastCheckTick = 0;
	this->isInit = false;
	this->left = new Eye(vr::Eye_Left, &this->context);
	this->right = new Eye(vr::Eye_Right, &this->context);
	this->init(false);
}

bool View::init(bool forced) {
	if (this->isInit) {
		return true;
	}
	if (GetTickCount() - 1000 < this->lastCheckTick && !forced) {
		return false;
	}

	vr::EVRInitError err = vr::VRInitError_None;
	vr::VR_Init(&err, vr::VRApplication_Background);
	if (err != vr::VRInitError_None) {
		debug("OpenVR not available");
		this->lastCheckTick = GetTickCount();
		return false;
	}

	HRESULT hr;
	D3D_FEATURE_LEVEL featureLevel;
	hr = D3D11CreateDevice(NULL, D3D_DRIVER_TYPE_HARDWARE, 0, 0, 0, 0, D3D11_SDK_VERSION, &this->context.dev11, &featureLevel, &this->context.pContext);
	if (FAILED(hr)) {
		warn("openVrInit: D3D11CreateDevice failed");
		return false;
	}

	if (!this->left->init()) {
		warn("openVrInit: failed to init left eye");
		return false;
	}
	if (!this->right->init()) {
		warn("openVrInit: failed to init right eye");
		return false;
	}

	this->isInit = true;
	return true;
}

View::~View() {
	delete this->left;
	delete this->right;

	if (this->isInit) {
		vr::VR_Shutdown();
	}

	if (this->context.pContext) {
		this->context.pContext->Release();
		this->context.pContext = NULL;
	}
	if (this->context.dev11) {
		if (this->context.dev11->Release() != 0) {
			warn("openVrdeinit: device refcount not zero!");
		}
		this->context.dev11 = NULL;
	}

	this->isInit = false;
}