#pragma once
#include "IncludeLib.h"
#include "Bitboard.h"
#include "HashTable.h"
#include "CN.h"
#include "Node.h"

// 定義證明數 (Proof Number) 與反證數 (Disproof Number) 的無限大常數
template<>
const int                   CN<int>::Immense = 0x80000;

class Search {
	// 預先計算好的 6x6 棋盤相鄰位置表 (包含上下左右方向)，用於加速合法步生成
	int kiki[36 * 5];
	unsigned long long calCount = 0;
	unsigned long long lastCalCount = 0;
	int maxDepth;

	// 雜湊表 (Transposition Table)，分別記錄 Player (己方) 與 Enemy (敵方) 的盤面狀態以避免重複計算
	Hashmap hashP;
	Hashmap hashE;

	std::string mode;

	unsigned int calPnDnCount = 0;

	bool overFlag = false;

	unsigned int enemyBlue;

	bool needans = false;
	int answerID[50];
	bool needReadableAns = false;
	bool needpndn = false;

	bool pnend = false;
	bool dnend = false;

	// 記憶體池 (Memory Pool)，預先保留空間以避免搜尋過程中頻繁 new/delete 造成效能瓶頸
	std::vector<Node*> vecNode;

	CN<int> maxrootpn;
	CN<int> maxrootdn;

public:
	std::string answerBoard;
	std::string subMode;
	bool onlyWantSteps = false;

	Search(std::string mode, std::string subMode) {	//ゲームが始まる前の処理
		// 遊戲開始前的初始化設定
		this->mode = mode;
		this->subMode = subMode;

		// 預先配置 6400 萬個節點的記憶體池
		vecNode.reserve(64000000);

		maxrootpn = CN<int>(0);
		maxrootdn = CN<int>(0);

		int y, x, dir, i, j;
		// 網格的四個移動方向：上, 右, 下, 左 (dy, dx 對應)
		int dy[4] = { -1, 0, 1, 0 };
		int dx[4] = { 0, 1, 0, -1 };

		// 初始化 kiki 相鄰位置表：將每一個格子的合法相鄰格子索引記錄下來
		for (i = 0; i < 180; i++) kiki[i] = -1;
		for (y = 0; y < 6; y++) {
			for (x = 0; x < 6; x++) {
				i = y * 6 + x;
				j = 0;
				for (dir = 0; dir < 4; dir++) {
					int ny = y + dy[dir];
					int nx = x + dx[dir];
					if (0 <= ny && ny < 6 && 0 <= nx && nx < 6) {
						kiki[5 * i + j] = ny * 6 + nx;
						j++;
					}
				}
			}
		}
	}


	// 外部呼叫的 AI 思考入口點
	int think(std::string board, int eB, int maxDepth, int minDepth=0) {
		this->maxDepth = maxDepth;
		BitBoard bb;
		bb.toBitBoard(board);

		BitBoard emptybb;
		emptybb.toBitBoard("................................................");

		int i = (mode == "n") ? 2 : 3;
		if (minDepth != 0) i = minDepth;
		int space = 2;

		enemyBlue = eB;
		calCount = 0;

		// 採用反覆運算深化 (Iterative Deepening)，逐步增加搜尋深度
		while (i <= maxDepth) {
			//std::cout << "Depth " << i << std::endl;
			lastCalCount = 0;
			// 每次加深深度時清空雜湊表與重置節點樹
			hashP.clear();
			hashE.clear();
			nodeCount = 1;

			// 建立初始空盤面 (Dummy 節點)
			Node* emptyn = new Node;
			emptyn->initNode(emptybb, 0, 0);
			vecNode.push_back(emptyn);

			// 建立當前盤面的根節點
			Node* n = new Node;
			n->initNode(bb, 0, 1);
			vecNode.push_back(n);

			maxrootpn = 0;
			maxrootdn = 0;
			pnend = false;
			dnend = false;

			// 初始化根節點的證明數 (pn) 與反證數 (dn) 為無限大 (即 -1 的內部表示)
			CN<int> pn, dn;
			pn = pn - 1;
			dn = dn - 1;

			// 從根節點啟動遞迴深化搜尋 (己方視為 OR 節點)
			MIDOr(*n, pn, dn, 0, i, eB);

			// 若證明數小於等於 0，代表已找到必勝解	
			if (pn <= 0) {
				if (needans == true) answerBoard = returnSequenceBoard(i);
				releaseVec(vecNode);
				return ((mode == "n") ? (i + 1) : i);
			}
			releaseVec(vecNode); 

			// 檢查記憶體或計算量是否達到上限
			if (overFlag == true) {
				std::cout << mode << subMode << ", " << bitCount(bb.existB) << bitCount(bb.existR)
					<< eB << (bitCount(bb.existP | bb.existEB | bb.existER) - eB)
					<< ", " << board
					<< ": over" << std::endl;
				overFlag = false;
				break;
			}

			i += space;
		}
		return 0;
	}


