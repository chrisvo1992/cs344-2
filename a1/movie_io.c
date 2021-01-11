#include <stdlib.h>
#include <string.h>
#include <stdio.h>
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
//

/* see corresponding header file for descriptions */

struct movie* _createMovie(char* line)
{
	struct movie *currMovie = malloc(sizeof(struct movie));

	char *savePtr;

	// movie title
	char* token = strtok_r(line, ",", &savePtr);
	//currMovie->title = token;
	currMovie->title = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->title, token);
	free(currMovie->title);

	// movie year
	token = strtok_r(NULL, ",", &savePtr);
	currMovie->year = atoi(token);

	// movie languages
	token = strtok_r(NULL, ",", &savePtr);
	currMovie->languages = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->languages, token);
	free(currMovie->languages);
	
	// movie rating
	token = strtok_r(NULL, "\n", &savePtr);
	currMovie->rating = atof(token);

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
		exit(EXIT_FAILURE);
	}

	size_t len = 0;
	ssize_t read;
	char* currLine = NULL;
	struct movie* head = NULL;
	struct movie* tail = NULL;

	read = getline(&currLine, &len, movieFile);

	while ((read = getline(&currLine, &len, movieFile)) != -1)
	{
		if (currLine[0] == '\n') 
		{
			continue;
		}
		else 
		{
			
			struct movie *newNode = _createMovie(currLine);

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

int findStr(char const *str, char const *subStr)
{
	char *pos = strstr(str, subStr);
	if (pos)
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}

void _printMovie(struct movie *aMovie)
{
	printf("%s, %d, %s, %f\n",
		aMovie->title,
		aMovie->year,
		aMovie->languages,
		aMovie->rating);
}

void _printByTitle(struct movie *aMovie)
{
	printf("%s\n", aMovie->title);
}

void _printMovieList(struct movie *list)
{
	while (list != NULL)
	{
		_printMovie(list);
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

void _showByYear(struct movie *list)
{
	int value = 0;
	int resultCount = 0;
	int input;	

	printf("\nEnter the year for which you want to see movies: ");

	scanf("%d", &input);

	value = input;

	while (list != NULL)
	{
		if (list->year == value)
		{
			_printByTitle(list);
			resultCount++;
		}
		list = list->next;
	}

	if (resultCount == 0)
	{
		printf("No data about movies exists for that year.\n");
	}
}

void _showByRating(struct movie *list)
{
	// read the unique years and create a list of those years,
	// assigning the first value to each year node.
	// Note: the list is sorted in asc order. 
	struct movie *newNode = 0;
	struct movie *tempYear = 0;
	struct movie *uniqueYearList = 0; 
	struct movie *uniqueListRef = 0;
	struct movie *headRef = list;

	newNode = malloc(sizeof(struct movie));
	newNode->title = list->title;
	strcpy(newNode->title, list->title);
	newNode->year = list->year;
	newNode->languages = list->languages;
	strcpy(newNode->languages, list->languages);
	newNode->rating = list->rating;
	newNode->next = 0;
	uniqueYearList = newNode;
	uniqueListRef = uniqueYearList;

	while (list != 0)
	{
		if (list->year == newNode->year)
		{
			list = list->next;
		}
		else
		{
			tempYear = newNode;		
			newNode = malloc(sizeof(struct movie));
			tempYear->next = newNode;
			newNode->title = list->title;
			newNode->year = list->year;
			newNode->languages = list->languages;
			newNode->rating = list->rating;
			newNode->next = 0;
		}
		list = list->next;
	}
	tempYear->next = newNode;
	newNode->next = 0;
	list = headRef;
	
	//
	// while checking the year and change of year, compare the rating
	// of each year in list with the rating of each unique year.
	//
	// if the list rating is hight, swap values.
	//
	
	while (uniqueYearList != 0)
	{
		// while the years are the same, 
		// compare the ratings.
		while (list->year == uniqueYearList->year)
		{
			if (list->rating > uniqueYearList->rating)
			{
				uniqueYearList->title = list->title;
				uniqueYearList->year = list->year;			
				uniqueYearList->languages = list->languages;
				uniqueYearList->rating = list->rating;
			}
			list = list->next;
		}

		uniqueYearList = uniqueYearList->next;
	}
	
	uniqueYearList = uniqueListRef;

	while (uniqueYearList != 0) 
	{
		printf("%d %0.1f %s\n", 
			uniqueYearList->year,
			uniqueYearList->rating,
			uniqueYearList->title);
		uniqueYearList = uniqueYearList->next;
	}

	printf("\n");

	uniqueYearList = uniqueListRef;
	while (uniqueYearList != 0)
	{
		uniqueListRef = uniqueYearList->next;
		free(uniqueYearList);
		uniqueYearList = uniqueListRef;
	}
}

void _showByLanguage(struct movie *list)
{
	char lang[20];
	unsigned int matchCount = 0;
	
	printf("\nEnter the language for which you want to see movies: ");
	scanf("%s", lang);

	while (list != 0)
	{
		//printf("\n%s\n", list->languages);		
		if (findStr(list->languages, lang))
		{
			printf("%d %s\n", list->year, list->title);	
			matchCount++;
		}
		list = list->next;
	}

	if (matchCount == 0)
	{
		printf("\nNo movies exist with that langauge.\n");
	}
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

void printMenuChoices(int val, struct movie *list)
{
	switch(val)
	{
		case 1: _showByYear(list); break;
		case 2: _showByRating(list); break;
		case 3: _showByLanguage(list); break; 	
		default: break;
	}
}
