#define _GNU_SOURCE
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/****DESCRIPTION****/
// - read a csv file as an arg to main
// - create structs to hold each movie
// - create a linked list of these structs
// - give user choices to answer questions about the movie data
// - prints info about movies based on user choices

struct movie
{
	char* title;
	char* year;
	char* languages;
	char* rating;
	struct movie* next;
};

struct movie* createMovie(char* line)
{
	struct movie *currMovie = malloc(sizeof(struct movie));

	char *savePtr;

	// https://en.cppreference.com/w/c/string/byte/strtok
	// https://en.cppreference.com/w/c/memory/calloc
	// https://linux.die.net/man/3/getline

	// movie title
	char* token = strtok_r(line, ",", &savePtr);
	currMovie->title = calloc(strlen(token) + 1, sizeof(char));
	printf("testing line 35\n");
	printf("%s",currMovie->title);
	strcpy(currMovie->title, token);

	// movie year
	token = strtok_r(NULL, ",", &savePtr);
	currMovie->year = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->year, token);

	// movie languages
	token = strtok_r(NULL, ",", &savePtr);
	currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->languages, token);
	
	// movie rating
	token = strtok_r(NULL, "\n", &savePtr);
	currMovie->rating = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->rating, token);

	currMovie->next = NULL;
	
	return currMovie;
}

struct movie *processFile(const char* filePath)
//struct movie *processFile()
{
	FILE* movieFile = fopen(filePath, "r");
	//FILE* movieFile = fopen("movie_list.txt", "r");

	if (!movieFile)
	{
		printf("oh no\n");
		perror("Failed to open file");
		return EXIT_FAILURE;
	}

	printf("we crazy and we out here");

	size_t len = 0;
	ssize_t read;
	//char* currLine = malloc(sizeof(char));
	//char** currLine = malloc(len * sizeof(char));
	char* currLine = NULL;
	struct movie* head = NULL;
	struct movie* tail = NULL;

    	//int test = 0;
	//test  = fscanf(movieFile, "%s");
	
	while ((read = getline(&currLine, &len, movieFile)) != -1)
	{
		//printf("%s\n", currLine);
		struct movie *newNode = createMovie(currLine);

		if (head == NULL)
		{
			head = newNode;
			tail = newNode;
		}
		else
		{
			tail->next = newNode;
			tail = newNode;
		}
	}
	free(currLine);
	fclose(movieFile);
	return head;
}

void printMovie(struct movie* aMovie)
{
	printf("%s, %s, %s, %s\n",
		aMovie->title,
		aMovie->year,
		aMovie->languages,
		aMovie->rating);
}

void printMovieList(struct movie* list)
{
	while (list != NULL)
	{
		printMovie(list);
		list = list->next;
	}
}

int main(int argc, char *argv[])
{
	printf("movie list started\n");
	if (argc < 2)
	{
		printf("Provide a file to process.");
		return EXIT_FAILURE;
	}

	//struct movie* list = processFile();
	struct movie* list = processFile(argv[1]);
	printMovieList(list);
	
	return 0;
}
