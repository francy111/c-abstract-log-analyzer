#include "DLinkedList.h"
#include <string.h>
#include <stdlib.h>

/**
 * Creates and returns a node that holds the value 'value'
 * The previous and next pointers will be set to NULL
 */
DLinkedList* createNode(char* value) {
	DLinkedList* list = (DLinkedList*)malloc(sizeof(DLinkedList));
	if (list != NULL) {
		list->value = (char*)calloc(strlen(value) + 1, sizeof(char));
		if (list->value != NULL) {
			int i;
			for (i = 0; i < strlen(value); i++)
				list->value[i] = value[i];
			list->value[i] = '\0';

		}
		list->prev = (DLinkedList*)NULL;
		list->next = (DLinkedList*)NULL;
	}
	return list;
}

/**
 * Inserts the new element 'value' as the first element
 * of the list
 */
int insertHead(DLinkedList** listHead, char* value) {

	/* 0 -> Inserted successfully
	 * 1 -> Insertion failed
	 */
	int r = 1;

	// Create new node
	DLinkedList* newNode = createNode(value);
	if (newNode != NULL) {

		// Set the new node as the first one
		newNode->next = (*listHead);

		// If this isn't an empty list, we need to update the old head's previous field
		if ((*listHead) != NULL) {
			(*listHead)->prev = newNode;
		}

		// Update the pointer to the list, reflecting the update
		(*listHead) = newNode;

		r = 0;
	}
	return r;
}

/**
 * Inserts the new element 'value' as the last element
 * of the list
 */
int insertTail(DLinkedList** listHead, char* value) {

	/* 0 -> Inserted successfully
	 * 1 -> Insertion failed
	 */
	int r = 1;

	// Create new node
	DLinkedList* newNode = createNode(value);
	if (newNode != NULL) {

		// If the list is not empty we iterate until the last element
		if ((*listHead) != NULL) {

			// Iterate until the last element, next is NULL on the last element
			DLinkedList* tail = (*listHead);
			while (tail->next != NULL) {
				tail = tail->next;
			}

			// Now we have the tail of the list
			tail->next = newNode;
			newNode->prev = tail;
		}

		// Otherwise, the list has no element so it's the same as appending at the head
		else {
			(*listHead) = newNode;
		}

		r = 0;
	}
	return r;
}

/**
 * Tries to insert the new element 'value' as  the element
 * number 'index' (starting from zero)
 */
int insertAt(DLinkedList** listHead, char* value, int index) {

	/* 0 -> Inserted successfully
	 * 1 -> Insertion failed
	 */
	int r = 1;

	// Only if the index 'makes sense'
	if (index >= 0) {

		// Create new node
		DLinkedList* newNode = createNode(value);
		if (newNode != NULL) {

			// If the list is not empty, we can try to iterate to the index-th element
			if ((*listHead) != NULL) {

				// Try to iterate until the index-th element (index == i), or until the last (index is overnext is NULL on the last element
				DLinkedList* tmp = (*listHead);
				int i = 0;
				while (tmp->next != NULL && i < index) {
					tmp = tmp->next;
					i++;
				}

				// Only if there are enough elements for the specified index
				if (i == index) {

					/* Insert new node between the current element(i - th, becoming i + 1 - th) and it's previous (i-1 -th)
					 * Connect the i-1 -th element with the new node (i -th)
					 * Then connect the new node (i -th) with the current one (i+1 -th)
					 */
					newNode->prev = tmp->prev;
					if (index > 0) newNode->prev->next = newNode;

					tmp->prev = newNode;
					newNode->next = tmp;

					r = 0;
				}

				// If the index is over the size of the list, by just 1, is the same as adding at the tail
				else if (i + 1 == index) {

					// Inserting after the last element
					newNode->prev = tmp;
					tmp->next = newNode;
				}
			}

			// Index 0 is the same as inserting at the head
			if (index == 0) {
				(*listHead) = newNode;
			}
		}
	}
	return r;
}

/**
 * Removes the first element of the list
 */
void removeHead(DLinkedList** listHead) {

	// Remove only if the list isn't empty, otherwise, no operation is needed
	if ((*listHead) != NULL) {

		// Shift the head to the second element of the list
		DLinkedList* head = (*listHead);
		(*listHead) = (*listHead)->next; // Logical deletion
		(*listHead)->prev = NULL;

		deleteNode(&head); // Phyisical deletion
	}
	return;
}

/**
 * Removes the last element of the list
 */
