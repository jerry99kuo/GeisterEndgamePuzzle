#pragma once
#include<string>
#include"json.hpp"

namespace Data {
	using ordered_json = nlohmann::ordered_json; // 沒有key排序的，這樣question answer 順序不會顛倒
	using json = nlohmann::json;
	ordered_json convertPuzzleCsvToJson(const std::string& csvPath, const std::string& jsonPath);


	struct Puzzle{
		std::string board;
		int eBnum;
		int maxDepth;
	};

	class PuzzlesLoader{
	public:
		PuzzlesLoader() = default;
		PuzzlesLoader(const std::string& csvPath);
		std::vector<Puzzle> puzzleSet;
	private:

		void readFile(const std::string& csvPath);
	};

}
