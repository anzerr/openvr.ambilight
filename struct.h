#pragma once
#include <d3d11.h>
#include "openvr.h"

#pragma comment(lib, "d3d11.lib")

#ifdef _WIN64
#pragma comment(lib, "lib/win64/openvr_api.lib")
#else
#pragma comment(lib, "lib/win32/openvr_api.lib")
#endif

#define log_info(log_level, message, ...) \
		printf("[openvr] [%s] %s\n", log_level, message, ##__VA_ARGS__)

#define debug(message, ...) \
		log_info("LOG_DEBUG", message, ##__VA_ARGS__)
#define warn(message, ...) \
		log_info("LOG_WARNING", message, ##__VA_ARGS__)

struct DirectContext {
	ID3D11Device* dev11;
	ID3D11DeviceContext* pContext;
};

struct Point {
	int x;
	int y;
};

struct Color {
	unsigned int r;
	unsigned int g;
	unsigned int b;
	unsigned int a;
};