void removeTail(DLinkedList** listHead) {

	// Remove only if the list isn't empty, otherwise, no operation is needed
	if ((*listHead) != NULL) {

		// Iterate until the last element (tail) of the list
		DLinkedList* tail = (*listHead); 
		while (tail->next != NULL) {
			tail = tail->next;
		}

		// If the tail is also the head
		if (tail->prev == NULL) {

			// Logical deletion of the only element of the list
			(*listHead) = NULL;
		} 
		else {

			// Logical deletion, set the second-last element of the list to point to NULL as next node
			tail->prev->next = NULL;
		} 

		// Physical deletion of the tail
		deleteNode(&tail);
	}
	return;
}

/**
 * Tries to remove the element at position 'index' (starting from zero)
 */
void removeAt(DLinkedList** listHead, int index) {

	// Only if the index 'makes sense'
	if (index >= 0) {

		// If the list isn't empty, otherwise no operation is needed
		if ((*listHead) != NULL) {

			// Try to iterate until the index-th element (index == i), or until the last (index is overnext is NULL on the last element
			DLinkedList* tmp = (*listHead);
			int i = 0;
			while (tmp->next != NULL && i < index) {
				tmp = tmp->next;
				i++;
			}

			// If there were enough elements
			if (i == index) {

				// The index-th element is the head (0 -th)
				if (tmp->prev == NULL) {

					// Logical deletion of the head
					(*listHead) = tmp->next;
				}

				// The index-th element is the tail (size-1 -th)
				else if (tmp->next == NULL) {

					// Logical deletion of the tail
					tmp->prev->next = NULL;
				}

				// Every other case index in (0, size - 1) <- open set
				else {

					// Logical deletion of the node (i -th), we link the previous (i-1 -th) element with the next (i+1 -th) 
					tmp->prev->next = tmp->next;
					tmp->next->prev = tmp->prev;
				}
				
				// Physical deletion of the node
				deleteNode(&tmp);
			}
		}
	}
	return;
}

/**
 * Returns the value stored in the first node of the list
 */
char* getItemHead(DLinkedList* listHead) {

	// Default value is NULL
	char* res = NULL;

	// Extract a value only if the list isn't empty
	if (listHead != NULL) {
		res = listHead->value;
	}
	return res;
}

/**
 * Returns the value stored in the last node of the list
 */
char* getItemTail(DLinkedList* listHead) {

	// Default value is NULL
	char* res = NULL;

	// Extract a value only if the list isn't empty
	if (listHead != NULL) {

		// Iterate to the tail of the list
		DLinkedList* tmp = listHead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		res = tmp->value;
	}
	return res;
}

/**
 * Tries to return the value stored in the node number 'index'
 */
char* getItemAt(DLinkedList* listHead, int index) {

	// Default value is NULL
	char* res = NULL;

	// Only if the index 'makes sense'
	if (index >= 0) {

		// Extract a value only if the list isn't empty
		if (listHead != NULL) {

			// Try to iterate until the index-th element (index == i), or until the last (index is overnext is NULL on the last element
			DLinkedList* tmp = listHead;
			int i = 0;
			while (tmp->next != NULL && i < index) {
				tmp = tmp->next;
				i++;
			}

			// If there were enough elements (we found the node we wanted)
			if (i == index) {
				res = tmp->value;
			}
		}
	}
	return res;
}

/**
 * Returns the number of elements in the list
 */
int size(DLinkedList* listHead) {

	// Iterate through the list and count the elements
	DLinkedList* tmp = listHead;
	int i = 0;
	while (tmp != NULL) {
		tmp = tmp->next;
		i++;
	}
	return i;
}

/**
 * Reverses the list
 */
void reverse(DLinkedList** listHead) {

	// Check if a valid pointer was passed
	if (listHead != NULL) {

		// Proceed unless the list is empty (no operation needed)
		DLinkedList* current = (*listHead);
		if (current != NULL) {

			// Iterate through the list, swapping 'prev' and 'next' pointers
			DLinkedList* t;
			while (current->next != NULL) {
				t = current->next;
				current->next = current->prev;
				current->prev = t;
				current = current->prev;
			}
			current->next = current->prev;
			current->prev = NULL;

			// New head is the old tail of the list
			(*listHead) = current;
		}
	}
}

/**
 * Check wheter the list contains the value or not
 * Returns the position the element is (from 1 to size istaed of 0 to size - 1)
 */
