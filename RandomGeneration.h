#pragma once
#include "IncludeLib.h"
#include "Random.h"
#include "Search.h"

//用於「非公開模式（Normal）」。在 36 格的棋盤上，隨機放置我方的藍棋（B）、紅棋（R），並將敵方所有的棋子（無論紅藍）全部標記為未知的狀態（'u'）。
inline std::string makeNormalBoard(std::string board, int myBnum, int myRnum, int eBnum, int eRnum) {
	std::random_device rnd;
	int r = 0;
	for (int i = 0; i < myBnum; i++) {
		do r = IntRandom::get_rand_range(0, 35); // edit: 把從1開始改為0，更一致
		while (board[r] != '.' || r == 0 || r == 5);
		board[r] = 'B';  
	}
	for (int i = 0; i < myRnum; i++) {
		do r = IntRandom::get_rand_range(0, 35);
		while (board[r] != '.');
		board[r] = 'R';
	}

	for (int i = 0; i < eBnum + eRnum; i++) {
		do r = IntRandom::get_rand_range(0, 35);
		while (board[r] != '.');
		board[r] = 'u';
	}
	return board;
}

// 用於「部分公開模式（Partial）」。除了放置我方棋子外，還會根據傳入的參數，隨機揭露一定數量的敵方紅棋（'r'）與敵方藍棋（'b'），剩下的敵方棋子才標記為未知（'u'）
inline std::string makePartBoard(std::string board, int myBnum, int myRnum, int eBnum, int eRnum, int openB, int openR) {
	std::random_device rnd;
	int r = 0;
	for (int i = 0; i < myBnum; i++) {
		do r = IntRandom::get_rand_range(0, 35); // edit: 把從1開始改為0，更一致
		while (board[r] != '.' || r == 0 || r == 5);
		board[r] = 'B';
	}
	for (int i = 0; i < myRnum; i++) {
		do r = IntRandom::get_rand_range(0, 35);
		while (board[r] != '.');
		board[r] = 'R';
	}

	for (int i = 0; i < openR; i++) {
		do r = IntRandom::get_rand_range(0, 35);
		while (board[r] != '.');
		board[r] = 'r';
	}
	for (int i = 0; i < openB; i++) {
		do r = IntRandom::get_rand_range(0, 35);
		while (board[r] != '.');
		board[r] = 'b';
	}
	for (int i = 0; i < eBnum + eRnum - openR - openB; i++) {
		do r = IntRandom::get_rand_range(0, 35);
		while (board[r] != '.');
		board[r] = 'u';
	}
	return board;
}

// 當使用者在命令列執行程式且參數給錯時，印出使用教學
inline void HelpMessageRandGen(const std::string& prog_name)
{
	std::cout << "Usage: " << prog_name << " RandomGen" <<
		" [mode] [#B] [#R] [#b] [#r] [min #actions] [max #depth] [#trails] [(random seed)]" << std::endl;
#if OUTPUT_JP_MESSAGE
	std::cout << "- mode: normal or partial" << std::endl;
	std::cout << "  * normal: 非公開問題" << std::endl;
	std::cout << "  * partial: 一部公開問題" << std::endl;
	std::cout << "- #B: 整数，自青駒の数" << std::endl;
	std::cout << "- #R: 整数，自赤駒の数" << std::endl;
	std::cout << "- #b: 整数，敵青駒の数" << std::endl;
	std::cout << "- #r: 整数，敵赤駒の数" << std::endl;
	std::cout << "- min #actions: 整数，最低手数" << std::endl;
	std::cout << "- max #depth: 19 までの整数，最大探索深さ" << std::endl;
	std::cout << "- #trails: 整数，試行回数（失敗問も含む）" << std::endl;
	std::cout << "- random seed: 整数，乱数種 (選択的)" << std::endl;
	std::cout << "  * 指定がなければ time(NULL) になり，記録する．" << std::endl;
	std::cout << "例：" << prog_name << " RandomGen normal 1 2 2 1 5 9 500 0" << std::endl;
	std::cout << "5~9手自青駒1自赤駒2敵青駒2敵赤駒1の非公開問題を500試行から作ってみる（乱数種は0）．" << std::endl;
#else
	std::cout << "- mode: normal or partial" << std::endl;
	std::cout << "  * normal: not-revealed" << std::endl;
	std::cout << "  * partial: partly-revealed" << std::endl;
	std::cout << "- #B: integer, the number of the player's blue pieces" << std::endl;
	std::cout << "- #R: integer, the number of the player's red pieces" << std::endl;
	std::cout << "- #b: integer, the number of the opponent's blue pieces" << std::endl;
	std::cout << "- #r: integer, the number of the opponent's red pieces" << std::endl;
	std::cout << "- min #actions: integer, the minimum move number required" << std::endl;
	std::cout << "- max #depth: integer up to 19, the maximum move number to search" << std::endl;
	std::cout << "- #trails: integer, the number of trials (including failed ones)" << std::endl;
	std::cout << "- random seed: integer, random seed (optional)" << std::endl;
	std::cout << "  * If not specified, time(NULL) is used, which will be recorded." << std::endl;
	std::cout << "Example: " << prog_name << " RandomGen normal 1 2 2 1 5 9 500 0" << std::endl;
	std::cout << "Try to make 5-9 moves not-revealed puzzles with 1 own blue piece, 2 own red pieces,"
		<< " 2 opponent blue pieces, and 1 opponent red piece from 500 trials (random seed of 0)." << std::endl;
#endif
}

