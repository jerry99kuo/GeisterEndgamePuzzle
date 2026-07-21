#include<iostream>
#include"answerGenerator.h"
#include <chrono>

void answerGenerator::solve(const std::string& outputCsvPath, const std::string& mode) {
	int numPuzzles = puzzleSet.size();
	if (numPuzzles == 0) {
		// 謎題還沒載入
		std::cerr << "no puzzle! please reload" << std::endl;
		return;
	}

	std::ofstream outFile(outputCsvPath);
	if (!outFile.is_open()) {
		std::cerr << "Can't create output file: " << outputCsvPath << std::endl;
		return;
	}

	outFile << "Board,Solution\n";

	for (const auto& puzzle : puzzleSet) {
		Search search(mode, "n");
		search.setNeedAns(true);
		search.setNeedReadableAns(true);
		search.onlyWantSteps = true;
		int actionNum = search.think(puzzle.board, puzzle.eBnum, puzzle.maxDepth);

		std::string solutionSteps = "no solution";
		if (actionNum > 0) {
			solutionSteps = search.answerBoard;

			// 移除換行符號，讓整筆 solution 變成單行
			std::replace(solutionSteps.begin(), solutionSteps.end(), '\n', ' ');

			// 去掉頭尾多餘空白
			auto start = solutionSteps.find_first_not_of(' ');
			auto end = solutionSteps.find_last_not_of(' ');
			if (start != std::string::npos)
				solutionSteps = solutionSteps.substr(start, end - start + 1);
		}

		// 用引號包住，防止內容裡的逗號破壞 CSV 格式
		outFile << "\"" << puzzle.board << "\","
			<< "\"" << solutionSteps << "\"\n";

		std::cout << "Board: " << puzzle.board
			<< " | steps: " << actionNum << "\n";
	}

	outFile.close();
	std::cout << "All answers are saved in: " << outputCsvPath << std::endl;
	std::cout << "Total puzzles solved: " << numPuzzles << std::endl;
}

namespace {
	void rtrim(std::string& s) {
		s.erase(s.find_last_not_of(" \n\r\t") + 1);
	}
}



void answerGenerator::loadPuzzleSet(const std::string& filepath) {
	using namespace Data;
	std::ifstream file(filepath);

	if (!file.is_open()) {
		std::cerr << "Can't open file: " << filepath << "\n";
	}

	std::string line;
	Puzzle currentPuzzle;
	bool isWaitingForBoard =false;

	while (std::getline(file, line)) {
		rtrim(line); // 去除結尾的換行符號

		// 1. 尋找參數行並萃取 eBnum 與 maxDepth
		// 你的格式: myBnum : X, myRnum : X, enBnum : X, enRnum : X, maxDepth : X
		size_t ebPos = line.find("enBnum : ");
		size_t mdPos = line.find("maxDepth : ");

		if (ebPos != std::string::npos && mdPos != std::string::npos) {
			// std::stoi 會自動讀取數字，遇到逗號或非數字字元會自動停止，非常安全
			currentPuzzle.eBnum = std::stoi(line.substr(ebPos + 9));
			currentPuzzle.maxDepth = std::stoi(line.substr(mdPos + 11));

			isWaitingForBoard = true;
			continue;
		}

		// 2. 尋找 36 個字元的盤面行
		// 根據你的程式碼： for(int j=0; j<36; j++) ss << board[j]; ss << "\n\n";
		if (isWaitingForBoard && line.length() == 36) {
			currentPuzzle.board = line;
			puzzleSet.push_back(currentPuzzle);

			isWaitingForBoard = false; // 讀取完畢，重置狀態等待下一題
		}
	}

	file.close();
}

//int main() {
//	std::string filename = "normal/1221/5.txt";
//	auto start = std::chrono::high_resolution_clock::now();
//
//	answerGenerator temp;
//	temp.loadPuzzleSet(filename);
//	temp.solve("result", "n");
//	auto end = std::chrono::high_resolution_clock::now();
//	std::chrono::duration<double> diff = end - start;
//	std::cout << "Total time for answerGen:" << diff.count() << std::endl;
//
//	std::vector<Data::Puzzle>myPuzzles = temp.puzzleSet;
//	std::cout << "success load " << myPuzzles.size() << " puzzles!\n\n";
//
//	// 印出前 3 個檢查結果
//	for (size_t i = 0; i < myPuzzles.size() && i < 3; ++i) {
//		std::cout  <<"Puzzle " << (i + 1) << "\n";
//		std::cout << "eBnum:    " << myPuzzles[i].eBnum << "\n";
//		std::cout << "maxDepth: " << myPuzzles[i].maxDepth << "\n";
//		std::cout << "Board:    " << myPuzzles[i].board << "\n";
//		std::cout << "-----------------------\n";
//	}
//
//	return 0;
//}