int contains(DLinkedList* listHead, char* value) {

	// Contained flags the presence of the element, pos is the position (1 to sizeof list)
	int contained = 0;
	int pos = 0;

	// Iterate the list
	DLinkedList* tmp = listHead;
	while (tmp != NULL) {
		
		// Update position every time
		pos++;

		// If the value is the one we wanted we exit the loop
		if (strncmp(tmp->value, value, strlen(tmp->value)) == 0) {
			contained = 1;
			break;
		} 
		tmp = tmp->next;
	}

	// Return the position only if it is present, 0 otherwise
	return contained ? pos : 0;
}

/**
 * Writes the entire list in the specified order into a string
 * Order >= 0 -> from the head to the tail
 * Order < 0 -> from the tail to the head
 * 
 * The list is created in the heap and must be deallocated after!!!
 */
char* listToString(DLinkedList* listHead, int order) {

	// We first need to find how long is the list (as in how many characters does it contain summed up)
	int charCount = 0;
	int count = 0;
	DLinkedList* tmp = listHead;
	while (tmp != NULL) {
		charCount += (int)strlen(tmp->value);
		tmp = tmp->next;
		count++;
	}

	// Add [, ], and ", " for every node (minus 1) so 1 + 1 + 2 * (count - 1) = 2 + 2(Coun t- 1) = 2 * count
	int stringLen = (count > 0 ? charCount + 2 * count : 2) + 1;
	char* string = (char*)calloc(stringLen, sizeof(char));

	// Continue only if it was correctly allocated
	if (string != NULL) {
		memset(string, '\0', stringLen);

		// The list is formatted head to tail when a number greater or equal to zero is passed
		if (order >= 0) {

			// First character of the list is '[' (charCount is 0, also updates it to 1)
			string[charCount++] = '[';

			// Iterate through the list
			tmp = listHead;
			int i;
			while (tmp != NULL)
			{

				// Copy the string stored in every node one character at a time
				for (i = 0; i < strlen(tmp->value); i++) {
					string[charCount + i] = tmp->value[i];
				}
				charCount += i;

				// Include ", " (only for formatting purposes) for every element except the tail
				if (tmp->next != NULL) {
					string[charCount++] = ',';
					string[charCount++] = ' ';
				}
				tmp = tmp->next;
			}

			// Last character of the list is ']'
			string[charCount++] = ']';
		}

		// On the other hand, when a number less than zero is passed, the list is formatted tail to head
		else {

			// First we need to get to the tail of the list
			tmp = listHead;
			if (tmp != NULL) {
				while (tmp->next != NULL) {
					tmp = tmp->next;
				}
			}

			// First character of the list is '[' (charCount is 0, also updates it to 1)
			string[charCount++] = '[';

			// Iterate from the tail to the head (tmp now points to the tail)
			int i;
			while (tmp != NULL)
			{

				// Copy the string stored in every node one character at a time
				for (i = 0; i < strlen(tmp->value); i++) {
					string[charCount + i] = tmp->value[i];
				}
				charCount += i;

				// Include ", " (only for formatting purposes) for every element except the head
				if (tmp->prev != NULL) {
					string[charCount++] = ',';
					string[charCount++] = ' ';
				}
				tmp = tmp->prev;
			}

			// Last character of the list is ']'
			string[charCount++] = ']';
		}
	}
	return string;
}

/**
 * Deletes the specified node, freeing
 * the used memory
 */
void deleteNode(DLinkedList** node) {

	// We first free the memory allocated for the string (we allocated it when creating the node)
	free((*node)->value);

	// Then we deallocate the memory used for the whole node struct (Physical deletion)
	free((*node));

	// Set the value pointed by node to be NULL (Logical deletion)
	(*node) = NULL;
}

/**
 * Deletes the entire list starting from the passed
 * element, deallocating the used memory
 */
void deleteList(DLinkedList** listHead) {

	// Only proceed if the list isn't null, otherwise no operation is needed
	if ((*listHead != NULL)) {

		// Iterate through the list
		DLinkedList* tmp = (*listHead);
		while (tmp->next != NULL) {

			// First we update tmp to get the next node, otherwise we would lose the pointer
			tmp = tmp->next;

			// Then we can delete, both logically and physically, the current node (previous of the next)
			deleteNode(&(tmp->prev));
		}

		// We exit the loop on the tail, this is the last element to delete
		deleteNode(&tmp);

		// Also logically delete the list, making sure the pointer points to NULL
		(*listHead) = NULL;
	}
}