	void appendMissingEscapeStep(std::string& solution_action, const std::string& last_board, int& move_id, int actionNum) {
		std::string current_board = last_board;

		BitBoard bb;
		bb.toBitBoard(current_board);

		while (move_id <= actionNum) {  // <= 確保最後一步逃脫也會被執行

			if (move_id % 2 != 0) {
				// 己方回合：找 B 的位置然後逃
				int b_pos = -1;
				for (int k = 0; k < 36; k++) {
					if (current_board[k] == 'B') { b_pos = k; break; }
				}
				if (b_pos == -1) break;

				solution_action += std::to_string(move_id) + ". ";

				if (b_pos == 0) {
					solution_action += "B a6 left   ";
				}
				else if (b_pos == 5) {
					solution_action += "B f6 right   ";
				}
				else {
					// 還沒到出口，往上走
					char src_x = char(b_pos % 6 + 'a');
					char src_y = char((6 - b_pos / 6) + '0');
					solution_action += "B " + std::string(1, src_x) + std::string(1, src_y) + " up   ";

					// 更新盤面
					current_board[b_pos - 6] = 'B';
					current_board[b_pos] = '.';
					bb.toBitBoard(current_board);
				}

			}
			else {
				// 敵方回合：隨便走一個合法步
				int from[32], to[32];
				int moveNum = bb.makeMoves(1, kiki, from, to);

				if (moveNum > 0) {
					int src = from[0];
					int dst = to[0];

					char enemy_p = current_board[src];
					char ex = char(src % 6 + 'a');
					char ey = char((6 - src / 6) + '0');

					std::string direction;
					if (dst - src == -6) direction = "up";
					else if (dst - src == 6) direction = "down";
					else if (dst - src == -1) direction = "left";
					else if (dst - src == 1) direction = "right";

					solution_action += std::to_string(move_id) + ". "
						+ std::string(1, enemy_p) + " " + ex + ey + " " + direction + "   ";

					current_board[dst] = current_board[src];
					current_board[src] = '.';
					bb.toBitBoard(current_board);
				}
			}
			move_id++;
		}
	}
	
	
	// 將找到的必勝路徑，轉換成人類可讀的連續盤面或操作步驟 (例如 "1. B a6 left")
	std::string returnSequenceBoard(int actionNum) {
		std::string board_sequence;
		std::string solution_action;
		std::string previous_board = vecNode[1]->bb.returnstring();
		int move_id = 1;

		// 找出通往勝利的節點 ID 序列，存入 answerID 陣列
		SearchAnswer(actionNum);

		for (int i = 0; i < 50; i++) {
			if (answerID[i] == 0) break;
			Node* _n;
			_n = new Node;
			*_n = *vecNode[answerID[i]]; 

			if (!onlyWantSteps) {
				for (int grid_id = 0; grid_id < 36; grid_id += 6) {
					if (needReadableAns) { board_sequence.append(std::to_string(6 - grid_id / 6) + " "); }
					board_sequence.append(_n->bb.returnstring().substr(grid_id, 6));
					if (needReadableAns) { board_sequence.append("\n"); }
				}

				if (needReadableAns) { board_sequence.append("  abcdef\n"); }
			}
			board_sequence.append("\n");
			if (needReadableAns) {
				std::string current_board = _n->bb.returnstring();
				int src_id = 36;
				int dst_id = 36;
				char moved_piece;
				int grid_id = 0;
				while (grid_id < 36) {
					char p = previous_board[grid_id];
					if (current_board[grid_id] == p) {
						grid_id++;
						continue;
					}
					if (((i % 2 == 0) && (p == 'B' || p == 'R'))
						|| ((i % 2 == 1) && (p == 'b' || p == 'r' || p == 'u')))
					{
						src_id = grid_id;
						moved_piece = p;
						break;
					}
					grid_id++;
				}
				grid_id = 0;
				while (grid_id < 36) {
					char p = current_board[grid_id];
					if (p != previous_board[grid_id] && p == moved_piece) {
						dst_id = grid_id;
						break;
					}
					grid_id++;
				}
				char src_x = char(src_id % 6 + 'a');
				char src_y = char((6 - src_id / 6) + '0');
				std::string direction;
				if (dst_id - src_id == -6) { direction = "N"; }      // 往上
				else if (dst_id - src_id == 6) { direction = "S"; }  // 往下
				else if (dst_id - src_id == -1) { direction = "W"; } // 往左
				else if (dst_id - src_id == 1) { direction = "E"; }  // 往右
				else { direction = "ERROR"; }
				solution_action += (std::to_string(move_id) + ". " + std::string(1, moved_piece));
				solution_action += (" " + std::string(1, src_x) + std::string(1, src_y) + " " + direction + "   ");
				previous_board = current_board;
				move_id++;
			}
			delete _n;
		}
		// 處理逃脫特殊邏輯 
		if (needReadableAns && actionNum % 2 == 0) {
			if (previous_board[0] == 'B') {
				solution_action += (std::to_string(move_id) + ". B a6 W   "); // 往左
			} else if (previous_board[5] == 'B') {
				solution_action += (std::to_string(move_id) + ". B f6 E   "); // 往右
			}
			else // 新增 為了解決剪枝答案缺失 
			{
				appendMissingEscapeStep(solution_action, previous_board, move_id, actionNum + 1); // actionNum 加一因為沒有算最後一步
			}
		}
		return board_sequence + (needReadableAns ? solution_action : "");
	}

