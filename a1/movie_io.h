#include "movie.h"

struct movie* _createMovie(char*);

struct movie* processFile(const char*);

void _printMovie(struct movie*);
void _printByTitle(struct movie*);

void _printMovieList(struct movie*);

void printMovieMenu();

void _showByYear(struct movie*);
void _showByRating(struct movie*);
void _showByLanguage(struct movie*);

void printMenuChoice(int, struct movie*);

int getMenuChoice();

