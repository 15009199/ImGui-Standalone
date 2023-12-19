#pragma once

#include <iostream>
#include <fstream>
#include <Windows.h>

namespace MyLogger {
    void LogHRESULT(HRESULT hr, const char* message, const char* file, int line);
}