	unsigned long long returnCount(void) { return calCount; }		    // 回傳計算節點的數量
	unsigned long long returnLastCount(void) { return lastCalCount; }   // 回傳必勝步數的計算節點數量
	CN<int> returnMaxPn(void) { return maxrootpn; }			            // 回傳根節點的最大證明數 (Proof Number)
	CN<int> returnMaxDn(void) { return maxrootdn; }			            // 回傳根節點的最大反證數 (Disproof Number)
	void setNeedAns(bool ans) { this->needans = ans; }		            // 切換是否需要解答範例。預設為關閉
	void setNeedReadableAns(bool ans) { this->needReadableAns = ans; }  // 設定解答範例中是否加入換行。預設為關閉
	void setNeedPnDn(bool pndn) { this->needpndn = pndn; }	            // 切換是否要計算根節點的最大證明數與反證數。預設為關閉

private:

	// 在已經證明必勝的樹中，挑選出「最短」的致勝分支
	void SearchAnswer(int actionNum) {
		int turn = 0;
		int id = 1;
		int depth = 0;

		for (int a = 0; a < 50; a++) {
			answerID[a] = 0;
		}

		//std::cout << "searchStart" << std::endl;
		CalDepthToGoal(1, 0, 0, actionNum);
		//std::cout << "selectStart" << std::endl;

		while (1) {
			int _depthToGoalAnd = 100;
			int _depthToGoalOr = -1;

			Node* n;
			n = new Node;
			*n = *vecNode[id];
			for (int i = 0; i < 32; i++) {
				if (n->childID[i] == 0) break;
				Node nChild = *vecNode[n->childID[i]];

				if (nChild.bb.existR == 0ll) { continue; }

				if (nChild.dn.isinf() == true) {
					if (turn == 0 && nChild.depthToGoal < _depthToGoalAnd) {
						_depthToGoalAnd = nChild.depthToGoal;
						answerID[depth] = n->childID[i];
						id = n->childID[i];
					}
					else if (turn == 1 && nChild.depthToGoal > _depthToGoalOr) {
						Search* s;
						s = new Search(mode, subMode);
						bool is_skip =
							(s->think(nChild.bb.returnstring(), enemyBlue, nChild.depthToGoal - 1) != 0);
						delete s;
						if (is_skip) { continue; }
						_depthToGoalOr = nChild.depthToGoal;
						answerID[depth] = n->childID[i];
						id = n->childID[i];
					}
				}
			}
			delete n;
			if (answerID[depth] == 0) break;

			depth += 1;
			turn = 1 - turn;
		}
	}


