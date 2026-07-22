#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <exception>
#include "CsvParser.h" 

using namespace std;

namespace Data {
	ordered_json convertCsvToJson(const string& csvPath, const string& jsonPath) {
		ordered_json allPuzzles;
		ifstream file(csvPath);

		if (!file.is_open()) {
			cerr << "Error: CSV file path does not exist: " << csvPath << endl;
			return {}; // 優化：回傳空的 JSON 物件，而非 NULL
		}

		string puzzle;
		while (getline(file, puzzle)) {
			if (puzzle.empty()) {
				continue;
			}

			string question, ansStep;
			stringstream ss(puzzle);
			getline(ss, question, ',');

			vector<string> answer;
			// 拆解答案
			while (getline(ss, ansStep, ',')) {
				if (!ansStep.empty()) {
					answer.push_back(ansStep);
				}
			}

			ordered_json onePuzzle;
			onePuzzle["question"] = question;
			onePuzzle["answer"] = answer;
			allPuzzles.push_back(onePuzzle);
		}
		// ifstream 會在離開 scope 時自動關閉，不需手動 close()

		ofstream outFile(jsonPath);
		if (outFile.is_open()) {
			outFile << allPuzzles.dump(4);
			outFile.close();
			cout << "Success! Json file path : " << jsonPath << endl;
		}
		else {
			cerr << "Error: Can't create json file at " << jsonPath << endl;
		}

		return allPuzzles;
	}


	// Constructor
	PuzzlesLoader::PuzzlesLoader(const string& csvPath) {
		readFile(csvPath);
	}

	void PuzzlesLoader::readFile(const string& csvPath) {
		ifstream inFile(csvPath);
		if (!inFile.is_open()) {
			cerr << "無法開啟輸入檔案: " << csvPath << endl;
			return;
		}

		string line;
		while (getline(inFile, line)) {
			if (line.empty()) continue;

			stringstream ss(line);
			string boardStr, eBnumStr, maxDepthStr;

			// CSV 格式：盤面,eBnum,maxDepth
			getline(ss, boardStr, ',');
			getline(ss, eBnumStr, ',');
			getline(ss, maxDepthStr, ',');

			// 優化：加上 Try-Catch 防止stoi遇到壞資料時程式崩潰
			try {
				int eBnum = stoi(eBnumStr);
				int maxDepth = stoi(maxDepthStr);

				// 優化：如果 puzzleSet 裝的是 struct，用 emplace_back 效能更好
				this->puzzleSet.push_back(Puzzle{ boardStr, eBnum, maxDepth });
			}
			catch (const exception& e) {
				cerr << "警告: 資料解析失敗，略過此行 [" << line << "] 錯誤原因: " << e.what() << endl;
			}
		}
		// inFile 離開 scope 會自動關閉
	}

} // namespace Data


/* test
int main() {
	string csvPath = "test.csv";
	string jsonPath = "result.json";
	Data::convertCsvToJson(csvPath, jsonPath);
	return 0;
}
*/

