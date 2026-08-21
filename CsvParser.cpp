#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <exception>
#include "CsvParser.h" 

using namespace std;

namespace Data {
    // 把1. B f4 up   2. u c6 left   3. B f5 up   4. u b6 left   5. B f6 right 拆分回傳ordered_json
	vector<ordered_json> splitAnswerStep(const string& ansStep) {
		vector<ordered_json> answer;
		stringstream ss(ansStep);
		string stepNum;
		string piece, pos, dir;
		while (ss >> stepNum >> piece >> pos >> dir) {
			ordered_json oneStep;
			oneStep["piece"] = piece;
			oneStep["from"] = pos;
			oneStep["direction"] = dir;
			answer.push_back(oneStep);
		}
		return answer;
	}

	string removeQuote(const string &str) {
		string no_quote_str = str;
		if (str.length() >= 2 && str.front() == '"' && str.back() == '"') {
			no_quote_str = str.substr(1, str.length() - 2);
		}
		return no_quote_str;
	}

	// 負責處理謎題原本是csv檔改成有結構的json檔
	ordered_json convertPuzzleCsvToJson(const string& csvPath, const string& jsonPath) {
		ordered_json allPuzzles;
		ifstream file(csvPath);

		if (!file.is_open()) {
			cerr << "Error: CSV file path does not exist: " << csvPath << endl;
			return {}; // 優化：回傳空的 JSON 物件，而非 NULL
		}

		string puzzle;
		// 跳過第一行 "Board, Solution"
		getline(file, puzzle);

		while (getline(file, puzzle)) {
			if (puzzle.empty()) {
				continue;
			}

			string question, ansStep;
			stringstream ss(puzzle);

			// 去掉頭尾的引號，避免 CSV 格式破壞
			//處理題目
			getline(ss, question, ',');
			string no_quote_question = removeQuote(question);
			// 處理答案
			getline(ss, ansStep, ',');
			string no_quote_ansStep = removeQuote(ansStep);
			vector<ordered_json> answer(splitAnswerStep(no_quote_ansStep)); 

			ordered_json onePuzzle;
			onePuzzle["question"] = no_quote_question;
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



//int main() {
//	string csvPath = "result.csv";
//	string jsonPath = "result.json";
//	Data::convertPuzzleCsvToJson(csvPath, jsonPath);
//	return 0;
//}


