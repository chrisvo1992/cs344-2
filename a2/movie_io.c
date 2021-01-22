#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include "movie_io.h"

#define PREFIX "movies_"
#define ONID "johnsal.movies."

/****DESCRIPTION****/
// - read a csv file as an arg to main
// - create structs to hold each movie
// - create a linked list of these structs
// - give user choices to answer questions about the movie data
// - prints info about movies based on user choices
//

/*** REFERENCES ***/
// https://en.cppreference.com/w/c/string/byte/strtok
// https://en.cppreference.com/w/c/memory/calloc
// https://linux.die.net/man/3/getline
//

// reads a line of input from a file and allocates
// memory for a movie struct
// input: a csv line
// output: a movie type containing the csv contents.
struct movie* _createMovie(char* line)
{
	struct movie *currMovie = malloc(sizeof(struct movie));
	// blank node for a new langauge
	struct node *newLang = 0;
	
	struct node *tempLang = 0;
	struct node *langList = 0;
	char *refToken;
	char *saveLangToken;
	char *savePtr;
	int colonCount = 0;
	size_t i = 0;

	// movie title
	char* token = strtok_r(line, ",", &savePtr);
	currMovie->title = calloc(strlen(token) + 1, sizeof(char));
	strcpy(currMovie->title, token);

	// movie year
	token = strtok_r(NULL, ",", &savePtr);
	currMovie->year = atoi(token);

	// movie languages
	//token = strtok_r(NULL, ";", &savePtr);
	saveLangToken = savePtr;
	token = strtok_r(NULL, ",", &savePtr);

	// get rid of the opening bracket
	//langToken++;
	token++;

	// get rid of the closing bracket
	token[strlen(token) - 1] = '\0';

	// preserve the location of the read location
	refToken = token;

	// movie rating
	token = strtok_r(NULL, "\n", &savePtr);
	currMovie->rating = atof(token);

	// count the number of languages
	// NOTE: compiler says that this for loop is not used. After
	// examining GDB, it has to be used because colonCount is 
	// incrementing. I suspect that the gdb warning occurs when 
	// there is only one language, in which case, this logic
	// would not be used.
	for (i; i < strlen(refToken); ++i)
	{
		if (refToken[i] == ';')
		{
			colonCount++;	
		}
	}

	// the number of languages will equal
	// colonCount + 1. Since there is at least always
	// one language, read it and loop over the remaining
	// languages, if any.
	token = strtok_r(refToken, ";", &saveLangToken);
	newLang = malloc(sizeof(struct node));
	newLang->val = calloc(strlen(token) + 1, sizeof(char));
	strcpy(newLang->val, token);

	// save the starting point of the list
	langList = newLang;

	// since i is only used in loops, it does not need
	// to be reset, *it seems*.
	i = 0;

	while (i < colonCount)
	{
		tempLang = newLang;
		token = strtok_r(NULL, ";", &saveLangToken);
		newLang = malloc(sizeof(struct node));
		tempLang->next = newLang;
		newLang->val = calloc(strlen(token) + 1, sizeof(char));
		newLang->next = 0;
		strcpy(newLang->val, token);
		i++;
	}

	// point the list of languages to the movie language list
	currMovie->languages = langList;

	currMovie->next = NULL;
	
	return currMovie;
}

