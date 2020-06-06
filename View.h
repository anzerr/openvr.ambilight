#pragma once
#include "openvr.h"
#include "Eye.h"
#include "struct.h"

class View {
	private:
		DirectContext context;
		DWORD lastCheckTick;
		bool init(bool forced);
	public:
		View();
		Eye* left;
		Eye* right;
		bool isInit;
		~View();
};
