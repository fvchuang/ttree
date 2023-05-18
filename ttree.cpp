/**
 * @brief	T树实现
 * @author	huangxx
*/

#include "ttree.h"


TTree::TTree(fnKeyComparator fn, bool unique, unsigned int keySize)
{
	m_keyCmp = fn;
	m_unique = unique;
	m_keySize = keySize;

	m_pRootNode = nullptr;
}

void TTree::Clear()
{
	if (m_pRootNode)
	{
		FreeNode(m_pRootNode);
		delete m_pRootNode;
		m_pRootNode = nullptr;
	}
}

TTree::~TTree()
{
	Clear();
}

void TTree::FreeNode(TTreeNode* pNode)
{
	if (pNode->left)
	{
		FreeNode(pNode->left);
		delete pNode->left;
	}

	if (pNode->right)
	{
		FreeNode(pNode->right);
		delete pNode->right;
	}

	free(pNode->keys);
}


int TTree::SearchForward(TTreeNode* pNode, const void* pKey, int* insertPos)
{
	int found = -1;
	int cmp;

	*insertPos = pNode->keyNum;	//假设在最后面，若for循环了一遍都没有被赋值，假设成立

	for(int i = 0; i < pNode->keyNum; i++)
	{
		cmp = m_keyCmp(pNode->keys[i], pKey);
		if (cmp == 0)
		{
			found = i;
			*insertPos = i + 1;
			break;
		}
		else if (cmp > 0)
		{
			*insertPos = i;
			break;
		}
	}

	return found;
}


int TTree::SearchBackward(TTreeNode* pNode, const void* pKey, int* insertPos)
{
	int found = -1;
	int cmp;

	*insertPos = 0;

	for(int i = pNode->keyNum; i > 0; i--)
	{
		cmp = m_keyCmp(pKey, pNode->keys[i - 1]);
		if (cmp == 0)
		{
			found = i - 1;
			*insertPos = i;
			break;
		}
		else if (cmp > 0)
		{
			*insertPos = i;
			break;
		}
	}

	return found;
}

int TTree::BinarySeach(TTreeNode* pNode, const void* pKey, int* insertPos)
{
	int left = 0, right = pNode->keyNum, m, cmp;

	while (left < right)
	{
		m = (left + right) / 2;

		cmp = m_keyCmp(pKey, pNode->keys[m]);

		if (cmp == 0)
		{
			*insertPos = m + 1;
			return m;
		}
		else if (cmp < 0)
		{
			right = m;
		}
		else
		{
			left = m + 1;
		}
	}

	if(cmp > 0)
		*insertPos = m + 1;
	else
		*insertPos = m;

	return -1;
}

void* TTree::Get(const void* pKey)
{
	TTreeNode* pNode = m_pRootNode;
	int index, insertPos;
	void* pTarget = nullptr;

	int cmpLeft, cmpRight;

	while (pNode)
	{
		cmpLeft = m_keyCmp(pKey, pNode->FirstKey());
		if (cmpLeft < 0)
		{
			pNode = pNode->left;
			continue;
		}
		else if ((cmpRight = m_keyCmp(pKey, pNode->LastKey())) > 0)
		{
			pNode = pNode->right;
			continue;
		}
		else
		{
			index = BinarySeach(pNode, pKey, &insertPos);
			if (index > 0)
				pTarget = pNode->keys[index];

			break;
		}
	}

	return pTarget;
}


