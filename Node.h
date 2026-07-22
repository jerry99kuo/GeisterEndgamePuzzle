#pragma once
#include "IncludeLib.h"
#include "CN.h"
#include "HashTable.h"

struct Node {
	// 記錄這個節點是否已經「展開過」 (也就是是否已經生成過子節點了)
	// DF-PN 會優先探索未展開的節點，展開後此標記設為 true
	bool expanded = false;

	// 這個節點的唯一識別碼 (ID)。
	// 在這個系統中，ID 同時也是這個節點在記憶體池 (vecNode) 中的索引位置。
	unsigned int id;

	// 證明數 (Proof Number, pn) 與 反證數 (Disproof Number, dn)
	// 用來評估這個節點「距離被證明必勝或必敗，還有多困難」
	CN<int> pn, dn;

	// 這個節點在搜尋樹中的深度 (距離初始盤面走了幾步)
	int depth;

	// 這個節點所代表的棋盤狀態 (位元棋盤)
	BitBoard bb;

	// 儲存所有合法下一步 (子節點) 的 ID 陣列。
	// 因為 6x6 棋盤遊戲的合法步數通常不會超過 32 步，所以固定開 32 的大小。
	// 預設為 0，代表沒有子節點。
	unsigned int childID[32] = {};

	// 距離達成目標 (獲勝) 的最短步數，預設為 -1 (尚未計算或無法獲勝)。
	// 這是為了最後回溯「最短致勝路徑」所記錄的分數。
	int depthToGoal = -1;

	// 初始化節點的基本狀態
	void initNode(BitBoard bb, int depth, int ID) {
		// 在 DF-PN 演算法中，一個「尚未展開」的未知節點，
		// 它的證明數與反證數預設皆為 1 (代表只要再探索 1 步就有機會證明)
		this->pn = 1;
		this->dn = 1;

		this->expanded = false;

		this->bb = bb;

		this->depth = depth;

		this->id = ID;
	}

	// 快速更新證明數與反證數的輔助函式
	void setPnDn(CN<int> pn, CN<int> dn) {
		this->pn = pn;
		this->dn = dn;
	}
};

// 根據傳入的合法步 (from, to)，生成這個節點的所有下一步，並記錄在 childID 中。 // 增加inline
inline bool makeChild(Hashmap& map, Node* n, int from[], int to[], int moveNum, int maxDepth, std::vector<Node*>& vecNode) {

	// 只有在還沒展開過的情況下才需要生成
	if (n->expanded == false) {
		
		for (int i = 0; i < moveNum; i++) {
			// 先備份目前的盤面狀態
			BitBoard bbtmp = n->bb;

			// 在盤面上實際模擬這一步棋 (移動棋子)
			n->bb.move(from[i], to[i]);

			//去雜湊表(Hash Table) 查查看，這個走出來的新盤面，以前是不是算過
			Node* cnp = LookUpHash(map, n->bb, n->depth + 1);

			// 如果 cnp != NULL，代表雜湊表裡有找到一模一樣的盤面！
			if (cnp != NULL) {
				// 直接把那個已經存在節點的 ID 抄過來當作自己的子節點
				n->childID[i] = cnp->id;
				if (n->childID[i] == -1) {
					std::cout << "error" << std::endl;
				}
			}
			else {
				// 雜湊表裡沒找到，代表這是一個從未見過的全新盤面
				Node* child = NULL;
				child = new Node;

				nodeCount += 1; // 全域節點總數 +1，作為新節點的 ID

				// 初始化這個小孩
				child->initNode(n->bb, n->depth + 1, nodeCount);
				
				// 把小孩的 ID 放到爸爸的 childID 陣列裡
				n->childID[i] = nodeCount;

				// 將小孩實體推入全域記憶體池 (vecNode) 中保存
				vecNode.push_back(child);

				// 將這個全新盤面註冊到雜湊表中
				PutInHash(map, child->bb, child->depth, child);
			}

			// 模擬完這一步後，把盤面「復原」，準備模擬下一個合法步
			n->bb = bbtmp;
		}

		// 所有可能的小孩都生成完畢，標記為已展開
		n->expanded = true;
	}
	return false;
}


// 記憶體釋放//增加inline
inline void releaseVec(std::vector<Node*>& vecNode) {

	for (int i = 0; i < vecNode.size(); i++) {
		if (vecNode[i] != NULL) {
			delete vecNode[i];
			vecNode[i] = NULL;
		}
	}
	vecNode.clear();
}
