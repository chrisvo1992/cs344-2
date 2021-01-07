
struct movie 
{
	char* title;
	char* year;
	char* languages;
	char* rating;
	struct movie* next;
};


struct movie* createMovie(char*);

struct movie* processFile(const char*);

void printMovie(struct movie*);

void printByTitle(struct movie*);

void printMovieList(struct movie*);

void printMovieMenu();

void showByYear(struct movie*);
void showByRating(struct movie*);
void showByLanguage(struct movie*);

void printMenuChoice(int, struct movie*);

int getMenuChoice();