	// 遞迴計算每個節點到達成目標 (獲勝) 的步數 (Minimax 邏輯)
	// 用於輔助 SearchAnswer 找出最短贏法，而不是無意義的繞圈子
	void CalDepthToGoal(int id, int depth, int turn, int actionNum) {
		int _depthToGoal;
		if (turn == 0) _depthToGoal = 300; // 己方找最小，初始設很大
		else _depthToGoal = -300;          // 敵方找最大，初始設很小

		Node* n;
		n = new Node;
		*n = *vecNode[id];

		int i = 0;

		for (i = 0; i < 32; i++) {
			if (n->childID[i] == 0) break;
			if (vecNode[n->childID[i]]->dn.isinf() == true) {
				CalDepthToGoal(n->childID[i], depth + 1, 1 - turn, actionNum);
				Node* nChild;
				nChild = new Node;
				*nChild = *vecNode[n->childID[i]];
				if (turn == 0 && nChild->depthToGoal != -1 && nChild->depthToGoal <= _depthToGoal) {
					_depthToGoal = nChild->depthToGoal;
					vecNode[id]->depthToGoal = nChild->depthToGoal + 1;

				}
				else if (turn == 1 && nChild->depthToGoal >= _depthToGoal && nChild->depthToGoal != -1) {
					_depthToGoal = nChild->depthToGoal;
					vecNode[id]->depthToGoal = nChild->depthToGoal + 1;
				}
				else if (turn == 1 && nChild->depthToGoal == -1) {
					_depthToGoal = 100;
					vecNode[id]->depthToGoal = -1;
				}
				delete nChild;
			}
			else if (turn == 1) {
				_depthToGoal = 100;
				vecNode[id]->depthToGoal = -1;
			}
		}
		if (i == 0 && (n->bb.check(turn, enemyBlue, "n") == 0 || n->bb.check(turn, enemyBlue, "n") == 2)) {
			vecNode[id]->depthToGoal = 0;
		}
		delete n;
	}

	// --- 根節點 PN/DN 定期更新區塊 ---
	// 為了監控搜尋進度，定期重新計算整棵樹根節點的真實 PN 與 DN
	void calRootPnDn(int maxdepth) {
		if (maxrootpn.isinf() == false && pnend == false) {
			CN<int> rootpn = calPn(1, 0, maxdepth);
			if (rootpn.isinf() == true)	pnend = true;
			else if (maxrootpn < rootpn) maxrootpn = rootpn;
		}
		if (maxrootdn.isinf() == false && dnend == false) {
			CN<int> rootdn = calDn(1, 0, maxdepth);
			if (rootdn.isinf() == true)	dnend = true;
			else if (maxrootdn < rootdn) maxrootdn = rootdn;
		}
	}

