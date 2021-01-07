#define EF1 "its all wrong"
#include "movie_io.h" 

/****DESCRIPTION****/
// - read a csv file as an arg to main
// - create structs to hold each movie
// - create a linked list of these structs
// - give user choices to answer questions about the movie data
// - prints info about movies based on user choices
//

// Show movies released in a specific year

/*** REFERENCES ***/
// https://en.cppreference.com/w/c/string/byte/strtok
// https://en.cppreference.com/w/c/memory/calloc
// https://linux.die.net/man/3/getline


int main(int argc, char *argv[])
{
	int value = 0;
	if (argc < 2)
	{
		printf("Provide a file to process.");
		return EF1;
	}

	struct movie* list = processFile(argv[1]);
	//printMovieList(list);

	while (value != 4)
	{
		printMovieMenu();
		value = getMenuChoice();
		displayMenuChoice(value, list);
	}

	return 0;
}
