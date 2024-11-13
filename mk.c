#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_CHILDREN 26
#define CMD_LENGTH 100
#define MAX_WORD 50
#define LINE_SIZE 200

struct trie_node {
	int is_word;
	int frecv;
	int sons;
	// the number of sons is necessary to know whether or not to remove
	// this node at remove
	// than to realocate many times I created a vector of sons
	struct trie_node *vec[MAX_CHILDREN];
};

void free_trie(struct trie_node *root);

struct trie_node *alloc_node(void)
{
	struct trie_node *nou = (struct trie_node *)calloc(1,
										sizeof(struct trie_node));
	return nou;
}

// the insert function trie creats all the necessary nodes until
// form the whole word
// if the word exists, I increase its frequency
// if the word exists as a prefix it will become a word
void insert(struct trie_node *root, char *word)
{
	if (!root || !word)
		return;

	struct trie_node *p = root;
	int len = strlen(word);

	int contor_alocari = 0;
	for (int i = 0; i < len; i++) {
		int index = word[i] - 'a';
		if (!p->vec[index]) {
			p->sons++;
			p->vec[index] = alloc_node();
			contor_alocari++;
		}

		p = p->vec[index];
	}
	int was_word = p->is_word;
	p->is_word = 1;

	if (!contor_alocari && was_word)
		p->frecv++;
}

// function that counts how many characters differ between 2 strings of
// n elements
int dif(const char *sir1, const char *sir2, int n)
{
	if (!sir1 || !sir2)
		return -1;
	int ret = 0;
	for (int i = 0; i <= n; i++)
		if (sir1[i] != sir2[i])
			ret++;

	return ret;
}

// function that checks if we can add a letter to the autocorrect without
// exceeding the maximum number of allowed differences
int este_sel(const char *word, char *sol, int i, int dif_max, int pas)
{
	if (!word && !sol)
		return 0;

	sol[pas] = 'a' + i;
	int ret;
	if (dif(word, sol, pas) > dif_max)
		ret = 0;
	else
		ret = 1;

	sol[pas] = '\0';
	return ret;
}

void print_first(struct trie_node *root, char *sol)
{
	if (!root || !sol)
		return;
	// if the prefix is itself a word
	if (root->is_word) {
		printf("%s\n", sol);
		return;
	}
	int wi; // write index

	wi = strlen(sol);
	struct trie_node *p = root;
	while (p) {
		// I try to look for the first word in lexicographical order
		// that is, I move p only to the branch with the first letter
		for (int i = 0; i < MAX_CHILDREN; i++)
			if (p->vec[i]) {
				sol[wi++] = 'a' + i;
				p = p->vec[i];
				break;
			}
		// stop if i found a word
		if (p->is_word) {
			printf("%s\n", sol);
			return;
		}
	}
	printf("No words found\n");
}

void browse_shortest(struct trie_node *root, char *so_far,
					 char *final, int *best_len, int *gasite)
{
	if (!root || !so_far || !final || !best_len || !gasite)
		return;
	if (root->is_word) {
		// if it is the first solution, it has -1
		if (*best_len == -1 || *best_len > (int)strlen(so_far)) {
			strcpy(final, so_far);
			*best_len = strlen(so_far);
			(*gasite)++;
		}
		return;
	}

	// if I haven't found a word so far, I keep trying all the
	// posibilities to add another letter
	for (int i = 0; i < MAX_CHILDREN; i++) {
		if (root->vec[i]) {
			char current[MAX_WORD] = {0};
			strcpy(current, so_far);
			current[strlen(so_far)] = 'a' + i;
			browse_shortest(root->vec[i], current, final, best_len, gasite);
		}
	}
}

void browse_most_used(struct trie_node *root, char *so_far,
					  char *final, int *best_frec, int *gasite)
{
	if (!root || !best_frec || !so_far  || !final || !gasite)
		return;

	if (root->is_word) {
		// I see if this word is a new maximum until this moment
		// -1 for the first value found
		if (*best_frec == -1 || *best_frec < root->frecv) {
			// save the new maximal solution
			strcpy(final, so_far);
			*best_frec = root->frecv;
			(*gasite)++;
		}
	}

	// try all the variants that contain our prefix
	for (int i = 0; i < MAX_CHILDREN; i++) {
		if (root->vec[i]) {
			char current[MAX_WORD] = {0};
			strcpy(current, so_far);
			// I add the new letter for which we will look for solutions
			current[strlen(so_far)] = 'a' + i;
			browse_most_used(root->vec[i], current, final, best_frec, gasite);
		}
	}
}

// This function prints shortest words in a trie by recursively exploring
// its nodes and tracking the shortest word found
void print_shortest(struct trie_node *root, char *prefix, int *gasite)
{
	if (!root || !prefix || !gasite)
		return;

	if (root->is_word) {
		printf("%s\n", prefix);
		(*gasite)++;
		return;
	}

	char final[MAX_WORD] = {0};
	int best_len = -1;
	for (int i = 0; i < MAX_CHILDREN; i++) {
		if (root->vec[i]) {
			char current[MAX_WORD] = {0};
			strcpy(current, prefix);
			current[strlen(prefix)] = 'a' + i;
			browse_shortest(root->vec[i], current, final, &best_len, gasite);
		}
	}
	if (*gasite > 0)
		printf("%s\n", final);
	else
		printf("No words found\n");
}

