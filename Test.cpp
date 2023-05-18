/**
 * @brief	T树功能测试
 * @author	huangxx
*/


#include "ttree.h"
#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <chrono>

#include <memory>

struct Record
{
	int 	pk;
	//char 	idx1Str[32];
};

int pkComparator(const void* pa, const void* pb)
{
	return ((const Record*)pa)->pk - ((const Record*)pb)->pk;
}

// int idx1Comparator(const void* pa, const void* pb)
// {
// 	return strncmp(((const Record*)pa)->idx1Str, ((const Record*)pb)->idx1Str, sizeof(((const Record*)pa)->idx1Str));
// }

bool CheckNode(TTreeNode* pNode, fnKeyComparator keyComp)
{
	//检查节点内的key是否有序
	for(int i = 0; i < pNode->keyNum - 1; i++)
	{
		if(keyComp(pNode->keys[i + 1], pNode->keys[i]) < 0)
		{
			return false;
		}
	}

	//检查左右子树平衡度
	int diff = TTREE_HEIGHT_OF(pNode->left) - TTREE_HEIGHT_OF(pNode->right);
	if(diff > 1 || diff < -1)
	{
		return false;
	}

	//左子树不大于当前节点的最小值
	if(pNode->left && (keyComp(pNode->FirstKey(), pNode->left->LastKey()) < 0))
	{
		return false;
	}

	//右子树不小于当前节点的最大值
	if(pNode->right && (keyComp(pNode->LastKey(), pNode->right->FirstKey()) > 0))
	{
		return false;
	}

	return ((pNode->left == nullptr) ? true : CheckNode(pNode->left, keyComp)) && ((pNode->right == nullptr) ? true : CheckNode(pNode->right, keyComp));
}

