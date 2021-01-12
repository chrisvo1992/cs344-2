#include <stdio.h>
#include <stdlib.h>
#include "movie_io.h" 

/****DESCRIPTION****/
// - read a csv file as an arg to main
// - create structs to hold each movie
// - create a linked list of these structs
// - give user choices to answer questions about the movie data
// - prints info about movies based on user choices

/*** REFERENCES ***/
// https://en.cppreference.com/w/c/string/byte/strtok
// https://en.cppreference.com/w/c/memory/calloc
// https://linux.die.net/man/3/getline
// mergesort help (JAVA): 
// 	- https://www.youtube.com/watch?v=pNTc1bM1z-4
// 	- https://www.geeksforgeeks.org/c-program-for-merge-sort-for-linked-lists/

/* see header files for function descriptions */

int main(int argc, char *argv[])
{
	int value = 0;
	int movieCount = 0;
	int *mcPtr = &movieCount;
	struct movie *sortedList = 0;
	struct movie *ref = 0;

	if (argc < 2)
	{
		printf("Provide a file to process.\n");
		return 1;
	}

	struct movie *list = processFile(argv[1], mcPtr);
	sortedList = mergeSort(list);
	ref = sortedList;

	printf("\nProcessed file: %s and parsed data for %u movies.\n",
		argv[1], movieCount);

	while (value != 4)
	{
		printMovieMenu();
		value = getMenuChoice();
		printMenuChoices(value, sortedList, movieCount);
	}

	printf("\n");

	sortedList = ref;

	while (sortedList != 0)
	{
		printf(sortedList->title);
		printf("\n");
		ref = sortedList;
		free(ref->title);
		free(ref->languages);
		free(ref);
		sortedList = sortedList->next;
	}
	free(sortedList);

	return 0;
}