	// 計算根節點的證明數 (Proof Number)
	// 概念： OR 節點 (己方) 的證明數 = 子節點證明數的最小值
	//       AND 節點 (敵方) 的證明數 = 子節點證明數的總和
	CN<int> calPn(unsigned int id, int depth, int maxdepth) {
		CN<int> minimum;
		CN<int> sum(0);
		Node _n = *vecNode[id];

		for (int i = 0; i < 32; i++) {
			if (_n.childID[i] == 0) break;
			if (depth % 2 == 0) {                // OR 節點 (己方回合) -> 找最小值
				if (minimum <= maxrootpn) break; // 剪枝優化
				if (vecNode[_n.childID[i]] == NULL) {}
				else if (vecNode[_n.childID[i]]->expanded == true && depth < maxDepth) {
					minimum = std::min(calPn(_n.childID[i], depth + 1, maxdepth), minimum);
				}
				else minimum = std::min(vecNode[_n.childID[i]]->pn, minimum);
			}
			else {                               // AND 節點 (敵方回合) -> 求總和
				if (sum.isinf() == true) break;  // 如果有無限大，總和就是無限大
				if (vecNode[_n.childID[i]] == NULL) sum += CN<int>();
				else if (vecNode[_n.childID[i]]->expanded == true && depth < maxDepth) {
					sum += calPn(_n.childID[i], depth + 1, maxdepth);
				}
				else sum += vecNode[_n.childID[i]]->pn;
			}
		}
		if (depth % 2 == 0) {
			return minimum;
		}
		else {
			return sum;
		}
	}

	// 計算根節點的反證數 (Disproof Number)
	// 概念： OR 節點 (己方) 的反證數 = 子節點反證數的總和 (必須所有選項都失敗才算失敗)
	//       AND 節點 (敵方) 的反證數 = 子節點反證數的最小值 (只要有一招防住就算防守成功)
	CN<int> calDn(unsigned int id, int depth, int maxdepth) {
		CN<int> minimum;
		CN<int> sum(0);

		Node _n = *vecNode[id];

		for (int i = 0; i < 32; i++) {
			if (_n.childID[i] == 0) break;

			if (depth % 2 == 1) {                // AND 節點 (敵方回合) -> 找最小值
				if (minimum <= maxrootdn) break;
				if (vecNode[_n.childID[i]] == NULL) minimum = 0;
				else if (vecNode[_n.childID[i]]->expanded == true && depth < maxdepth) {
					minimum = std::min(calDn(_n.childID[i], depth + 1, maxdepth), minimum);
				}
				else minimum = std::min(vecNode[_n.childID[i]]->dn, minimum);
			}
			else {                               // OR 節點 (己方回合) -> 求總和
				if (sum.isinf() == true) break;
				if (vecNode[_n.childID[i]] == NULL) {}
				else if (vecNode[_n.childID[i]]->expanded == true && depth < maxdepth) {
					sum += calDn(_n.childID[i], depth + 1, maxdepth);
				}
				else sum += vecNode[_n.childID[i]]->dn;
			}
		}
		if (depth % 2 == 1) return minimum;
		else return sum;
	}

	// [ AND 節點計算 ] : 反證數 (DN) 取子節點 DN 的最小值 (對手只要找到一步能成功防禦，就能反證我們的攻擊)
	CN<int> getMinChildDN(Node& n) {
		CN<int> minimum;

		for (int i = 0; i < 32; i++) {
			if (n.childID[i] == 0) break;
			if (vecNode[n.childID[i]] == NULL) {  // NULL 代表這是一個「不合法」或「已經確定玩家必敗」的無效節點
				minimum = 0;
			}
			else {
				minimum = std::min(vecNode[n.childID[i]]->dn, minimum);
			}
		}
		return minimum;
	}

