#include "ApiUploader.h"
#include <iostream>
#include"CsvParser.h"
// 初始化設定
ApiUploader::ApiUploader(const std::string& host, int port)
    : client(host, port) {
    // 批次上傳 CSV 轉換的 JSON 資料可能會比較大，可以稍微把 timeout 設長一點
    client.set_connection_timeout(5, 0);
}

// 執行發送
bool ApiUploader::sendJsonData(const std::string& endpoint, const nlohmann::ordered_json& payload) {
    if (payload.is_null()) {
        std::cerr << "ERROR : Payload is empty or NULL." << std::endl;
        return false;
    }

    std::string json_payload = payload.dump();
    std::cout << "Sending batch data to FastAPI " << endpoint << "..." << std::endl;

    if (auto res = client.Post(endpoint.c_str(), json_payload, "application/json")) {
        if (res->status == 200) {
            std::cout << "Success!" << std::endl;
            std::cout << "FastAPI Response: " << res->body << std::endl;
            return true;
        }
        else {
            std::cerr << "Failed! HTTP Status: " << res->status << std::endl;
            std::cerr << "error info: " << res->body << std::endl;
            return false;
        }
    }
    else {
        auto err = res.error();
        std::cerr << "Connection error: " << httplib::to_string(err) << std::endl;
        std::cerr << "Please check if FastAPI is running." << std::endl;
        return false;
    }
}


//using namespace std;
//using ordered_json = nlohmann::ordered_json;
//
//int main() {
//    // 1. 讀取並轉換資料
//    ordered_json allPuzzles = Data::convertCsvToJson("result.csv", "result.json");
//    if (allPuzzles == nullptr) {
//        cout << "ERROR : No Data in result.csv" << endl;
//        return 0;
//    }
//
//    // 2. 實例化上傳器
//    ApiUploader uploader("127.0.0.1", 8000);
//
//    // 3. 呼叫發送方法
//    uploader.sendJsonData("/upload", allPuzzles);
//    return 0;
//}