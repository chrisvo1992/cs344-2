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
//

// Show movies released in a specific year

/*** REFERENCES ***/
// https://en.cppreference.com/w/c/string/byte/strtok
// https://en.cppreference.com/w/c/memory/calloc
// https://linux.die.net/man/3/getline


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

	// movie title
	char* token = strtok_r(line, ",", &savePtr);
	currMovie->title = calloc(strlen(token) + 1, sizeof(char));
	printf(currMovie->title);
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
{
	FILE* movieFile = fopen(filePath, "r");

	if (!movieFile)
	{
		printf("oh no\n");
		perror("Failed to open file\n");
		return EXIT_FAILURE;
	}

	size_t len = 0;
	ssize_t read;
	char* currLine = NULL;
	struct movie* head = NULL;
	struct movie* tail = NULL;

	while ((read = getline(&currLine, &len, movieFile)) != -1)
	{
		if (currLine[0] == '\n') 
		{
			continue;
		}
		else 
		{
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
	}
	free(currLine);
	fclose(movieFile);
	return head;
}

void printMovie(struct movie *aMovie)
{
	printf("%s, %s, %s, %s\n",
		aMovie->title,
		aMovie->year,
		aMovie->languages,
		aMovie->rating);
}

void printByTitle(struct movie *aMovie)
{
	printf("%s\n", aMovie->title);
}

void printMovieList(struct movie *list)
{
	while (list != NULL)
	{
		printMovie(list);
		list = list->next;
	}
}

void printMovieMenu()
{
	printf("\n1. Show movies released in the specified year\n");
	printf("2. Show hightest rated movie for each year\n");
	printf("3. Show the title and year of release of all\n");
	printf("   movies in a specified language\n");
	printf("4. Exit from the program\n\n");
}

void showByYear(struct movie *list)
{
	int value = 0;
	int resultCount = 0;
	char *str = malloc(4 * sizeof(char));

	printf("\nEnter the year for which you want to see movies: ");

	scanf("%s", str);

	value = atoi(str);

	while (list != NULL)
	{
		if (atoi(list->year) == value)
		{
			printByTitle(list);
			resultCount++;
		}
		list = list->next;
	}

	if (resultCount == 0)
	{
		printf("No data about movies exists for that year.\n");
	}
	
	free(str);
}

void showByRating()
{
	printf("\nshow by rating\n");
}

void showByLanguage()
{
	printf("\nshow by language\n");
}


int getMenuChoice()
{
	char str[2];
	char *str1 = NULL;
	int choice;
	
	printf("Enter a choice from 1 to 4: ");
	scanf("%s", str);

	str1 = malloc(sizeof(char));
	strcpy(str1, str);

	choice = atoi(str1);

	while (str[1] != '\0' || (choice < 1 || choice > 4))
	{
		
		strcpy(str, "");
		choice = 0;

		printf("Not a valid choice, try again.\n");
		printf("Enter a choice from 1 to 4: ");
		scanf("%s", str);
		strcpy(str1, str);
	
		choice = atoi(str1);	
		
	}

	free(str1);

	return choice;	
}

void printMenuChoice(int val, struct movie *list)
{
	switch(val)
	{
		case 1: showByYear(list); break;
		case 2: showByRating(); break;
		case 3: showByLanguage(); break; 	
		default: break;
	}
}