	// [ OR 節點計算 ] : 證明數 (PN) 取子節點 PN 的最小值 (己方只要找到一步能贏，就能證明這個局面獲勝)
	CN<int> getMinChildPN(Node& n) {
		CN<int> minimum;

		for (int i = 0; i < 32; i++) {
			if (n.childID[i] == 0) break;
			if (vecNode[n.childID[i]] == NULL) {} // NULL 代表這是一個「不合法」或「已經確定玩家必敗」的無效節點
			else {
				minimum = std::min(vecNode[n.childID[i]]->pn, minimum);
			}
		}
		return minimum;
	}

	// [ OR 節點計算 ] : 反證數 (DN) 取所有子節點 DN 的總和 (必須堵死己方所有的攻擊選項，才能反證這個局面必敗)
	CN<int> getSumChildPN(Node& n) {
		CN<int> sum(0);

		for (int i = 0; i < 32; i++) {
			if (n.childID[i] == 0) break;
			if (vecNode[n.childID[i]] == NULL) {
				sum += CN<int>();
			}
			else {
				sum += vecNode[n.childID[i]]->pn;
			}
		}
		return sum;
	}

	// [ AND 節點計算 ] : 反證數 (DN) 取所有子節點 DN 的總和 (必須所有防禦都被擊破才算獲勝)
	CN<int> getSumChildDN(Node& n) {
		CN<int> sum(0);

		for (int i = 0; i < 32; i++) {
			if (n.childID[i] == 0) break;
			if (vecNode[n.childID[i]] == NULL) {}
			else {
				sum += vecNode[n.childID[i]]->dn;
			}
		}
		return sum;
	}

	// 選擇 OR 節點中最有希望的分支：尋找反證數 (DN) 最小的子節點 (自己是AND節點)
	int SelectChildOr(Node& n, CN<int>& pn, CN<int>& dn2) {
		CN<int> dn1;
		int best = 0;

		pn.infinity();

		for (int i = 0; i < 32; ++i) {
			if (n.childID[i] == 0) break;

			Node _n;
			if (vecNode[n.childID[i]] == NULL) {
				_n.pn = CN<int>();
				_n.dn = CN<int>(0);
			}
			else {
				_n = *vecNode[n.childID[i]];
			}

			if (_n.dn < dn1) {
				best = i;
				pn = _n.pn;
				dn2 = dn1;
				dn1 = _n.dn;
			}
			else if (_n.dn < dn2) {
				dn2 = _n.dn;
			}
		}
		return best;
	}

	// 選擇 AND 節點中最有希望的分支：尋找證明數 (PN) 最小的子節點 (自己是OR節點)
	int SelectChildAnd(Node& n, CN<int>& dn, CN<int>& pn2) {
		CN<int> pn1;
		int best = 0;

		dn.infinity();

		for (int i = 0; i < 32; ++i) {
			if (n.childID[i] == 0) break;

			Node _n;
			if (vecNode[n.childID[i]] == NULL) {
				_n.pn = CN<int>();
				_n.dn = CN<int>(0);
			}
			else {
				_n = *vecNode[n.childID[i]];
			}

			if (_n.pn < pn1) {
				best = i;
				dn = _n.dn;
				pn2 = pn1;
				pn1 = _n.pn;
			}
			else if (_n.pn < pn2) {
				pn2 = _n.pn;
			}
		}
		return best;
	}

