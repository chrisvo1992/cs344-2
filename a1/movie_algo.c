
#include "movie_algo.h"

void sort(struct movie *list)
{
	unsigned int totalCount = 0;
	unsigned int countA = 0;
	unsigned int countB = 0;
	struct movie *listA = list;
	struct movie *listB = 0;

	// count the number of movies in the existing linked list
	while (listA != 0)
	{
		listA = listA->next;
		totalCount++;
	}
	// reset a
	listA = list;
	// get the second half of the linked list 
	while (countA < totalCount / 2)
	{
		listA = listA->next;	
		countA++;
	}
	countB = totalCount - countA;
	listB = listA;
	// reset a, again
	listA = list;

	test(listA,listB,countA, countB);
	// list = the whole list
	// listA = first half of linked list
	// listB = second half of linked list
	// NOTE - listB will always have a greater count when list size
	// 	is odd.
	//mergeSort(list, listA, listB); 
	
}

void mergeSort(struct movie* list, struct movie* a, struct movie* b, unsigned int sizeA, unsigned int sizeB)
{
	/*if (sizeA < sizeB) 
	{
	}
	*/	
}

void test(struct movie *left, struct movie *right, int sizeA, int sizeB)
{
	printf("left\n");
	int i = 0;
	while (i < sizeA) 
	{
		++i;
		printf("%i\n", i);
		printf(left->year);
		printf(left->rating);
		printf(left->title);
		printf("\n");
		left = left->next;
	}		
	i = 0;
	printf("\nright: \n");
	while (i < sizeB) 
	{
		++i;
		printf("%i\n", i);
		printf(right->year);
		printf(right->rating);
		printf(right->title);
		printf("\n");
		right = right->next;
	}
}

