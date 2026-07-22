#pragma once

// ==========================================
// 1. C++ 標準函式庫 (Standard Libraries)
// ==========================================
#include <iostream>
#include <iomanip>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include <random>
#include <chrono>
#include <cassert> 
#include <cstdlib>
#include <cstdio>
#include <ctime>   

// ==========================================
// 2. 跨平台系統 API (OS-Specific Headers)
// ==========================================
#ifdef _WIN32
    // --- Windows 環境 (包含 Visual Studio 與 MinGW) ---
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN // 防止 Windows 標頭檔引入舊版 Winsock 造成衝突
#endif

#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#include <direct.h>
#include <intrin.h>

// 處理 MinGW 下的安全字串函數相容性
#if defined(__GNUC__)
#define sprintf_s(buf, ...) snprintf((buf), sizeof(buf), __VA_ARGS__)
#endif

#else
    // --- POSIX 環境 (Linux / macOS) ---
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#endif

// ==========================================
// 3. 全域設定與巨集 (Global Config & Macros)
// ==========================================
#define OUTPUT_JP_MESSAGE 0

// ==========================================
// 4. 跨平台目錄操作函式 (Directory Utils)
// ==========================================
/*
 * [跨平台目錄操作]
 * 使用 _WIN32 判斷作業系統，避免 MinGW (__GNUC__) 誤用 POSIX 的雙參數 mkdir。
 */
inline int makeDir(const char* dirName) {
#ifdef _WIN32
    return _mkdir(dirName);
#else
    return mkdir(dirName, S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

inline int changeDir(const char* dirName) {
#ifdef _WIN32
    return _chdir(dirName);
#else
    return chdir(dirName);
#endif
}