	// 多重反覆運算深化 OR 節點 (Multiple Iterative Deepening OR-node) - 模擬己方行動
	void MIDOr(Node& n, CN<int>& pn, CN<int>& dn, int depth, int maxdepth, int enemyBlue) {
		calCount += 1;
		lastCalCount += 1;
		if (overFlag == true) {
			return;
		}

		if (vecNode.size() >= 56000000) {
			overFlag = true;
			return;
		}

		// 1. 終止條件判定 (檢查盤面勝負)
		switch (n.bb.check(0, enemyBlue, subMode)) {
			case 0:  // 判定為己方獲勝 (證明成功) -> PN=0, DN=無窮大
				n.pn = 0;
				n.dn.infinity();
				pn = 0;
				dn.infinity();
				PutInHash(hashP, n.bb, n.depth, &n);

				return;
			case 1:  // 判定為敵方獲勝 (反證成功) -> PN=無窮大, DN=0
				n.pn.infinity();
				n.dn = 0;
				pn.infinity();
				dn = 0;
				PutInHash(hashP, n.bb, n.depth, &n);
				return;
			case 2:  // 其他特殊勝利條件 -> 視同己方獲勝
				n.pn = 0;
				n.dn.infinity();
				pn = 0;
				dn.infinity();
				PutInHash(hashP, n.bb, n.depth, &n);

				return;
			default:
				// 剪枝邏輯 (Pruning)：如果判斷己方來不及逃脫或達成目標，提前視為失敗(反證成功)
				int escapeMaxDepth = (maxdepth % 2 == 0) ? maxdepth : (maxdepth - 1);
				bool isMoreThanMinEscape = ((n.bb.minEscapeAction() * 2) > (escapeMaxDepth - depth));
				bool canPrune = ((mode == "n" && isMoreThanMinEscape)
					|| (mode == "p" && isMoreThanMinEscape && ((n.bb.existER == 0)
						|| (n.bb.minCaptureAction() > (maxdepth - depth)))));
				if ((depth == maxdepth) || canPrune) {
					n.pn.infinity();
					n.dn = 0;
					pn.infinity();
					dn = 0;
					PutInHash(hashP, n.bb, n.depth, &n);
					return;
				}
				break;
		}

		// 2. 節點展開 (Node Expansion) - 如果該節點尚未展開，則生成所有合法的下一步
		int from[32], to[32], moveNum;
		if (n.expanded == false) {
			moveNum = n.bb.makeMoves(0, kiki, from, to);  // 產生移動步
			n.bb.moveOrder(0, moveNum, from, to);         // 排序移動步(提高Alpha-Beta/df-pn剪枝效率)
			makeChild(hashE, &n, from, to, moveNum, maxdepth, vecNode);  // 產生子節點

			// 定期檢查與更新根節點的 PN/DN 狀態
			calPnDnCount++;
			if ((maxdepth <= 7 || (maxdepth <= 11 && calPnDnCount % 100 == 0) || (maxdepth <= 17 && calPnDnCount % 1000 == 0) || calPnDnCount % 10000 == 0) && this->needpndn == true) {
				calRootPnDn(maxdepth);
			}
		}

		// 將盤面存入己方雜湊表 (hashP)
		n.setPnDn(CN<int>(), CN<int>(0));
		PutInHash(hashP, n.bb, n.depth, &n);

		CN<int> dmin, psum;
		CN<int> _pn, _dn;


		// 3. 多重反覆運算深化迴圈
		while (1) {
			// OR 節點的當前評估值：PN 應為子節點 PN 最小值，DN 應為子節點 DN 總和
			dmin = getMinChildPN(n);
			psum = getSumChildDN(n);

			// 如果子節點傳回來的評估值已超出當前函數所被允許的閾值 (pn, dn)，就提早 return 交回控制權
			if (pn <= dmin || dn <= psum) {
				// 如果真實難度 (dmin) 已經超過了上層給的容忍上限 (pn)...
				// 代表這條路「比想像中難」，我不玩了！
				pn = dmin;  // 把真實難度回報給上層
				dn = psum;
				n.setPnDn(dmin, psum);
				PutInHash(hashP, n.bb, n.depth, &n);

				return;
			}
			CN<int> _pn2;  // 3. 挑選最優解與第二名
			int best = SelectChildAnd(n, _dn, _pn2);  // 選出最好走的路，並打聽「第二好走的路難度是多少 (_pn2)」

			Node* nextN = vecNode[n.childID[best]]; 
			_dn = (dn - psum) + _dn;   // 4. 計算要發給下一層的「容忍度 (預算)」
			_pn = std::min(pn, _pn2 + CN<int>(1));  // 【DF-PN 最著名的公式】

			MIDAnd(*nextN, _pn, _dn, depth + 1, maxdepth, enemyBlue);  // 下一層

			if (overFlag == true) {
				return;
			}
		}
	}

