#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>
#include <errno.h>
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(errno);				        \
		}							\
	} while (0)

// structure for a node in k-d tree
typedef struct _node {
	// points stored in node
	int *points;
	// dimension of the space
	int k;
	struct _node *left;
	struct _node *right;
} _node;

typedef struct{
	// list of points
	int **list_of_points;
	// number of points
	int size;
	int max_capacity;
} _list;

// function which create a list of points
_list *create_new_list(void)
{
	_list *new_list = (_list *)malloc(sizeof(_list));
	DIE(!new_list, "malloc failed");
	new_list->max_capacity = 200;

	new_list->list_of_points = (int **)
	malloc(sizeof(int *) * new_list->max_capacity);
	DIE(!new_list->list_of_points, "malloc failed");

	new_list->size = 0;

	return new_list;
}

// function which add a point to the list
void add(_list *list, int *points, int k)
{
	// check if the capacity has been reached and if realloc
	if (list->size >= list->max_capacity - 3) {
		list->max_capacity += 10;
		list->list_of_points = (int **)
		realloc(list->list_of_points, list->max_capacity * sizeof(int *));
	}

	// allocate memory for a new point in list of point
	list->list_of_points[list->size] = (int *) malloc(sizeof(int) * k);
	DIE(!list->list_of_points[list->size], "malloc failed");
	list->size++;

	for (int i = 0; i < k; i++)
		list->list_of_points[list->size - 1][i] = points[i];
}

// remove all points from the list
void remove_all(_list *list)
{
	// free the memory for each point
	for (int i = 0; i < list->size; i++)
		free(list->list_of_points[i]);

	free(list->list_of_points);
	free(list);
}

// create a node for k-d tree
_node *new_node(int *points, int k)
{
	_node *node = (_node *)malloc(sizeof(_node));
	DIE(!node, "malloc failed");

	node->points = (int *) malloc(sizeof(int) * k);
	DIE(!node->points, "malloc failed");
	node->k = k;

	for (int i = 0; i < k; i++)
		node->points[i] = points[i];

	node->left = NULL;
	node->right = NULL;
	return node;
}

// function which insert a point in k-d tree
void insert(_node **node, int *points, int k, int depth)
{
	if (!(*node)) {
		*node = new_node(points, k);
	} else {
		// calculate remainder of the depth division at k
		int dimension = depth % k;

		// check if the value of new point is lower than value of current node
		// if yes put it in the left subtree else in right subtree
		if (points[dimension] < (*node)->points[dimension])
			insert(&((*node)->left), points, k, depth + 1);
		else
			insert(&((*node)->right), points, k, depth + 1);
	}
}

// function which calculate distance euclidean between 2 points
double distance(int *p1, int *p2, int k)
{
	double dist = 0.;
	for (int i = 0; i < k; i++)
		dist += ((p1[i] - p2[i]) * (p1[i] - p2[i]));

	return sqrt(dist);
}

// find the nearest neighbor of a given point(NN)
void nearest_neighbor(_node *cn, int *tp, int *np, int depth)
{
	// if current node is null stop
	if (!cn)
		return;
	// distance between curent node and target point
	double cd = distance(cn->points, tp, cn->k);
	// distance between the nearest point and the target point
	double nd = distance(np, tp, cn->k);

	// if the distance of the current node is smaller than distance
	// of the nearest point , current node become nearest
	if (cd < nd) {
		for (int i = 0; i < cn->k; i++)
			np[i] = cn->points[i];
	}

	int dim = depth % cn->k;

	// if the target point is smaller than current point
	// do a recursive search in the left tree
	if (tp[dim] < cn->points[dim]) {
		nearest_neighbor(cn->left, tp, np, depth + 1);

	// if the distance to the nearest point is greater than difference
	// between of the target point and current node
	// then do a recursive search in the rright subtree
		if (nd > abs(cn->points[dim] - tp[dim]))
			nearest_neighbor(cn->right, tp, np, depth + 1);
	} else {
		nearest_neighbor(cn->right, tp, np, depth + 1);

		if (nd > abs(cn->points[dim] - tp[dim]))
			nearest_neighbor(cn->left, tp, np, depth + 1);
	}
}

// function which check a point is in range
int in_range(int *points, int *start, int *end, int k)
{
	for (int i = 0; i < k; i++)
		if (points[i] < start[i] || points[i] > end[i])
			return 0;
	return 1;
}

// find all points within a given range
void range_search(_node *node, int *s, int *e, int k, _list *r)
{
	if (!node)
		return;

	int dim = k % node->k;
	// if start value point is less or equal than current node
	// do search recursive left subtree
	if (s[dim] <= node->points[dim])
		range_search(node->left, s, e, k + 1, r);

	// check if current node is in range
	// if yes then add in result list 'r'
	if (in_range(node->points, s, e, node->k))
		add(r, node->points, node->k);

	// end value point is greater than value current node
	// do search recursive in right subtree
	if (e[dim] > node->points[dim])
		range_search(node->right, s, e, k + 1, r);
}

// go through tree and add values which are in the start end range
_list *find_points_in_range(_node *current_node, int *start, int *end)
{
	_list *result = create_new_list();
	range_search(current_node, start, end, current_node->k, result);
	return result;
}

// function which destroy k- d tree
void destroy(_node *root)
{
	if (!root)
		return;

	destroy(root->left);
	destroy(root->right);

	free(root->points);
	free(root);
}

// function which display a point
void display_point(int *point, int k)
{
	for (int i = 0; i < k; i++)
		printf("%d ", point[i]);
	printf("\n");
}

// function which load data from the file
void load(const char *filename, _node **root)
{
	FILE *file = fopen(filename, "r");
	if (!file) {
		printf("Error opening file: %s\n", filename);
		return;
	}
		int n, k;
		fscanf(file, "%d %d", &n, &k);

		for (int i = 0; i < n; i++) {
			int p[k];
			// each point is add at k-d tree
			for (int j = 0; j < k; j++)
				fscanf(file, "%d", &p[j]);
			insert(root, p, k, 0);
		}

		fclose(file);
}

int main(void)
{
	_node *root;

	root = NULL;

	char option[10];

	while (1) {
		scanf("%s", option);

		if (strcmp(option, "LOAD") == 0) {
			char filename[50];
			scanf("%s", filename);

			load(filename, &root);

		} else if (strcmp(option, "NN") == 0) {
			int value[root->k], n[root->k];
			for (int i = 0; i < root->k; i++) {
				scanf("%d", &value[i]);
				n[i] = root->points[i];
			}
			// find the nearest point
			nearest_neighbor(root, value, n, 0);
			// display point found
			display_point(n, root->k);
		} else if (strcmp(option, "RS") == 0) {
			int s[root->k], e[root->k];
			for (int i = 0; i < root->k; i++)
				scanf("%d %d", &s[i], &e[i]);
			// find the points in the given interval
			_list *result = find_points_in_range(root, s, e);

			// display points found
			for (int i = 0; i < result->size; i++) {
				int *rr = result->list_of_points[result->size - 1 - i];
				display_point(rr, root->k);
			}

			remove_all(result);

		} else if (strcmp(option, "EXIT") == 0) {
			destroy(root);
			exit(0);
		}
	}

	destroy(root);
}
