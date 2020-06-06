#pragma once
#include "struct.h"
#include <vector>
#include "Eye.h"

class Line {
private:
public:
	Point a;
	Point b;
	Line(Point a, Point b);
	bool inside(Point c);
};

class Zone {
public:
	Point start;
	int size;
	int id;
	Zone(Point start, int id);
	std::vector<Point>* points;
	void add(Point a);
	size_t length();
};

class Pixel {
private:
	int width;
	int height;
	std::vector<Line>* lines;
	std::vector<Zone>* zones;
	bool inside(Point c);
public:
	Pixel(int width, int height);
	void createZone(int count);
	void getData(Eye* eye, bool negative);
	void debugImage(const char* fileName, Eye* eye, bool negative = false);
	Color ReadColor(D3D11_MAPPED_SUBRESOURCE* mappedResource, Point c);
};
