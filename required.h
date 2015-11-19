

#ifndef REQUIRED_H
#define REQUIRED_H

#define _CRT_SECURE_NO_WARNINGS

#include <Windows.h>
#include <vector>
#include <map>
#include <string>
#include <fstream>
#include <time.h>
#include <iostream>
#include <functional>
#include <algorithm>
#include <ctime>

void Log(const char* szText, ...);
void GetDirFile(const char* file, char* out, size_t len);

#endif
