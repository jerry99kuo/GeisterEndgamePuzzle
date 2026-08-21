#define WIN32_LEAN_AND_MEAN

#include "RandomGeneration.h"
#include "ExampleSolution.h"
#include "CsvParser.h"
#include <direct.h>
#include "ApiUploader.h"
#pragma comment(lib, "ws2_32.lib")
#include"answerGenerator.h"
using namespace std;

std::mt19937_64 IntRandom::mt64;

// 處理資料 (取消註解以啟用)
//#define COMMUNICATE_WITH_WEBSITE

namespace {
    // 讀取命令列的輸入，來執行解題功能
    void RunAnswerGen(const std::vector<std::string>& argvVec) {
        // 至少需要 4 個參數：[程式名] [輸入檔至少1個] [輸出檔] [mode]
        if (argvVec.size() < 4) {
            std::cerr << "Usage: " << argvVec[0] << " AnswerGen [input1.txt] [input2.txt] ... [output CSV path] [mode]" << std::endl;
            return;
        }

        // 從後面抓取固定的參數
        const std::string mode = argvVec.back(); // 最後一個
        const std::string outputCsvFilePath = argvVec[argvVec.size() - 2]; // 倒數第二個

        answerGenerator generator;

        // 迴圈讀取所有的輸入檔 (從索引 1 開始，直到倒數第三個)
        for (size_t i = 1; i < argvVec.size() - 2; ++i) {
            const std::string inputTxtFilePath = argvVec[i];
            std::cout << "Loading: " << inputTxtFilePath << std::endl;

            generator.loadPuzzleSet(inputTxtFilePath);
        }

        // 所有檔案都 load 完之後，一次進行 solve
        generator.solve(outputCsvFilePath, mode);
        string outputJsonFilePath = outputCsvFilePath.substr(0, outputCsvFilePath.size() - 4) + ".json";
        Data::convertPuzzleCsvToJson(outputCsvFilePath , outputJsonFilePath);
    }

	// 打印幫助訊息
    void PrintHelpMessage(const char* programName) {
#if OUTPUT_JP_MESSAGE
        cout << "引数が正しくない" << endl;
        cout << "2 つのモード RandomGen GetSol それぞれの詳細を下記に参照してください．" << endl << endl;
#else
        cout << "Wrong program arguments" << endl;
        cout << "Please refer to the following for the details of the two modes, RandomGen and GetSol." << endl << endl;
#endif

#if OUTPUT_JP_MESSAGE
        cout << "ランダム生成法を回したい場合：" << endl;
#else
        cout << "To run random generation:" << endl;
#endif
        HelpMessageRandGen(programName);
        cout << endl;

#if OUTPUT_JP_MESSAGE
        cout << "解答例を得たい場合：" << endl;
#else
        cout << "To obtain an example solution:" << endl;
#endif
        HelpMessageGetSol(programName);
        cout << endl;
    }

}



int main(int argc, char* argv[]) {
    //       傳輸Json給網站功能
    #ifdef COMMUNICATE_WITH_WEBSITE
        using ordered_json = nlohmann::ordered_json;
        ordered_json allPuzzles = Data::convertCsvToJson("C:\\Geister-Endgame-Puzzle\\result.csv", "result.json");

        if (allPuzzles == nullptr) {
            cout << "ERROR : No Data in answer.csv" << endl;
            return 0;
        }

        ApiUploader uploader("jerrykuo123.xyz", 8000);
        uploader.sendJsonData("/upload?mode=overwrite", allPuzzles);

        return 0;
    #endif // COMMUNICATE_WITH_WEBSITE

    initializeManhattanDistance();

    vector<string> argvVec;
    for (int i = 0; i < argc; i++) {
		if (i == 1) { continue; } // 排除第一個參數，因為它是模式關鍵字
        argvVec.push_back(string(argv[i]));
    }
    
	if (argc < 2) {
        PrintHelpMessage(argv[0]);
        return 0;
    }
    
    // 根據第一個參數選擇模式
    if (argv[1] == string("RandomGen")) {
        RandomGeneration(argvVec);
    }
    else if (argv[1] == string("GetSol")) {
        GetSolution(argvVec);
    }
    else if (argv[1] == string("AnswerGen")) {
		// 這個模式呼叫格式是：AnswerGen [input1.txt] [input2.txt] ... [output CSV path] [mode]  中間可以插入任意數量的輸入檔案
        // 示範 :C:\Geister-Endgame-Puzzle\x64\Release\GeisterEndgamePuzzle.exe AnswerGen normal/1221/5.txt normal/1221/7.txt result.csv n
        RunAnswerGen(argvVec);
    }
    else {
		PrintHelpMessage(argv[0]);
        return 0;
    }

    return 0;
}