// Function which prints the most used words in a trie by recursively
// exploring its nodes and tracking the word with the highest frequency
void print_most_used(struct trie_node *root, char *prefix, int *gasite)
{
	if (!root || !prefix || !gasite)
		return;
	int best_frec = -1;
	char final[MAX_WORD];

	browse_most_used(root, prefix, final, &best_frec, gasite);

	if (*gasite > 0)
		printf("%s\n", final);
	else
		printf("No words found\n");
}

void autocompletare(struct trie_node *root, const char *word, int op)
{
	if (!root || !word)
		return;

	struct trie_node *p = root;
	// position ourselves at the node corresponding to the prefix
	int n = strlen(word);
	for (int i = 0; i < n; i++) {
		int index = word[i] - 'a';
		if (!p->vec[index]) {
			if (op)
				printf("No words found\n");
			else
				for (int j = 0; j < 3; j++)
					printf("No words found\n");
			return;
		}
		p = p->vec[index];
	}

	int gasite = 0;
	// the smallest lexicographic word
	if (op == 0 || op == 1) {
		char first_word[MAX_WORD] = {0};
		strcpy(first_word, word);
		print_first(p, first_word);
	}
	if (op == 0 || op == 2) {
		gasite = 0;
		char short_word[MAX_WORD] = {0};
		strcat(short_word, word);
		print_shortest(p, short_word, &gasite);
	}
	if (op == 0 || op == 3) {
		gasite = 0;
		char prefix[MAX_WORD] = {0};
		strcat(prefix, word);
		print_most_used(p, prefix, &gasite);
	}
}

void autocorectare(struct trie_node *root, const char *word, int dif_lim,
				   char *sol, unsigned int pas, int *gasite)
{
	if (!root || !word || !sol)
		return;

	if (pas == strlen(word)) {
		if (root->is_word) {
			printf("%s\n", sol);
			(*gasite)++;
		}
		return;
	}
	// i try all possible options
	for (int i = 0; i < MAX_CHILDREN; i++)
		if (root->vec[i] && este_sel(word, sol, i, dif_lim, pas)) {
			sol[pas] = 'a' + i;
			autocorectare(root->vec[i], word, dif_lim, sol, pas + 1, gasite);
		}
	// when exiting the first call, if I have no words found, I display this
	if (pas == 0 && *gasite == 0)
		printf("No words found\n");
}

void remove_word(struct trie_node *root, const char *word)
{
	if (!root || !word)
		return;

	struct trie_node *p = root;

	// the last father with several sons from there on down I will cut the tree
	// or the last father, which is a word in its turn, so i can't cut it
	struct trie_node *father = root;
	// the index of the branch i will cut
	int cut_index = word[0] - 'a';

	// we position ourselves at the node corresponding to the prefix
	int n = strlen(word);
	for (int i = 0; i < n; i++) {
		int index = word[i] - 'a';
		if (!p->vec[index])
			return;

		if (p->sons > 1 || p->is_word) {
			cut_index = index;
			father = p;
		}

		p = p->vec[index];
	}

	// if he has children
	if (p->sons > 0) {
		p->is_word = 0;
		p->frecv = 0;
		return;
	}

	// break the connection
	free_trie(father->vec[cut_index]);
	// and free the memory
	father->vec[cut_index] = NULL;
	if (father->sons > 0)
		father->sons--;
}

void free_trie(struct trie_node *root)
{
	if (!root)
		return;
	for (int i = 0; i < MAX_CHILDREN; i++)
		if (root->vec[i])
			free_trie(root->vec[i]);
	free(root);
}

int main(void)
{
	struct trie_node *root = alloc_node();
	char line[CMD_LENGTH];

	while (1) {
		fgets(line, CMD_LENGTH, stdin);
		char *cmd = strtok(line, "\n ");
		if (strcmp(cmd, "INSERT")  == 0) {
			char *word = strtok(NULL, "\n ");
			insert(root, word);
		} else if (strcmp(cmd, "AUTOCORRECT") == 0) {
			char *word = strtok(NULL, "\n ");
			char *arg = strtok(NULL, "\n ");
			int n = atoi(arg);
			char *sol = calloc(strlen(word) + 1, sizeof(char));
			int gasite = 0;

			autocorectare(root, word, n, sol, 0, &gasite);
			free(sol);
		} else if (strcmp(cmd, "AUTOCOMPLETE") == 0) {
			char *word = strtok(NULL, "\n ");
			char *arg = strtok(NULL, "\n ");
			int n = atoi(arg);
			autocompletare(root, word, n);
		} else if (strcmp(cmd, "LOAD") == 0) {
			char *fisier = strtok(NULL, "\n ");
			FILE *f_data = fopen(fisier, "r");
			char file_line[LINE_SIZE];

			while (fgets(file_line, sizeof(file_line), f_data)) {
				char *tok = strtok(file_line, " \t\n");
				insert(root, tok);
				tok = strtok(NULL, " \t\n");
				while (tok) {
					insert(root, tok);
					tok = strtok(NULL, " \t\n");
				}
			}
			fclose(f_data);
		} else if (strcmp(cmd, "REMOVE") == 0) {
			char *word = strtok(NULL, "\n ");
			remove_word(root, word);
		} else if (strcmp(cmd, "EXIT") == 0) {
			break;
		}
	}

	free_trie(root);
	return 0;
}