// creates a linked list of movies, using
// 	_createMovie as a helper function.
// input: an csv file 
// output: a linked list (unsorted) of movies.
struct movie *_processFile(char* filePath)
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

	// read the first line and discard
	read = getline(&currLine, &len, movieFile);

	while ((read = getline(&currLine, &len, movieFile)) != -1)
	{
		// if the line is blank
		if (currLine[0] == '\n') 
		{
			continue;
		}
		else 
		{
			struct movie *newNode = _createMovie(currLine);

			// create the first node if empty
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


// creates a linked list of movies, using
// 	_createMovie as a helper function.
// input: an csv file and an address reference to 
// 	the number of movies read from the file.
// output: a linked list (unsorted) of movies.
/*
struct movie *_processFile(const char* filePath, int *size)
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

	// read the first line and discard
	read = getline(&currLine, &len, movieFile);

	while ((read = getline(&currLine, &len, movieFile)) != -1)
	{
		// if the line is blank
		if (currLine[0] == '\n') 
		{
			continue;
		}
		else 
		{
			struct movie *newNode = _createMovie(currLine);

			// used as a reference to keep track of the number of 
			// movies in the provided file.
			(*size)++;

			// create the first node if empty
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
*/

// creates a list of unique years for use in the 
// _sortByRating function. Reduces the repeated 
// allocation of memory when option 2 is chosen.
struct movie* _createUniqueYearList(struct movie* list)
{
	struct movie *uniqueYears = 0;
	struct movie *tempNode = 0;
	struct movie *newNode = 0;

	// The list is presorted in asc order, therefore the first
	// value read is assigned to the first node in the uniqueYear
	// linked list.
	
	newNode = malloc(sizeof(struct movie));
	newNode->year = list->year;
	newNode->title = list->title;
	newNode->rating = list->rating;
	newNode->next = 0;
	uniqueYears = newNode;

	while (list != 0)
	{
		if (newNode->year != list->year)
		{
			tempNode = newNode;
			newNode = malloc(sizeof(struct movie));
			tempNode->next = newNode;
			newNode->title = list->title;
			newNode->year = list->year;
			newNode->rating = list->rating;
			newNode->next = 0;
		}
		list = list->next;
	}

	return uniqueYears;
}

// finds a string within a linked list of lang nodes.
// input: the string being searched for 
// 	and the string being searched.
// output: int representing true or false.
int _findStr(char const *str, char const *subStr)
{
	// lazy check of the user input
	char *pos = strstr(str, subStr);

	// strstr return true for substrings as well as
	// substrings of substrings. 
	// if the user input a str that is the length of the 
	// language and it matches
	if (pos && strlen(str) == strlen(subStr))
	{
		return 1;
	}
	else
	{
		return 0;
	}	
}

// prints the list of languages for that movie
// input: a list of languages 
// output: stdout
void _printLanguages(struct node *langs)
{
	while (langs != 0)
	{
		printf("%s ", langs->val);	
		langs = langs->next;
	}
}

// prints a node in the linked list of movies
// input: a linked list of movies
// output: stdout
void _printMovie(struct movie *aMovie)
{
	struct node *languages = aMovie->languages;

	printf("%s, %d",
		aMovie->title,
		aMovie->year);
	_printLanguages(languages),
	printf("%f", aMovie->rating);
}

// prints the linked list of movies by title only
// input: a linked list of movies, title only
// output: stdout
void _printByTitle(struct movie *aMovie)
{
	printf("%s\n", aMovie->title);
}

// prints the entire linked list of movies,
// using _printMovie as a helper function.
// input: a linked list of movies
// output: stdout
void _printMovieList(struct movie *list)
{
	while (list != NULL)
	{
		_printMovie(list);
		list = list->next;
	}
}

// prints the menu instructions for the user
// to follow.
// input: stdin
// output: stdout
void printMovieMenu()
{
	printf("\n1. Select the file to process\n");
	printf("2. Exit from the program\n\n");
}

// when movie menu option 1 is chosen, the user is
// asked what year they would like to see movies for.
// input: a linked list of movies
// output: a list of movies created that match the 
// 	year the user enters to stdout.
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

// when movie menu option 2 is chosen, the user does
// not input any values. Instead, a list of unique years
// is created and destroyed. Movies with the same year 
// have their rating compared and swapped using the greater
// of the two. 
// input: a linked list of movies
// output: stdout of each year that has a movie with the
// highest rating.
void _showByRating(struct movie *list, struct movie *uniqueList)
{
	//struct movie *uniqueYear = 0;
	//struct movie *headRef = list;
	//struct movie *tempNode = 0;

	while (uniqueList != 0)
	{
		if (list != 0)
		{
			// check all the years that == the 
			// year in the uniqueYear node
			while (list->year == uniqueList->year)
			{
				if (list->rating > uniqueList->rating)
				{
					uniqueList->title = list->title;
					uniqueList->year = list->year;
					uniqueList->rating = list->rating;
				}
				if (list->next != 0)
				{
					list = list->next;
				} else {break;}
			}
		}
		printf("%u ", uniqueList->year);
		printf("%0.1f ",uniqueList->rating);
		printf(uniqueList->title);
		printf("\n");
		uniqueList = uniqueList->next;
	}
}

// when movie menu option 3 is chosen, asks the user to enter
// a language that they would like to search the movie list for.
// uses _findStr helper function to search the languages field of the 
// movie, exact matches only. When a movie is with that language is
// found, prints that movie to stdout.
// input: a linked list of movies
// output: stdout of movies that are in the language entered by the user.
void _showByLanguage(struct movie *list)
{
	char lang[20];
	size_t matchCount = 0;
	struct node *langRef = list->languages;
	
	printf("\nEnter the language for which you want to see movies: ");
	scanf("%s", lang);

	while (list != 0)
	{
		while (langRef != 0)
		{
			if (_findStr(langRef->val, lang))
			{
				matchCount++;	
				printf("%d %s\n", list->year, list->title);
			}
			langRef = langRef->next;
		}

		list = list->next;

		if (list != 0)
		{
			langRef = list->languages;
		}
	}

	if (matchCount == 0)
	{
		printf("\nNo movies exist with that langauge.\n");
	}
}

void _createFilesByUniqueYear(const char *dirname,struct movie *list, struct movie *uniqueList)
{
	FILE *newFile = 0;
	char cstr[9]; 
	// move into the dir <dirname>
	int newDir = chdir(dirname);

	/* //for debugging only
	char *buf = 0;
	char *cwd = getcwd(buf, malloc(sizeof(strlen(dirname + 1) * sizeof(char))));
	printf("cwd: %s\n", cwd);
	*/

	// parse data to find out the movies released in each year. 
	// create a file named <year>.txt for each year in 
	// which >= 1 movie was released
	// and set each file permission to 0640
	
	while (uniqueList != 0)
	{
		// create a new file <year>.txt
		sprintf(cstr, "%d", uniqueList->year);
		strcat(cstr, ".txt");
   	newFile = fopen(cstr, "a+");

		if (list != 0)
		{
			while (list->year == uniqueList->year)
			{
				//if (list->rating > uniqueList->rating)
				//{
					fputs(list->title, newFile);
					fputs("\n", newFile);
					/*
					uniqueList->title = list->title;
					uniqueList->year = list->year;
					uniqueList->rating = list->rating;
					*/
				//}
				if (list->next != 0)
				{
					list = list->next;
				} else {break;}
			}
		}
		/*
		printf("%u ", uniqueList->year);
		printf("%0.1f ",uniqueList->rating);
		printf(uniqueList->title);
		printf("\n");
		*/
		uniqueList = uniqueList->next;
	}
}

void _readFile(struct movie *list, struct movie *uniqueList)
{

	//DIR* currDir = opendir(".");
	int newDir;
	char str[] = ONID;
	struct dirent *aDir;
	struct stat dirStat;
	
	strcat(str,"12345\0");

	// create a new dir named <onid>.movies.<rand_num>
	// with permissions set to 0750
	
	newDir = mkdir(str, 0750);	
	
	// print the name of the dir 
	printf("Created directory with name ");
	printf(" %s\n", str);

	// parse data to find out the movies released in each year. 
	// create a file named <year>.txt for each year in 
	// which >= 1 movie was released
	// and set each file permission to 0640
	
	_createFilesByUniqueYear(str, list, uniqueList);

	// in each file, write the titles of every movie with 
	// the same year on a single line.
	//
	//
}

void _findLargestFile()
{
	printf("find by largest file\n");
}

void _findSmallestFile()
{
	printf("find by smallest file\n");
}

// asks for the name of the file.
// checks if file exists and writes err msg if the file 
// is not found. Repeats this process until a file is 
// found.
// input:
// output:
void _specifyFile()
{
	struct movie *list = 0;
	struct movie *sortedList = 0;
	struct movie *uniqueYears = 0;
	struct movie *ref1 = 0;
	// for the languages in each movie
	struct node *ref2 = 0;
	struct node *ref3	= 0;

	char filename[] = "";

	int file_descriptor = -1;

	while (file_descriptor < 0)
	{
		printf("Enter the complete filename: %s", filename);
		scanf("%s", filename);

		file_descriptor = open(filename, O_RDONLY, 0440);
		if (file_descriptor < 0)
		{
			strcpy(filename, "");
			printf("failed to open \"%s\"\n", filename);
			perror("Error");
		}
	}

	printf("Now processing the chosen file named %s\n", filename);

	list = _processFile(filename);
	sortedList = _mergeSort(list);
	uniqueYears = _createUniqueYearList(sortedList);

	close(file_descriptor);

	_readFile(sortedList, uniqueYears);

	while (sortedList != 0)
	{
		ref1 = sortedList;
		free(ref1->title);
		ref2 = sortedList->languages;
		while (ref2 != 0)
		{
			ref3 = ref2;
			free(ref3->val);		
			free(ref3);
			ref2 = ref2->next;
		}
		free(ref1);
		sortedList = sortedList->next;
	}
	free(sortedList);

	while (uniqueYears != 0)
	{
		ref1 = uniqueYears;
		uniqueYears = ref1->next;
		free(ref1);
	}
}

void _selectFile()
{
	char str[2];
	char *str1 = NULL;
	int choice;
	
	printf("Which file do you want to process?\n");
	printf("Enter 1 to pick the largest file\n");
	printf("Enter 2 to pick the smallest file\n");
	printf("Enter 3 to specify the name of a file\n");
	printf("Enter a choice from 1 to 3: ");
	// only expect one char to be entered. no more, no less.
	scanf("%s", str);

	str1 = malloc(sizeof(char));
	strcpy(str1, str);

	choice = atoi(str1);

	while (str[1] != '\0' || (choice < 1 || choice > 3))
	{
		
		strcpy(str, "");
		choice = 0;

		printf("Not a valid choice, try again.\n");
		printf("Enter a choice between 1 and 3: ");
		scanf("%s", str);
		strcpy(str1, str);
	
		choice = atoi(str1);	
		
	}
	free(str1);

	switch (choice)
	{
		case 1: _findLargestFile(); break;
		case 2: _findSmallestFile(); break;
		case 3: _specifyFile(); break;
		default: break;
	}	
	
}

// asks the user to enter a choice and gets the user input
// input: none
// output: an integer representing the valid user input.
int getMenuChoice()
{
	// only expect one char to be entered. no more, no less.
	char str[2];
	char *str1 = NULL;
	int choice;
	
	printf("Enter a choice 1 or 2: ");
	scanf("%s", str);

	str1 = malloc(sizeof(char));
	strcpy(str1, str);

	choice = atoi(str1);

	while (str[1] != '\0' || (choice < 1 || choice > 2))
	{
		
		strcpy(str, "");
		choice = 0;

		printf("Not a valid choice, try again.\n");
		printf("Enter a choice 1 or 2: ");
		scanf("%s", str);
		strcpy(str1, str);
	
		choice = atoi(str1);	
		
	}

	free(str1);

	return choice;	
}

// calls the given function the corresponds to the menu
// choice selected by the user.
// input: an integer representing the movie menu choice
// output: the resulting output of the choices made.
void printMenuChoices(int val)
{
	switch(val)
	{
		case 1: _selectFile(); break;
		default: break;
	}
}
