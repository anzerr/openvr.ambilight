#include "Pixel.h"
#include "struct.h"
#include <array>
#include <vector>
#include <math.h>
#include "Eye.h"
#include <fstream>
#include <d3d11.h>

bool isLeft(Point a, Point b, Point c) {
	return ((b.x - a.x) * (c.y - a.y) - (b.y - a.y) * (c.x - a.x)) > 0;
}

int scale(int a, int b, int c) {
	if (a == 0 || c == 0) {
		return 0;
	}
	float x = (float)a / (float)b;
	return static_cast<int>(x * c);
}

const double pi = atan(1) * 4;

std::array<Point, 10> const mapPoints = { {
	{385, 67},
	{235, 170},
	{118, 318},
	{69, 469},
	{69, 1050},
	{97, 1164},
	{176, 1285},
	{265, 1367},
	{404, 1462},
	{576, 1515}
} };

int const wW = 1364;
int const hH = 1516;


Line::Line(Point a, Point b) {
	this->a = a;
	this->b = b;
};

bool Line::inside(Point c) {
	return isLeft(this->a, this->b, c);
}


Zone::Zone(Point start, int id) {
	this->start = start;
	this->id = id;
	this->size = 100;
	this->points = new std::vector<Point>();
};

void Zone::add(Point a) {
	this->points->push_back(a);
}

size_t Zone::length() {
	return this->points->size();
}


Pixel::Pixel(int width, int height) {
	this->width = width;
	this->height = height;
	this->lines = new std::vector<Line>();
	this->zones = new std::vector<Zone>();
	Point last = { 600, 0 };
	for (Point p : mapPoints) {
		Point a = {
			scale(last.x, wW, this->width),
			scale(last.y, hH, this->height)
		};
		Point b = {
			scale(p.x, wW, this->width),
			scale(p.y, hH, this->height)
		};
		this->lines->push_back(Line(a, b));
		last = p;
	}
}

bool Pixel::inside(Point c) {
	if (c.x < 0 || c.y < 0) {
		return false;
	}
	for (Line p : *this->lines) {
		if (p.inside(c)) {
			return false;
		}
	}
	return true;
}

void Pixel::createZone(int count) {
	double offset = pi;
	Point radius = {
		static_cast<int>(this->width * 0.45),
		static_cast<int>(this->height * 0.49)
	};
	Point center = {this->width / 2, this->height / 2};
	int steps = count + 1;
	for (int i = 1; i < steps; i++) {
		double o = offset + (pi * i / steps);
		Point start = {
			static_cast<int>(center.x + radius.x * sin(o)),
			static_cast<int>(center.y + radius.y * cos(o))
		};
		this->zones->push_back(Zone(start, i));
	}
	for (Zone z : *this->zones) {
		int s = z.size / 2;
		for (int x = 0; x < z.size; x++) {
			for (int y = 0; y < z.size; y++) {
				Point p = {(z.start.x - s) + x, (z.start.y - s) + y};
				if (this->inside(p)) {
					z.add(p);
				}
			}
		}
	}
}

Color Pixel::ReadColor(D3D11_MAPPED_SUBRESOURCE* mappedResource, Point pos) {
	int offset = (pos.y * mappedResource->RowPitch) + (pos.x * 4);
	const unsigned char* source = static_cast<const unsigned char*>(mappedResource->pData);
	Color color = {
		source[offset + 0],
		source[offset + 1],
		source[offset + 2],
		source[offset + 3]
	};
	return color;
	Color avg = { 255, 255, 255, 255 };
	return avg;
}

void Pixel::getData(Eye* eye, bool negative) {
	ID3D11Device* d3dDevice;
	eye->texture->GetDevice(&d3dDevice);
	ID3D11DeviceContext* d3dContext;
	d3dDevice->GetImmediateContext(&d3dContext);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = d3dContext->Map(
		eye->texture,
		0,
		D3D11_MAP_READ,
		0,
		&mappedResource
	);
	if (FAILED(hr)) {
		warn("Failed to map staging texture");
		return;
	}

	for (Zone z : *this->zones) {
		int total = 0;
		Color avg = { 0, 0, 0, 0 };
		for (Point p : *z.points) {
			Point pt = p;
			if (negative) {
				pt = { this->width - p.x, this->height - p.y };
			}
			Color t = this->ReadColor(&mappedResource, pt);
			avg.r += t.r;
			avg.g += t.g;
			avg.b += t.b;
			avg.a += t.a;
			total++;
		}
		if (total != 0) {
			avg.r = (avg.r == 0) ? 0 : avg.r / total;
			avg.g = (avg.g == 0) ? 0 : avg.g / total;
			avg.b = (avg.b == 0) ? 0 : avg.b / total;
			avg.a = (avg.a == 0) ? 0 : avg.a / total;
		}
		printf("avg: %d %d %d\n", avg.r, avg.g, avg.b);
	}
	d3dContext->Unmap(eye->texture, 0);
}


void Pixel::debugImage(const char* fileName, Eye* eye, bool negative) {
	ID3D11Device* d3dDevice;
	eye->texture->GetDevice(&d3dDevice);
	ID3D11DeviceContext* d3dContext;
	d3dDevice->GetImmediateContext(&d3dContext);

	D3D11_MAPPED_SUBRESOURCE mappedResource;
	HRESULT hr = d3dContext->Map(
		eye->texture,
		0,
		D3D11_MAP_READ,
		0,
		&mappedResource
	);
	if (FAILED(hr)) {
		warn("Failed to map staging texture");
		return;
	}

	unsigned char* m_captureData = nullptr;
	unsigned int size = this->width * this->height;
	m_captureData = new unsigned char[size * 4];

	const int pitch = this->width << 2;
	const unsigned char* raw_source = static_cast<const unsigned char*>(mappedResource.pData);

	unsigned char* source = nullptr;
	source = new unsigned char[mappedResource.DepthPitch];
	memcpy(source, raw_source, mappedResource.DepthPitch);

	for (Zone z : *this->zones) {
		int total = 0;
		Color avg = { 0, 0, 0, 0 };
		for (Point p : *z.points) {
			Point pt = p;
			if (negative) {
				pt = { this->width - p.x, this->height - p.y };
			}
			Color t = this->ReadColor(&mappedResource, pt);
			avg.r += t.r;
			avg.g += t.g;
			avg.b += t.b;
			avg.a += t.a;
			total++;
		}
		if (total != 0) {
			avg.r = (avg.r == 0) ? 0 : avg.r / total;
			avg.g = (avg.g == 0) ? 0 : avg.g / total;
			avg.b = (avg.b == 0) ? 0 : avg.b / total;
			avg.a = (avg.a == 0) ? 0 : avg.a / total;
		}
		for (Point p : *z.points) {
			Point pt = p;
			if (negative) {
				pt = { this->width - p.x, this->height - p.y };
			}
			int offset = (pt.y * mappedResource.RowPitch) + (pt.x * 4);
			source[offset + 0] = avg.r;
			source[offset + 1] = avg.g;
			source[offset + 2] = avg.b;
			source[offset + 3] = avg.a;
		}
	}

	unsigned char* dest = m_captureData;
	for (int i = 0; i < this->height; ++i) {
		memcpy(dest, source, this->width * 4);
		source += mappedResource.RowPitch;
		dest += pitch;
	}

	printf("pitch: %d, RowPitch: %d\n", pitch, mappedResource.RowPitch);
	d3dContext->Unmap(eye->texture, 0);

	std::ofstream file(fileName, std::ios::binary);
	file.write((char*)&m_captureData[0], size * 4);
}