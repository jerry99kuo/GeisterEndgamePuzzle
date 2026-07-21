#pragma once
#ifdef _WIN32
#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0A00 
#endif
#endif

#include <string>
#include "json.hpp"
#include "httplib.h"

class ApiUploader {
private:
    httplib::Client client;

public:
    // 初始化連線設定 (主機、Port)
    ApiUploader(const std::string& host, int port);

    // 專門負責發送 ordered_json 資料
    bool sendJsonData(const std::string& endpoint, const nlohmann::ordered_json& payload);
};