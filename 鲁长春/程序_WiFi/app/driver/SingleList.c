
#include "SingleList.h"
#include "malloc.h"


// 创建链表
SingleList* NewSingleList(void)
{
	 SingleList* sl = (SingleList*)malloc(sizeof(SingleList));
	 sl->next = 0;
	 sl->num = 0;
	 return sl;
}

// 链表尾部插入节点,返回新插入的节点地址
unsigned int SingleList_InsertEnd(SingleList* list, SingleListNode* node) 
{ 
	unsigned int i = 0;
    SingleList* current = list;

    SingleListNodeStr* newnode = (SingleListNodeStr*)malloc(sizeof(SingleListNodeStr));
	newnode->next = 0;
	newnode->node = node;

    for(i=0; current->next != 0; i++)
    {
        current = current->next;
    }
	current->next = newnode;     
    list->num ++;
    return (unsigned int)newnode;
}

//返回移除节点的上一个节点
SingleListNode*  SingleList_DeleteNode(SingleList* list, SingleListNode* node) 
{

    SingleList* current = list;
	void* back;
    for(;current->next != 0; )
    {
        if( ((SingleListNodeStr*)(current->next))->node == node )	break; 
        current = current->next;
    }  
	back =  current->next;
    current->next = ((SingleListNodeStr*)(current->next))->next;
	free(back);
	list->num --;
    return (SingleListNode*)current;
}

//迭代器
//如果有下一个则返回下一个指针，否则返回 0
SingleListNode* SingleList_Iterator(SingleListNode** node)
{
	if(*node) *node = ((SingleListNodeStr*)*node)->next;
	return ((SingleListNodeStr*)*node);

}

// 释放链表
void FreeSingList(SingleList* list)
{

	SingleListNode* node = (SingleListNode*)list;
	while(SingleList_Iterator(&node))
	{
		 SingleList_DeleteNode(list,node);
	}
	free(list);
}