TEST(LeftRotate, SingleNode)
{
	TTree tree(pkComparator, true, 3);

	Record records[4];

	for(int i = 0; i < 4; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	ASSERT_EQ(tree.Count(), 4);
}

TEST(LeftRotate, RRShape)
{
	TTree tree(pkComparator, true, 3);

	Record records[7];

	for(int i = 0; i < 7; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	/**
	 * 		|4,5,6|
	 * 	/			\
	 * |1,2,3|			|7|
	*/

	TTreeNode* pRoot = tree.m_pRootNode;
	ASSERT_EQ(pRoot->keyNum, 3);
	ASSERT_EQ(((Record*)pRoot->keys[0])->pk, 4);
	ASSERT_EQ(((Record*)pRoot->keys[1])->pk, 5);
	ASSERT_EQ(((Record*)pRoot->keys[2])->pk, 6);

	TTreeNode* pLeft = pRoot->left;
	ASSERT_EQ(pLeft->keyNum, 3);
	ASSERT_EQ(((Record*)pLeft->keys[0])->pk, 1);
	ASSERT_EQ(((Record*)pLeft->keys[1])->pk, 2);
	ASSERT_EQ(((Record*)pLeft->keys[2])->pk, 3);

	TTreeNode* pRight = pRoot->right;
	ASSERT_EQ(pRight->keyNum, 1);
	ASSERT_EQ(((Record*)pRight->keys[0])->pk, 7);
}

TEST(LeftRotate, RLShape)
{
	TTree tree(pkComparator, true, 3);

	Record records[7];

	for(int i = 0; i < 3; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	for(int i = 4; i < 7; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	for(int i = 3; i < 4; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	/**
	 * 		|4|
	 * 	/			\
	 * |1,2,3|			|5,6,7|
	*/

	TTreeNode* pRoot = tree.m_pRootNode;
	ASSERT_EQ(pRoot->keyNum, 1);
	ASSERT_EQ(((Record*)pRoot->keys[0])->pk, 4);

	TTreeNode* pLeft = pRoot->left;
	ASSERT_EQ(pLeft->keyNum, 3);
	ASSERT_EQ(((Record*)pLeft->keys[0])->pk, 1);
	ASSERT_EQ(((Record*)pLeft->keys[1])->pk, 2);
	ASSERT_EQ(((Record*)pLeft->keys[2])->pk, 3);

	TTreeNode* pRight = pRoot->right;
	ASSERT_EQ(pRight->keyNum, 3);
	ASSERT_EQ(((Record*)pRight->keys[0])->pk, 5);
	ASSERT_EQ(((Record*)pRight->keys[1])->pk, 6);
	ASSERT_EQ(((Record*)pRight->keys[2])->pk, 7);
}

TEST(RightRotate, LLShape)
{
	TTree tree(pkComparator, true, 3);

	Record records[7];

	for(int i = 7; i > 0; i--)
	{
		records[i - 1].pk = i;
		tree.Insert(&records[i - 1]);
	}

	/**
	 * 		|2,3,4|
	 * 	/			\
	 * |1|			|5,6,7|
	*/

	TTreeNode* pRoot = tree.m_pRootNode;
	ASSERT_EQ(pRoot->keyNum, 3);
	ASSERT_EQ(((Record*)pRoot->keys[0])->pk, 2);
	ASSERT_EQ(((Record*)pRoot->keys[1])->pk, 3);
	ASSERT_EQ(((Record*)pRoot->keys[2])->pk, 4);

	TTreeNode* pLeft = pRoot->left;
	ASSERT_EQ(pLeft->keyNum, 1);
	ASSERT_EQ(((Record*)pLeft->keys[0])->pk, 1);


	TTreeNode* pRight = pRoot->right;
	ASSERT_EQ(pRight->keyNum, 3);
	ASSERT_EQ(((Record*)pRight->keys[0])->pk, 5);
	ASSERT_EQ(((Record*)pRight->keys[1])->pk, 6);
	ASSERT_EQ(((Record*)pRight->keys[2])->pk, 7);
}

TEST(RightRotate, LRShape)
{
	TTree tree(pkComparator, true, 3);

	Record records[7];

	for(int i = 4; i < 7; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	for(int i = 0; i < 3; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	for(int i = 3; i < 4; i++)
	{
		records[i].pk = i + 1;
		tree.Insert(&records[i]);
	}

	/**
	 * 					|5,6,7|
	 * 		|1,2,3|
	 * 				|4|
	*/

	/**
	 * 		|4|
	 * 	/			\
	 * |1,2,3|		|5,6,7|
	*/

	TTreeNode* pRoot = tree.m_pRootNode;
	ASSERT_EQ(pRoot->keyNum, 1);
	ASSERT_EQ(((Record*)pRoot->keys[0])->pk, 4);

	TTreeNode* pLeft = pRoot->left;
	ASSERT_EQ(pLeft->keyNum, 3);
	ASSERT_EQ(((Record*)pLeft->keys[0])->pk, 1);
	ASSERT_EQ(((Record*)pLeft->keys[1])->pk, 2);
	ASSERT_EQ(((Record*)pLeft->keys[2])->pk, 3);


	TTreeNode* pRight = pRoot->right;
	ASSERT_EQ(pRight->keyNum, 3);
	ASSERT_EQ(((Record*)pRight->keys[0])->pk, 5);
	ASSERT_EQ(((Record*)pRight->keys[1])->pk, 6);
	ASSERT_EQ(((Record*)pRight->keys[2])->pk, 7);
}

//正序插入
TEST(Insert, Seq)
{
	int count = 10000;
	Record* pRecords = new Record[count];
	std::shared_ptr<Record> ptr(pRecords);

	TTree tree(pkComparator, true, 3);

	for(int i = 0; i < count; i++)
	{
		pRecords[i].pk = i;
		tree.Insert(pRecords + i);
	}

	ASSERT_EQ(tree.Count(), count);

	for(int i = 0; i < count; i++)
	{
		ASSERT_EQ(pRecords + i, tree.Query(pRecords + i));
	}

	ASSERT_TRUE(CheckNode(tree.m_pRootNode, pkComparator));
}

//倒序插入
TEST(Insert, Revert)
{
	int count = 10000;
	Record* pRecords = new Record[count];
	std::shared_ptr<Record> ptr(pRecords);

	TTree tree(pkComparator, true, 3);

	for(int i = 0; i < count; i++)
	{
		pRecords[i].pk = count - i;
		tree.Insert(pRecords + i);
	}

	ASSERT_EQ(tree.Count(), count);

	for(int i = 0; i < count; i++)
	{
		ASSERT_EQ(pRecords + i, tree.Query(pRecords + i));
	}

	ASSERT_TRUE(CheckNode(tree.m_pRootNode, pkComparator));


}

//随机插入
TEST(Insert, Random)
{
	int count = 100000;
	std::vector<int> vec(count);

	Record* pRecords = new Record[count];
	std::shared_ptr<Record> ptr(pRecords);

	TTree tree(pkComparator, true, 3);

	for(int i = 0; i < count; i++)
	{
		vec[i] = i;
	}

	auto seed = std::chrono::system_clock::now().time_since_epoch().count();
	std::shuffle(vec.begin(), vec.end(), std::default_random_engine(seed));

	for(int i = 0; i < count; i++)
	{
		pRecords[i].pk = vec[i];
		tree.Insert(pRecords + i);
	}

	ASSERT_EQ(tree.Count(), count);

	for(int i = 0; i < count; i++)
	{
		ASSERT_EQ(pRecords + i, tree.Query(pRecords + i));
	}

	ASSERT_TRUE(CheckNode(tree.m_pRootNode, pkComparator));
}


int main(int argc, char** argv)
{
	::testing::InitGoogleTest(&argc, argv);
	
	return RUN_ALL_TESTS();
}