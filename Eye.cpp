
#include <d3d11.h>
#include "openvr.h"
#include "struct.h"
#include <fstream>
#include <stdio.h>
#include "Eye.h"

Eye::Eye(vr::Hmd_Eye type, DirectContext* context) {
	this->context = context;
	this->type = type;
	this->source = nullptr;
	this->pSource = nullptr;
	this->texture = nullptr;
	this->height = 0;
	this->width = 0;
}

bool Eye::init() {
	vr::VRCompositor()->GetMirrorTextureD3D11(this->type, this->context->dev11, (void**)&this->source);
	if (!this->source) {
		warn("initEye: GetMirrorTextureD3D11 failed");
		return false;
	}

	this->source->GetResource(&this->pSource);
	if (!this->pSource) {
		warn("initEye: GetResource failed");
		return false;
	}

	ID3D11Texture2D* tex2D;
	this->pSource->QueryInterface(&tex2D);
	if (!tex2D) {
		warn("initEye: QueryInterface failed");
		return false;
	}

	D3D11_TEXTURE2D_DESC desc;
	tex2D->GetDesc(&desc);
	desc.BindFlags = 0;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;// | D3D11_CPU_ACCESS_WRITE;
	desc.Usage = D3D11_USAGE_STAGING;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	tex2D->Release();

	this->height = desc.Height;
	this->width = desc.Width;
	HRESULT hr = this->context->dev11->CreateTexture2D(&desc, NULL, &this->texture);
	if (FAILED(hr)) {
		warn("initEye: CreateTexture2D failed");
		return false;
	}
	return true;
}

void Eye::load() {
	ID3D11Texture2D* tex2D;
	this->pSource->QueryInterface(&tex2D);

	D3D11_TEXTURE2D_DESC desc;
	tex2D->GetDesc(&desc);

	if (this->height != desc.Height || this->width != desc.Width) {
		warn("resolution has changed\n");
		return;
	}
	if (!this->context || !this->context->pContext) {
		warn("missing context\n");
	}
	this->context->pContext->CopyResource(this->texture, tex2D);
	tex2D->Release();
}

void Eye::toFile(const char* fileName) {
	unsigned char* m_captureData = nullptr;
	unsigned int size = this->width * this->height;
	m_captureData = new unsigned char[size * 4];

	ID3D11Device* d3dDevice;
	this->texture->GetDevice(&d3dDevice);
	ID3D11DeviceContext* d3dContext;
	d3dDevice->GetImmediateContext(&d3dContext);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = d3dContext->Map(
		this->texture,
		0,
		D3D11_MAP_READ,
		0,
		&mappedResource
	);
	if (FAILED(hr)) {
		warn("Failed to map staging texture");
		return;
	}

	const int pitch = this->width << 2;
	const unsigned char* source = static_cast<const unsigned char*>(mappedResource.pData);
	unsigned char* dest = m_captureData;
	for (int i = 0; i < this->height; ++i) {
		memcpy(dest, source, this->width * 4);
		source += mappedResource.RowPitch;
		dest += pitch;
	}
	d3dContext->Unmap(this->texture, 0);

	std::ofstream file(fileName, std::ios::binary);
	file.write((char*)&m_captureData[0], size * 4);
}

Eye::~Eye() {
	if (this->pSource) {
		this->pSource->Release();
	}
	if (this->texture) {
		this->texture->Release();
	}

	this->source = NULL;
	this->pSource = NULL;
	this->texture = NULL;
}