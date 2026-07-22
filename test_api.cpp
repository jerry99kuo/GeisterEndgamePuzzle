// 檔案名稱：test_api.cpp

// 1. 這兩行是 doctest 的靈魂，會自動幫你生成 main()
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN 
#include "doctest.h"

// 2. 引入你要測試的模組
#include "ApiUploader.h"
#include "CsvParser.h"
#include <iostream>

using ordered_json = nlohmann::ordered_json;

// 3. 定義一個測試案例
TEST_CASE("test ApiUploader can upload JSON to FastAPI") {

    std::cout << "--- ready to read CSV test data ---\n";
    ordered_json allPuzzles = Data::convertCsvToJson("C:\\Geister-Endgame-Puzzle\\result.csv", "result.json");

    // 如果 allPuzzles 是 nullptr，測試會立刻中斷並報錯
    REQUIRE(allPuzzles != nullptr);

    std::cout << "--- ready to send API request ---\n";
    ApiUploader uploader("127.0.0.1", 8000);
    bool isSuccess = uploader.sendJsonData("/upload", allPuzzles);

    // CHECK：驗證上傳結果是否為 true
    CHECK(isSuccess == true);
}