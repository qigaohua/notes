#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>


struct node {
	int v;
	struct node *next;
};

/*非递归逆转*/
int reverse(struct node **head) 
{
	if(!head || !(*head)) return -1;
	struct node *pre, *cur, *nex;

	pre = NULL, cur=*head;
	while(cur) {
		nex = cur->next;
		cur->next = pre;
		pre = cur;
		cur = nex;
	}
	*head = pre;

	return 0;
}

/*递归逆转*/
struct node* reverse2(struct node* head)
{
	struct node *p1, *p2;

	if(!head || !(head->next)) return head;
	p1 = head;
	p2 = p1->next;

	head = reverse2(p2);

	p2->next = p1;
	p1->next = NULL;

	return head;
}

int printfList(struct node* head)
{
	struct node *p;

	p = head;

	while(p) {
		printf("%d\t", p->v);
		p = p->next;
	}
	printf("\n");	

	return 0;
}


#if 1

/*test*/
int main()
{
	int i;
	struct node *head = NULL;
	struct node *p;


	for(i = 0; i < 16; i++) {
		if (head) {
			p = (struct node*) malloc(sizeof(*p));
			p->v = i;
			p->next = head;
			head = p;
		} else { 
			head = (struct node*) malloc(sizeof(*head));
			head->next = NULL;
			head->v = i;
		}
	}
	printfList(head);

	reverse(&head);
	printfList(head);

	head = reverse2(head);
	printfList(head);

	return 0;
}


#endif


