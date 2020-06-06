#pragma once
#include "openvr.h"
#include "struct.h"
#include <d3d11.h>

class Eye {
	private:
		ID3D11ShaderResourceView* source;
		ID3D11Resource* pSource;
		vr::Hmd_Eye type;
	public:
		DirectContext* context;
		ID3D11Texture2D* texture;
		int height;
		int width;
		Eye(vr::Hmd_Eye type, DirectContext* context);
		bool init();
		void load();
		void toFile(const char* fileName);
		~Eye();
};
