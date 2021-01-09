
#include "movie_algo.h"

void sort(struct movie *list)
{
	unsigned int count = 0;
	unsigned int i = 0;
	struct movie *a = list;
	struct movie *b = 0;

	// count the number of movies in the existing linked list
	while (a != 0)
	{
		a = a->next;
		count++;
	}
	// reset a
	a = list;
	// get the second half of the linked list 
	while (i < count / 2)
	{
		a = a->next;	
		i++;
	}
	b = a;
	// reset a, again
	a = list;

	test(a,b,i);
}

void test(struct movie *left, struct movie *right, int size)
{
	printf("left\n");
	int i = 0;
	while (i < size) 
	{
		printf("\n");
		printf(left->year);
		printf(left->rating);
		printf(left->title);
		printf("\n");
		left = left->next;
		i++;
	}		

	printf("\nright: \n");
	while (right != 0) 
	{
		printf("\n");
		printf(right->year);
		printf(right->rating);
		printf(right->title);
		printf("\n");
		right = right->next;
	}
}

