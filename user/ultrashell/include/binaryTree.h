#ifndef BINARYTREE_H_INCLUDED
#define BINARYTREE_H_INCLUDED



#define binary_tree struct branch

#define MAX_COMMAND_LENGTH 20

#define SUCCESS 0
#define BINARY_TREE_CORRUPTED -1
#define COMMAND_ALREADY_ADDED -2



enum branchType
{
	moreBranching, branchEnd
};


struct condition	// not sure if i really want to make it like this
{
	int number;
	char value;
};


struct function
{
	char *command;
	int (*f)(int, char **);
};


struct branching
{
	struct condition *c;
	struct branch *left;
	struct branch *right;
};


struct branch
{
	enum branchType type;
	union branchElement *element;
};


union branchElement
{
	struct branching *b;
	struct function *f;
};



struct function *searchFunction(binary_tree *tree, char *function_name);
int addFunction(binary_tree *tree, struct function *new_function);
binary_tree *create_tree(struct function *dummy);



#endif