// 負責將 AI 生成與解題的統計結果（包含解題步數分佈、花費時間、搜尋節點數等）寫入 .csv 檔案
inline void OutputCount(const std::string& puzzleType, const std::string& countType,
	int myBnum, int myRnum, int eBnum, int eRnum,
	int minActionNum, int maxActionNum,
	int count[20], int trialNum, double timeS,
	unsigned long long sumCalCount, double sumTime)
{
	std::string resultFileName = puzzleType + "-" + countType + ".csv";
	if (!std::ifstream(resultFileName.c_str())) {
		std::ofstream ofsResult(resultFileName.c_str(), std::ofstream::out);
		ofsResult << "BRbr,#B,#R,#b,#r,#minAction,#maxAction";
		for (int j = 3; j <= 19; j += 2) {
			ofsResult << "," << j << "-move";
		}
		ofsResult << ",#trials,totalTimeS,#nodes,genTimeS" << std::endl;
		ofsResult.close();
	}
	std::ofstream ofsResult(resultFileName.c_str(), std::ofstream::out | std::ofstream::app);
	ofsResult << myBnum << myRnum << eBnum << eRnum
		<< "," << myBnum << "," << myRnum << "," << eBnum << "," << eRnum;
	ofsResult << "," << minActionNum << "," << maxActionNum;
	for (int j = 3; j <= 19; j += 2) {
		ofsResult << "," << count[j];
	}
	ofsResult << "," << trialNum;
	ofsResult << "," << timeS;
	ofsResult << "," << sumCalCount;
	ofsResult << "," << sumTime << std::endl;
	ofsResult.close();
}


