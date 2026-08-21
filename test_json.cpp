// 檔案名稱：test_json.cpp
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN 
#include "doctest.h"

#include "CsvParser.h"
#include <iostream>

using ordered_json = nlohmann::ordered_json;

TEST_CASE("test csv to json conversion with real data") {

    std::cout << "--- ready to read CSV test data ---\n";
    ordered_json allPuzzles = Data::convertPuzzleCsvToJson("C:\\Geister-Endgame-Puzzle\\result.csv", "result.json");

    // 如果 allPuzzles 是 nullptr，測試會立刻中斷並報錯
    REQUIRE_FALSE(allPuzzles.is_null());
    REQUIRE_FALSE(allPuzzles.empty());
    REQUIRE(allPuzzles.is_array());
    REQUIRE(allPuzzles.size() > 0);
    
	for (size_t i = 0; i < allPuzzles.size(); ++i) {
        INFO("Checking puzzle  at index " << i);
        
        const auto& puzzle = allPuzzles[i];
        REQUIRE(puzzle.contains("question"));
        REQUIRE(puzzle.contains("answer"));

        std::string questionStr = puzzle["question"].get<std::string>();
        REQUIRE_FALSE(questionStr.empty());

        CHECK(questionStr.front() != '"');
        CHECK(questionStr.back() != '"');

        REQUIRE(puzzle["answer"].is_array());
        for (size_t j = 0; j < puzzle["answer"].size(); j++) {
            INFO("Check answer at index" << j);

            std::string stepAnswer = puzzle["answer"][j].get<std::string>();
            REQUIRE_FALSE(stepAnswer.empty());

            CHECK(stepAnswer.front() != '"');
            CHECK(stepAnswer.back() != '"');
        }
	}
    
}