int TTree::InsertIntoNode(TTreeNode* pNode, void* pKey)
{
	int insertPos, foundIndex;
	foundIndex = SearchBackward(pNode, pKey, &insertPos);
	//foundIndex = BinarySeach(pNode, pKey, &insertPos);

	if (m_unique && foundIndex >= 0)
	{
		return -1;
	}
	
	//往后挪
	for (int i = pNode->keyNum; i > insertPos; i--)
	{
		pNode->keys[i] = pNode->keys[i - 1];
	}
	pNode->keys[insertPos] = pKey;

	//插入前就挤满了格子，那么会多出来的一格，多出来的往右子树的最左边插
	if (pNode->keyNum >= m_keySize)
	{
		void* pOverflow =  pNode->keys[m_keySize];
		if (pNode->right)
		{
			TTreeNode* pMostLeft = GetLeft(pNode->right);
			//满了
			if(pMostLeft->keyNum >= m_keySize)
			{
				TTreeNode* pNewNode = new TTreeNode(m_keySize);
				pNewNode->parent = pMostLeft;
				pNewNode->keys[0] = pOverflow;
				pNewNode->keyNum++;

				pMostLeft->left = pNewNode;
				Rebalance(pMostLeft);
			}
			else
			{
				for(int i = pMostLeft->keyNum; i > 0; i--)
				{
					pMostLeft->keys[i] = pMostLeft->keys[i - 1];
				}
				pMostLeft->keys[0] = pOverflow;
				pMostLeft->keyNum++;
			}
		}
		else
		{
			TTreeNode* pNewNode = new TTreeNode(m_keySize);
			pNewNode->parent = pNode;
			pNode->right = pNewNode;

			pNewNode->keys[0] = pOverflow;
			pNewNode->keyNum++;

			Rebalance(pNode);
		}
	}
	else
	{
		pNode->keyNum++;
	}

	return 0;
}

//取最左边的节点
TTreeNode* TTree::GetLeft(TTreeNode* pNode)
{
	while (pNode->left)
	{
		pNode = pNode->left;
	}

	return pNode;
}

/**
 * RR型，直接左旋
 * 	P
 * 		R
 * 
 * 			R
 * 
 * RL型，先右旋，转化成RR型，再左旋
 * P
 * 		R
 * L
 * 
 * LL型与RR型类似，LR型与RL型类似
 * 
*/

void TTree::Rebalance(TTreeNode* pNode)
{
	int diff;	//左右子树高度差

	do
	{
		pNode->Reheight();

		diff = TTREE_HEIGHT_OF(pNode->left) - TTREE_HEIGHT_OF(pNode->right);

		//右子树过高
		if (diff < -1)
		{
			int subDiff = TTREE_HEIGHT_OF(pNode->right->left) - TTREE_HEIGHT_OF(pNode->right->right);
			//RL型
			if (subDiff > 0)
			{
				pNode = RightRotate(pNode->right->left);
				pNode = LeftRotate(pNode);
			}
			//RR型
			else
			{
				pNode = LeftRotate(pNode->right);
			}
		}
		//左子树过高
		else if (diff > 1)
		{
			int subDiff = TTREE_HEIGHT_OF(pNode->left->left) - TTREE_HEIGHT_OF(pNode->left->right);
			//LR型
			if (subDiff < 0)
			{
				pNode = LeftRotate(pNode->left->right);
				pNode = RightRotate(pNode);
			}
			else
			{
				pNode = RightRotate(pNode->left);
			}
		}
		
		if(pNode->parent == nullptr)
		{
			m_pRootNode = pNode;
		}

		pNode = pNode->parent;
	}
	while (pNode);

}


