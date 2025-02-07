#ifndef DLINKEDLIST_H
#define DLINKEDLIST_H

#define MAX_LEN 512
#define HEAD_TO_TAIL 1
#define TAIL_TO_HEAD -1

/**
 * Double Linked List
 * 
 * A linked list of nodes (containing strings of characters), with possibility to operate in both sides
 *
 * Fields:
 *	Value -> Value hold by this node
 *	Prev -> Pointer to the previous node
 *	Next -> Pointer to the next node
 */
typedef struct DLinkedList{
	char* value;
	struct DLinkedList* prev;
	struct DLinkedList* next;
} DLinkedList;

/**
 * Creates and returns a node that holds the value 'value'
 * The previous and next pointers will be set to NULL
 */
DLinkedList* createNode(char* value);

/**
 * Inserts the new element 'value' as the first element
 * of the list
 */
int insertHead(DLinkedList** listHead, char* value);

/**
 * Inserts the new element 'value' as the last element
 * of the list
 */
int insertTail(DLinkedList** listHead, char* value);

/**
 * Tries to insert the new element 'value' as  the element
 * number 'index' (starting from zero)
 */
int insertAt(DLinkedList** listHead, char* value, int index);

/**
 * Removes the first element of the list
 */
void removeHead(DLinkedList** listHead);

/**
 * Removes the last element of the list
 */
void removeTail(DLinkedList** listHead);

/**
 * Tries to remove the element at position 'index' (starting from zero)
 */
void removeAt(DLinkedList** listHead, int index);

/**
 * Returns the value stored in the first node of the list
 */
char* getItemHead(DLinkedList* listHead);

/**
 * Returns the value stored in the last node of the list
 */
char* getItemTail(DLinkedList* listHead);

/**
 * Tries to return the value stored in the node number 'index'
 */
char* getItemAt(DLinkedList* listHead, int index);

/**
 * Returns the number of elements in the list
 */
int size(DLinkedList* listHead);

/**
 * Reverses the list
 */
void reverse(DLinkedList** listHead);

/**
 * Check wheter the list contains the value or not
 * Returns the position the element is (from 1 to size instead of 0 to size - 1)
 */
int contains(DLinkedList* listHead, char* value);

/**
 * Writes the entire list in the specified order into a string
 * Order >= 0 -> from the head to the tail
 * Order < 0 -> from the tail to the head
 *
 * The list is created in the heap and must be deallocated after!!!
 */
char* listToString(DLinkedList* listHead, int order);

/**
 * Deletes the specified node, freeing
 * the used memory
 */
void deleteNode(DLinkedList** node);

/**
 * Deletes the entire list starting from the passed
 * element, deallocating the used memory
 */
void deleteList(DLinkedList** listHead);

#endif