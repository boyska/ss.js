struct listNode {  /* self-referential structure */
	char *data;
	struct listNode *nextPtr;
};
typedef struct listNode LISTNODE;
typedef LISTNODE *LISTNODEPTR;


void stringlist_insert(LISTNODEPTR *sPtr, char *value);