	// --- 核心遞迴：AND 節點 (敵方回合) ---
	// 多重反覆運算深化 AND 節點 (Multiple Iterative Deepening AND-node) - 模擬敵方應對
	void MIDAnd(Node& n, CN<int>& pn, CN<int>& dn, int depth, int maxdepth, int enemyBlue) {
		calCount += 1;
		lastCalCount += 1;
		if (overFlag == true) {
			return;
		}
		if (vecNode.size() >= 56000000) {
			overFlag = true;
			return;
		}

		// 1. 終止條件與盤面評估 (敵方視角)
		switch (n.bb.check(1, enemyBlue, subMode)) {
		case 0:   // 判定為己方獲勝 (證明成功)
			n.pn = 0;
			n.dn.infinity();
			pn = 0;
			dn.infinity();
			PutInHash(hashE, n.bb, n.depth, &n);
			return;
		case 1:   // 判定為敵方獲勝 (反證成功)
			n.pn.infinity();
			n.dn = 0;
			pn.infinity();
			dn = 0;
			PutInHash(hashE, n.bb, n.depth, &n);
			return;
		case 3:
			n.pn.infinity();
			n.dn = 0;
			pn.infinity();
			dn = 0;
			PutInHash(hashE, n.bb, n.depth, &n);
			return;
		default:
			// 剪枝邏輯 (Pruning)：如果己方來不及逃脫，判定敵方防禦成功 (反證成功)
			int escapeMaxDepth = (maxdepth % 2 == 0) ? maxdepth : (maxdepth - 1);
			bool isMoreThanMinEscape = ((n.bb.minEscapeAction() * 2 + 1) > (escapeMaxDepth - depth));
			bool canPrune = ((mode == "n" && isMoreThanMinEscape)
				|| (mode == "p" && isMoreThanMinEscape && ((n.bb.existER == 0)
					|| (n.bb.minCaptureAction() > (maxdepth - depth)))));
			if ((depth == maxdepth) || canPrune) {
				n.pn.infinity();
				n.dn = 0;
				pn.infinity();
				dn = 0;
				PutInHash(hashE, n.bb, n.depth, &n);
				return;
			}
			break;
		}

		// 2. 節點展開 (Node Expansion) - 生成敵方所有可能的應對步
		int from[32], to[32], moveNum;
		if (n.expanded == false) {
			moveNum = n.bb.makeMoves(1, kiki, from, to);
			n.bb.moveOrder(1, moveNum, from, to);
			makeChild(hashP, &n, from, to, moveNum, maxdepth, vecNode);
		}

		// 將盤面存入敵方雜湊表 (hashE) 避免重複走訪
		n.setPnDn(CN<int>(), CN<int>(0));
		PutInHash(hashE, n.bb, n.depth, &n);

		CN<int> dmin, psum;
		CN<int> _pn, _dn;

		// 3.多重反復深化
		while (1) {
			// AND 節點邏輯：PN 取總和，DN 取最小值
			dmin = getMinChildDN(n);
			psum = getSumChildPN(n);

			if (dn <= dmin || pn <= psum) {
				dn = dmin;
				pn = psum;
				n.setPnDn(psum, dmin);
				PutInHash(hashE, n.bb, n.depth, &n);
				return;
			}

			CN<int> _dn2;
			int best = SelectChildOr(n, _pn, _dn2);

			Node* nextN = vecNode[n.childID[best]];
			_pn = (pn - psum) + _pn;
			_dn = std::min(dn, _dn2 + CN<int>(1));

			MIDOr(*nextN, _pn, _dn, depth + 1, maxdepth, enemyBlue);

			if (overFlag == true) {
				return;
			}
		}
	}
};
