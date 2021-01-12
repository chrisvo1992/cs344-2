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
	int *movieCount = 0;
	struct movie *sortedList = 0;
	struct movie *ref = 0;

	if (argc < 2)
	{
		printf("Provide a file to process.\n");
		return 1;
	}

	struct movie *list = processFile(argv[1], &movieCount);
	sortedList = mergeSort(list);
	//ref = sortedList;

	while (value != 4)
	{
		printMovieMenu();
		value = getMenuChoice();
		printMenuChoices(value, sortedList, movieCount);
	}

	printf("\n");

	//sortedList = ref->next;

	while (sortedList != 0)
	{
		printf(sortedList->title);
		printf("\n");
		ref = sortedList->next;
		free(sortedList->title);
		free(sortedList->languages);
		free(sortedList);
		sortedList = ref;
	}
	free(sortedList);

	return 0;
}