int TTree::Insert(void* pKey)
{
	if (m_pRootNode == nullptr)
	{
		m_pRootNode = new TTreeNode(m_keySize);
		m_pRootNode->keys[0] = pKey;
		m_pRootNode->keyNum++;
		return 0;
	}
	
	TTreeNode* pNode = m_pRootNode;

	int cmpLeft, cmpRight;

	while (true)
	{
		cmpLeft = m_keyCmp(pKey, pNode->FirstKey());

		// key < left
		if (cmpLeft < 0)
		{
			if (pNode->left)
			{
				pNode = pNode->left;
				continue;
			}

			//这个结点还有空间
			else if(pNode->keyNum < m_keySize)
			{
				return InsertIntoNode(pNode, pKey);
			}
			else	// key 可能会下沉？
			{
				TTreeNode* pNewNode = new TTreeNode(m_keySize);
				pNewNode->parent = pNode;
				pNewNode->keys[0] = pKey;
				pNewNode->keyNum++;

				pNode->left = pNewNode;
				Rebalance(pNode);

				return 0;
			}
		}

		cmpRight = m_keyCmp(pKey, pNode->LastKey());
		// key > right
		if (cmpRight > 0)
		{
			if (pNode->right)
			{
				pNode = pNode->right;
				continue;
			}	

			//这个结点还有空间
			if(pNode->keyNum < m_keySize)
			{
				return InsertIntoNode(pNode, pKey);
			}

			else
			{
				TTreeNode* pNewNode = new TTreeNode(m_keySize);
				pNewNode->parent = pNode;
				pNewNode->keys[0] = pKey;
				pNewNode->keyNum++;

				pNode->right = pNewNode;
				Rebalance(pNode);
				return 0;
			}
		}
		
		// left <= key <= right , key应当在这个node
		return InsertIntoNode(pNode, pKey);
	}

	//不会到这里
	return -1;
}

const void* TTree::Query(void* pKey)
{
	TTreeNode* pNode = m_pRootNode;

	int cmpLeft, cmpRight, index, pos;

	while (pNode)
	{
		cmpLeft = m_keyCmp(pKey, pNode->FirstKey());
		if(cmpLeft < 0)
		{
			pNode = pNode->left;
			continue;
		}
		
		cmpRight = m_keyCmp(pKey, pNode->LastKey());
		if(cmpRight > 0)
		{
			pNode = pNode->right;
			continue;
		}

		index = SearchBackward(pNode, pKey, &pos);

		if(index >= 0)
		{
			return pNode->keys[index];
		}
		else
		{
			return nullptr;
		}
	}

	return nullptr;
}

int TTree::Delete(void* pKey)
{

	return 0;
}

unsigned int TTree::Count()
{
	return m_pRootNode == nullptr ? 0 : Count(m_pRootNode);
}

unsigned int TTree::Count(TTreeNode* pNode)
{
	return pNode->keyNum + \
			(pNode->left == nullptr ? 0 : Count(pNode->left)) + 
			(pNode->right == nullptr ? 0 : Count(pNode->right));
}
/** 左旋，右子树的树高转移到左子树，
 * 
 *			pParent 		
 * 				pNode
 * 		pLeft		pRight
 * 					
*/

TTreeNode* TTree::LeftRotate(TTreeNode* pNode)
{
	TTreeNode* pParent = pNode->parent;		//parent 肯定存在
	TTreeNode* pLeft = pNode->left;

	//如果存在祖父节点，祖父的孙子(即pNode)变成儿子
	if (pParent->parent)
	{
		if (pParent == pParent->parent->left)
		{
			pParent->parent->left = pNode;
		}
		else
		{
			pParent->parent->right = pNode;
		}
	}

	pNode->parent = pParent->parent;
	pNode->left = pParent;
	if(pLeft)
	{
		pLeft->parent = pParent;
	}


	pParent->parent = pNode;
	pParent->right = pLeft;

	//pParent和pNode的父子关系互换了，需要子节点先reheight
	pParent->Reheight();
	pNode->Reheight();

	return pNode;
}

TTreeNode* TTree::RightRotate(TTreeNode* pNode)
{
	TTreeNode* pParent = pNode->parent;
	TTreeNode* pRight = pNode->right;

	if (pParent->parent)
	{
		if (pParent == pParent->parent->left)
		{
			pParent->parent->left = pNode;
		}
		else
		{
			pParent->parent->right = pNode;
		}
	}

	pNode->parent = pParent->parent;
	pNode->right = pParent;
	if (pRight)
	{
		pRight->parent = pParent;
	}

	pParent->parent = pNode;
	pParent->left = pRight;

	pParent->Reheight();
	pNode->Reheight();

	return pNode;
}