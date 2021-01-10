struct movie 
{
	char* title;
	char* year;
	char* languages;
	char* rating;
	struct movie* next;
};

struct node
{
	int val;
	struct node *next;
};
