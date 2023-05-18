/**
 * @brief	T树实现
 * @author	huangxx
*/

#ifndef __TTREE_H__
#define __TTREE_H__

#include <stdlib.h>
#include <vector>


typedef int (*fnKeyComparator)(const void* pa, const void* pb);

#define TTREE_HEIGHT_OF(node) (node == nullptr ? 0 : node->height)
#define MAX(a, b) (a > b ? a : b)


struct TTreeNode
{
	int				height;		//高度
	unsigned int	keyNum;		//key数量
	void**			keys;		//key指针

	union {
		TTreeNode* children[2];
		struct
		{
			TTreeNode* left;
			TTreeNode* right;
		};
	};

	TTreeNode* parent;

	void* FirstKey()
	{
		return keyNum == 0 ? nullptr : keys[0];
	}

	void* LastKey()
	{
		return keyNum == 0 ? nullptr : keys[keyNum - 1];
	}

	void Reheight()
	{
		height = MAX(TTREE_HEIGHT_OF(left), TTREE_HEIGHT_OF(right)) + 1;
	}

	TTreeNode(unsigned int size)
	{
		//多申请一格，插入时有可能挤出来一个key
		keys = (void**)malloc(sizeof(void*) * (size + 1));
		parent = left = right = nullptr;

		keyNum = 0;
		height = 1;
	}
};

class TTreeIterator
{
	public:
		void Reserve(size_t size)
		{
			m_keys.reserve(size);
		}

		void Add(void* pKey)
		{
			m_keys.push_back(pKey);
		}

		void* Get()
		{
			if(IsEOF())
			{
				return nullptr;
			}

			return m_keys[m_current];
		}

		bool IsEOF()
		{
			return m_current == m_keys.size();
		}

		bool Next()
		{
			if(m_current < m_keys.size())
			{
				m_current++;
				return true;
			}

			return false;
		}

		void Reset()
		{
			m_current = 0;
		}

	private:
		std::vector<void*>	m_keys;
		size_t m_current {0};
};

class TTree
{
public:
	TTree(fnKeyComparator fn, bool unique, unsigned int keySize);

	//自上而下插入
	int Insert(void* pKey);

	const void* Query(void* pKey);

	int Delete(void* pKey);

	unsigned int Count();

	void Clear();

	~TTree();

//private:
public:
	unsigned int Count(TTreeNode* pNode);
	/**
	 * 二分查找
	*/
	int BinarySeach(TTreeNode* pNode, const void* pKey, int* insertPos);

	/**
	 * 从前往后找
	*/
	int SearchForward(TTreeNode* pNode, const void* pKey, int* insertPos);

	/**
	 * 从后往前找
	*/
	int SearchBackward(TTreeNode* pNode, const void* pKey, int* insertPos);

	void* Get(const void* pKey);

	int InsertIntoNode(TTreeNode* pNode, void* pKey);

	void InsertIntoLeft(TTreeNode* pNode, void* pKey);

	//取最左边的节点
	TTreeNode* GetLeft(TTreeNode* pNode);

	void Rebalance(TTreeNode* pNode);

	TTreeNode* LeftRotate(TTreeNode* pNode);

	TTreeNode* RightRotate(TTreeNode* pNode);

	void FreeNode(TTreeNode* pNode);
	
//private:
public:
	bool			m_unique;
	fnKeyComparator	m_keyCmp;

	TTreeNode* 		m_pRootNode;

	unsigned int 	m_keySize;
};

#endif