// 整個隨機生成謎題（或稱盤面）的核心控制器
inline void RandomGeneration(const std::vector<std::string>& argvVec) {
	if (argvVec.size() != 9 && argvVec.size() != 10) { HelpMessageRandGen(argvVec[0]); }

	std::string mode;
	std::string board;
	if (argvVec[1] == std::string("normal")) { mode = "n"; }
	else if (argvVec[1] == std::string("partial")) { mode = "p"; }
	else {
		HelpMessageRandGen(argvVec[0]);
		return;
	}

	int myBnum, myRnum;
	int eBnum, eRnum;
	int openBnum, openRnum;
	int maxDepth;
	int trialNum;
	int actionNum, minAction;
	int makeCountAll = 0;
	int makeCount[20] = {};
	memset(makeCount, 0, sizeof(makeCount));
	int redWallCount[20] = {};
	memset(redWallCount, 0, sizeof(redWallCount));
	int captureBlueCount[20] = {};
	memset(captureBlueCount, 0, sizeof(captureBlueCount));
	int bothCount[20] = {};
	memset(bothCount, 0, sizeof(bothCount));
	double makeTimeCount[20] = {};
	memset(makeTimeCount, 0, sizeof(makeTimeCount));

	unsigned int calCount;
	CN<int> maxpn, maxdn;

	myBnum = stoi(argvVec[2]);
	myRnum = stoi(argvVec[3]);
	eBnum = stoi(argvVec[4]);
	eRnum = stoi(argvVec[5]);
	minAction = stoi(argvVec[6]);
	maxDepth = stoi(argvVec[7]);
	trialNum = stoi(argvVec[8]);
	int randomSeed;
	if (argvVec.size() == 10) {
		randomSeed = stoi(argvVec[9]);
	}
	else {
		struct tm stm;
		time_t tim = time(NULL);
#if defined(_MSC_VER)
		localtime_s(&stm, &tim);
#elif defined(__GNUC__)
		localtime_r(&tim, &stm);
#endif
		char t[100];
		strftime(t, 100, "%Y-%m-%d_%H:%M:%S", &stm);
		randomSeed = int(tim);
		std::ofstream ofsRandSeed("RandSeed.csv", std::ofstream::out | std::ofstream::app);
		ofsRandSeed << myBnum << myRnum << eBnum << eRnum << ","
			<< argvVec[1] << "," << t << "," << randomSeed << std::endl;
		ofsRandSeed.close();
	}
	IntRandom::reset(randomSeed);

	if ((mode == "p") && (eBnum == 1) && (eRnum == 1)) { return; }

	std::string dir_name = argvVec[1];
	makeDir(dir_name.c_str());
	std::string allPuzzleFileName = dir_name + std::string("/") + mode + std::string("-")
		+ std::to_string(myBnum) + std::to_string(myRnum) + std::to_string(eBnum) + std::to_string(eRnum)
		+ std::string("-") + std::to_string(minAction) + std::string("_") + std::to_string(maxDepth) + std::string("ply-")
		+ std::to_string(trialNum) + std::string("-sd=") + std::to_string(randomSeed) + std::string(".txt");
	std::ofstream ofsAllPuzzle;
	ofsAllPuzzle.open(allPuzzleFileName.c_str());
	std::ofstream ofsPly;
	std::ofstream ofsRedWall;
	std::ofstream ofsCaptureBlue;
	std::ofstream ofsBoth;

	double sumTime = 0.0;
	unsigned long long sumCalCount = 0;

	clock_t makeStart;
	clock_t makeEnd;

	clock_t start_clock = clock();
	for (int i = 0; i < trialNum; i++) {
		bool isRedWall = false;
		bool isCaptureBlue = false;

		makeStart = clock();

		makeCountAll += 1;

		Search search(mode, "n");

		board = "";
		for (int j = 0; j < 6; j++) board += "......";

		if (mode == "n") {
			board = makeNormalBoard(board, myBnum, myRnum, eBnum, eRnum);
			actionNum = search.think(board, eBnum, maxDepth);
			calCount = search.returnCount();
			sumCalCount += calCount;

			maxpn = search.returnMaxPn();
			maxdn = search.returnMaxDn();
		}
		else {
			openBnum = 0;
			openRnum = 0;
			while ((openBnum + openRnum) == 0) {
				openBnum = IntRandom::get_rand_range(0, eBnum - 1);
				openRnum = IntRandom::get_rand_range(0, eRnum - 1);
			}

			board = makePartBoard(board, myBnum, myRnum, eBnum, eRnum, openBnum, openRnum);

			actionNum = search.think(board, eBnum, maxDepth);
			calCount = search.returnCount();
			sumCalCount += calCount;

			maxpn = search.returnMaxPn();
			maxdn = search.returnMaxDn();
		}

		makeEnd = clock();
		sumTime += static_cast<double>((makeEnd - makeStart) / double(CLOCKS_PER_SEC));

		if (minAction > actionNum) { continue; }

		if (mode == "n") {
			Search subSearch("n", "r");
			int actionNumRed = subSearch.think(board, eBnum, actionNum);
			if (actionNumRed == 0) {
				isRedWall = true;
			}
		}
		else if (mode == "p") {
			Search subSearchRed("p", "r");
			int actionNumRed = subSearchRed.think(board, eBnum, actionNum);
			if (actionNumRed == 0) {
				isRedWall = true;
			}
			Search subSearchBlue("p", "a");
			int actionNumBlueCapture = subSearchBlue.think(board, eBnum, actionNum, actionNum - 1);
			if (actionNumBlueCapture == 0) {
				isCaptureBlue = true;
			}
		}

		makeCount[actionNum] += 1;
		if (isRedWall) { redWallCount[actionNum] += 1; }
		if (isCaptureBlue) { captureBlueCount[actionNum] += 1; }
		if (isRedWall && isCaptureBlue) { bothCount[actionNum] += 1; }
		makeTimeCount[actionNum] += static_cast<double>((makeEnd - makeStart) / CLOCKS_PER_SEC * 1000.0);

		std::string sub_dir_name = dir_name + std::string("/")
			+ std::to_string(myBnum) + std::to_string(myRnum)
			+ std::to_string(eBnum) + std::to_string(eRnum);
		makeDir(sub_dir_name.c_str()); //
		std::string puzzle_file_name = sub_dir_name + std::string("/")
			+ std::to_string(actionNum) + std::string(".txt");
		ofsPly.open(puzzle_file_name.c_str(), std::ofstream::out | std::ofstream::app);
		if (isRedWall) {
			puzzle_file_name = sub_dir_name + std::string("/")
				+ std::to_string(actionNum) + std::string("-RedWall.txt");
			ofsRedWall.open(puzzle_file_name.c_str(), std::ofstream::out | std::ofstream::app);
		}
		if (isCaptureBlue) {
			puzzle_file_name = sub_dir_name + std::string("/")
				+ std::to_string(actionNum) + std::string("-CaptureBlue.txt");
			ofsCaptureBlue.open(puzzle_file_name.c_str(), std::ofstream::out | std::ofstream::app);
		}
		if (isRedWall && isCaptureBlue) {
			puzzle_file_name = sub_dir_name + std::string("/")
				+ std::to_string(actionNum) + std::string("-WallCapture.txt");
			ofsBoth.open(puzzle_file_name.c_str(), std::ofstream::out | std::ofstream::app);
		}

		std::stringstream ss;
		//std::cout << actionNum << "手詰  " << i << "問目" << std::endl << std::endl;
#if OUTPUT_JP_MESSAGE
		ss << actionNum << "手詰" << "\n";
#else
		ss << actionNum << " moves" << "\n";
#endif
		ss << "myBnum : " << myBnum << ", myRnum : " << myRnum << ", enBnum : " << eBnum << ", enRnum : " << eRnum << ", maxDepth : " << maxDepth << "\n";
		ss << "calCount = " << calCount << "\n";
		ss << "maxpn = " << maxpn << ", maxdn = " << maxdn << "\n";

		for (int j = 0; j < 6; j++) {
			for (int k = 0; k < 6; k++) {
				ss << board[j * 6 + k];
			}
			ss << "\n";
		}
		ss << "\n";
		for (int j = 0; j < 36; j++) {
			ss << board[j];
		}
		ss << "\n" << "\n";

		for (int j = minAction; j < 20; j++) {
			if (makeCount[j] != 0) {
				ss << "actionCount " << j << " : " << makeCount[j] << "\n";
			}
		}
		ss << "\n";

		ofsAllPuzzle << ss.str();
		ofsPly << ss.str();

		std::cout << "CSV WRITE TRIGGERED" << std::endl;

        // ===== 產生 CSV 題目 =====
        std::string boardLine;
        for (int j = 0; j < 36; j++) {
            boardLine += board[j];
        }

        // 檔名：5.csv / 7.csv
        std::string csvFile = sub_dir_name + std::string("/") + std::to_string(actionNum) + ".csv";

        // append 模式
        std::ofstream ofsCsv(csvFile, std::ofstream::out | std::ofstream::app);

        // 這裡你先固定寫死（之後可調整）
        int enemyBlueNum = eBnum;
        int maxDepthForSolve = actionNum * 3; // 你可以自己調

        ofsCsv << boardLine << "," << enemyBlueNum << "," << maxDepthForSolve << std::endl;

        ofsCsv.close();
        
		if (isRedWall) {
			ofsRedWall << ss.str();
			ofsRedWall.close();
		}
		if (isCaptureBlue) {
			ofsCaptureBlue << ss.str();
			ofsCaptureBlue.close();
		}
		if (isRedWall && isCaptureBlue) {
			ofsBoth << ss.str();
			ofsBoth.close();
		}
        std::string solver_file_name = sub_dir_name + std::string("/") + "solver.txt";
        std::ofstream ofsSolver(solver_file_name.c_str(), std::ofstream::out | std::ofstream::app);
        ofsSolver << board << "," << eBnum << "," << maxDepth << std::endl;
        ofsSolver.close();
	}
	clock_t end_clock = clock();
	ofsAllPuzzle.close();

	double timeS = double(end_clock - start_clock) / double(CLOCKS_PER_SEC);
	OutputCount(argvVec[1], "All", myBnum, myRnum, eBnum, eRnum, minAction, maxDepth, makeCount, trialNum, timeS, sumCalCount, sumTime);
	OutputCount(argvVec[1], "RedWall", myBnum, myRnum, eBnum, eRnum, minAction, maxDepth, redWallCount, trialNum, timeS, sumCalCount, sumTime);
	if (std::string(argvVec[1]) == std::string("partial")) {
		OutputCount(argvVec[1], "CaptureBlue", myBnum, myRnum, eBnum, eRnum, minAction, maxDepth, captureBlueCount, trialNum, timeS, sumCalCount, sumTime);
		OutputCount(argvVec[1], "Both", myBnum, myRnum, eBnum, eRnum, minAction, maxDepth, bothCount, trialNum, timeS, sumCalCount, sumTime);
	}
}

