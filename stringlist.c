#include <stdlib.h>
#include "stringlist.h"

void stringlist_insert(LISTNODEPTR *sPtr, char *value)
{
	LISTNODEPTR newPtr, previousPtr, currentPtr;

	newPtr = malloc(sizeof(LISTNODE));


	if (newPtr != NULL) {    /* is space available */
		newPtr->data = malloc(sizeof(value));
		newPtr->data = value;
		newPtr->nextPtr = NULL;

		previousPtr = NULL;
		currentPtr = *sPtr;

		while (currentPtr != NULL && value > currentPtr->data) {
			previousPtr = currentPtr;          /* walk to ...   */
			currentPtr = currentPtr->nextPtr;  /* ... next node */
		}

		if (previousPtr == NULL) {
			newPtr->nextPtr = *sPtr;
			*sPtr = newPtr;
		}
		else {
			previousPtr->nextPtr = newPtr;
			newPtr->nextPtr = currentPtr;
		}
	}
	else
		exit(1);
}

