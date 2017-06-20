#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#define INFINITY 1000000000
// For vertical and horizontal ONLY

typedef struct {
	int x, y, value;
} Field;

typedef struct TempQueueElement {
	Field* content;
	struct TempQueueElement *behind;
} QueueElement;

typedef struct {
	char* fields;
	int xMax;
	int yMax;
} Lab;

Lab* LabRead(FILE*);
int LabSolve(Lab*, int);
Field* getStartField(Lab*);
int escape(Lab*, int, int);
int greedy(Lab*, int, int);
int bfs(Lab*, Field*);
int getLabWidth(FILE*);
int getLabHeight(FILE*);
void printOutputField(Lab*);
void rewindOutputField();
void setEvaluated(Lab*, int, int, int);
void setField(Lab*, int, int, char);
bool isFree(Lab*, int, int);
bool exists(Lab*, int, int);
char getField(Lab*, int, int);
void printHelp(char*);

int move_size = 7;
int xShift[] = { 0, -1, 1, 0 };
int yShift[] = { -1, 0, 0, 1 };
int xShift_diag[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
int yShift_diag[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
int *p_xShift = xShift_diag, *p_yShift = yShift_diag;
QueueElement *first_q, *last_q;
bool silent = false;
int delay = 0;

int main(int argc, char* argv[]) {
	FILE* in = stdin;
	Lab* pLab = NULL;
	int search, result;
	if (argc < 3) {
		printHelp(argv[0]);
		return 1;
	} else {
		in = fopen(argv[1], "r");
		if (!in) {
			perror(argv[1]);
			return -1;
		}
		if (!strcmp(argv[2], "-escape")) {
			search = 0;
		} else if (!strcmp(argv[2], "-greedy")) {
			search = 1;
		} else if (!strcmp(argv[2], "-bfs")) {
			search = 2;
		} else {
			printf("Unknown search %s\n", argv[2]);
			printHelp(argv[0]);
			return -1;
		}
		if (argc == 4) { // silent OR linear
			silent = !strcmp(argv[3], "-s");
			if (!silent) {
				if (!strcmp(argv[3], "-l")) {
					move_size = 4;
					p_xShift = xShift;
					p_yShift = yShift;
				} else {
					printHelp(argv[0]);
					return 1;
				}
			}
		} else if (argc == 5) { //  (delay) OR (silent AND linear)
			bool syntax = !strcmp(argv[3], "-t");
			if (syntax) {
				delay = atoi(argv[4]);
			} else {
				silent = !strcmp(argv[3], "-s") || !strcmp(argv[4], "-s");
				if (!strcmp(argv[3], "-l") || !strcmp(argv[4], "-l")) {
					move_size = 4;
					p_xShift = xShift;
					p_yShift = yShift;
				} else {
					printHelp(argv[0]);
					return 1;
				}
			}
		} else if (argc == 6) { // (silent OR linear) AND delay
			if (!strcmp(argv[3], "-s") && !strcmp(argv[4], "-t")) {
				silent = true;
				delay = atoi(argv[5]);
			} else if (!strcmp(argv[5], "-s") && !strcmp(argv[3], "-t")) {
				silent = true;
				delay = atoi(argv[4]);
			} else if (!strcmp(argv[3], "-l") && !strcmp(argv[4], "-t")) {
				move_size = 4;
				p_xShift = xShift;
				p_yShift = yShift;
				delay = atoi(argv[5]);
			} else if (!strcmp(argv[5], "-l") && !strcmp(argv[3], "-t")) {
				move_size = 4;
				p_xShift = xShift;
				p_yShift = yShift;
				delay = atoi(argv[4]);
			} else {
				printHelp(argv[0]);
				return 1;
			}
		} else if (argc == 7) { // silent AND linear AND delay
			silent = true;
			move_size = 4;
			p_xShift = xShift;
			p_yShift = yShift;

			if (!strcmp(argv[3], "-s")) {
				if (!strcmp(argv[4], "-l")) {
					if (!strcmp(argv[5], "-t")) {
						delay = atoi(argv[6]);
					} else {
						printHelp(argv[0]);
						return 1;
					}
				} else if (!strcmp(argv[6], "-l")) { // 5 not needed
					if (!strcmp(argv[4], "-t")) {
						delay = atoi(argv[5]);
					} else {
						printHelp(argv[0]);
						return 1;
					}
				} else {
					printHelp(argv[0]);
					return 1;
				}
			} else if (!strcmp(argv[4], "-s")) {
				if (!strcmp(argv[3], "-l")) {
					if (!strcmp(argv[5], "-t")) {
						delay = atoi(argv[6]);
					} else {
						printHelp(argv[0]);
						return 1;
					}
				} else {
					printHelp(argv[0]);
					return 1;
				}
			} else if (!strcmp(argv[5], "-s")) {
				if (!strcmp(argv[3], "-t")) {
					delay = atoi(argv[4]);
					if (strcmp(argv[6], "-l")) {
						printHelp(argv[0]);
						return 1;
					}
				} else {
					printHelp(argv[0]);
					return 1;
				}
			} else if (!strcmp(argv[6], "-s")) {
				if (!strcmp(argv[3], "-l")) {
					if (strcmp(argv[4], "-t")) {
						delay = atoi(argv[5]);
					} else {
						printHelp(argv[0]);
						return 1;
					}
				} else if (!strcmp(argv[5], "-l")) {
					if (strcmp(argv[3], "-t")) {
						delay = atoi(argv[4]);
					} else {
						printHelp(argv[0]);
						return 1;
					}
				} else {
					printHelp(argv[0]);
					return 1;
				}
			} else {
				printHelp(argv[0]);
				return 1;
			}
		}
	}
	pLab = LabRead(in);

	result = LabSolve(pLab, search);

	free(pLab->fields);
	fclose(in);
	return result;
}

void printHelp(char* a) {
	fprintf(stderr, "Usage: %s [<file>] [<algorithm>] [<args>]"
			"\nSearches:"
			"\t\n-escape Just finds a path to escape."
			"\t\n-greedy Finds mostly a good path."
			"\t\n-bfs Finds best path via breadth first search."
			"\nOptionals:"
			"\t\n-l Linear-only. Default is diagonally."
			"\t\n-s Silent searching without unnessecary output. Default is not silent."
			"\t\n-t <nanoseconds> Delay in nanoseconds. Accepts integer values. Default is 0."
			"\n", a);
}

void enqueue(QueueElement* el) {
// initial
	if (!first_q) {
		first_q = el;
	} else {
		last_q->behind = el;
	}
	last_q = el;
}

QueueElement* dequeue() {
	QueueElement* one = first_q;
	first_q = first_q->behind;
// only one or none remaining
	if (!first_q || !first_q->behind) {
		last_q = first_q;
	}
	return one;
}

int getLabWidth(FILE* pFile) {
	int x = 0;
	char temp;
	while ((temp = fgetc(pFile)) != EOF) {
		if (temp == '\n') {
			return x;
		} else {
			x++;
		}
	}
	rewind(pFile);
	return 1;
}

int getLabHeight(FILE* pFile) {
	int yMax = 1;
	char temp;
	while ((temp = fgetc(pFile)) != EOF)
		if (temp == '\n')
			yMax++;
	rewind(pFile);
	return yMax;
}

void rewindOutputField() {
	printf("\033[2;1H");
}

void printOutputField(Lab* pLab) {
	printf("Solving...");
	for (int i = 0; i < pLab->xMax; i++) {
		printf(" ");
	}
	printf("\n");
	for (int y = 0; y < pLab->yMax; y++) {
		for (int x = 0; x < pLab->xMax; x++) {
			printf("%c", pLab->fields[y * pLab->xMax + x]);
		}
		printf("\n");
	}
}

Field* getStartField(Lab* pLab) {
	Field* field = malloc(sizeof(Field));
	for (int y = 0; y < pLab->yMax; y++) {
		for (int x = 0; x < pLab->xMax; x++) {
			if (*(pLab->fields + y * pLab->xMax + x) == 'S') {
				field->x = x;
				field->y = y;
				return field;
			}
		}
	}
	perror("Error: Unable to find start field 'S'.\n");
	exit(EXIT_FAILURE);
}

Lab* LabRead(FILE* file) {
	Lab* pLab = malloc(sizeof(Lab));
	pLab->xMax = getLabWidth(file);
	pLab->yMax = getLabHeight(file);
	pLab->fields = malloc(sizeof(char) * pLab->xMax * pLab->yMax);
	char temp;
	int x = 0, y = 0;
	while ((temp = fgetc(file)) != EOF) {
		if (temp == '\n') {
			x = 0;
			y++;
		} else {
			pLab->fields[y * pLab->xMax + x] = temp;
			x++;
		}
	}
	rewind(file);

	return pLab;
}

bool exists(Lab* pLab, int x, int y) {
	return x > 0 && y > 0 && x < pLab->xMax && y < pLab->yMax;
}

bool isFree(Lab* pLab, int x, int y) {
	return getField(pLab, x, y) == ' ';
}

bool isGoal(Lab* pLab, int x, int y) {
	return getField(pLab, x, y) == 'X';
}

void setVisited(Lab* pLab, int x, int y) {
	setField(pLab, x, y, '.');
}

void setUnvisited(Lab* pLab, int x, int y) {
	setField(pLab, x, y, ' ');
}

void setField(Lab* pLab, int x, int y, char value) {
	*(pLab->fields + y * pLab->xMax + x) = value;
}

char getField(Lab* pLab, int x, int y) {
	return pLab->fields[y * pLab->xMax + x];
}

void setEvaluated(Lab* pLab, int x, int y, int value) {
	setField(pLab, x, y, value + '0');
}

Field* dummyMove() {
	Field* d = malloc(sizeof(Field));
	d->x = -2;
	d->y = -2;
	d->value = INFINITY;
	return d;
}

Field* cloneMove(Field* src) {
	Field* result = malloc(sizeof(Field));
	result->x = src->x;
	result->y = src->y;
	result->value = src->value;
	return result;
}

int greedy(Lab* pLab, int currX, int currY) {
	int x, y, v, minV = INFINITY;

// Update the UI
	setVisited(pLab, currX, currY);
	usleep(delay);
	if (!silent) {
		rewindOutputField();
		printOutputField(pLab);
	}

	for (int i = 0; i < move_size; i++) {
		x = currX + p_xShift[i];
		y = currY + p_yShift[i];
		if (exists(pLab, x, y)) {
			if (isGoal(pLab, x, y)) {
				return 1;
			} else if (isFree(pLab, x, y)) {
				v = greedy(pLab, x, y);
				if (v < minV) {
					minV = v;
				}
			}
		}
	}

	return minV + 1;
}

int escape(Lab* pLab, int currX, int currY) {
	int x, y;

	for (int i = 0; i < move_size; i++) {
		x = currX + p_xShift[i];
		y = currY + p_yShift[i];
		if (exists(pLab, x, y)) {
			if (isGoal(pLab, x, y)) {
				return 1;
			} else if (isFree(pLab, x, y)) {
				// Update the UI
				setVisited(pLab, x, y);
				usleep(delay);
				if (!silent) {
					rewindOutputField();
					printOutputField(pLab);
				}
				if (escape(pLab, x, y)) {
					return 1;
				}
			}
		}
	}

	return 0;
}

int bfs(Lab* pLab, Field* startField) {
	QueueElement *start = malloc(sizeof(QueueElement)), *temp_q = NULL, *temp_q_next = NULL;
	Field *temp_m = NULL, *temp_m_next = NULL;
	int x, y;

	first_q = NULL;
	last_q = NULL;

	startField->value = 0;
	start->content = startField;
	start->behind = NULL;
	enqueue(start);

	do {
		temp_q = dequeue();
		temp_m = temp_q->content;

// Update the UI
		usleep(delay);
		if (!silent) {
			rewindOutputField();
			printOutputField(pLab);
		}

		for (int i = 0; i < move_size; i++) {
			x = temp_m->x + p_xShift[i];
			y = temp_m->y + p_yShift[i];
			if (exists(pLab, x, y)) {
				if (isFree(pLab, x, y)) {
					temp_m_next = malloc(sizeof(Field));
					temp_m_next->x = x;
					temp_m_next->y = y;
					temp_m_next->value = temp_m->value + 1;
					setVisited(pLab, temp_m_next->x, temp_m_next->y);
					temp_q_next = malloc(sizeof(QueueElement));
					temp_q_next->behind = NULL;
					temp_q_next->content = temp_m_next;
					enqueue(temp_q_next);
				} else if (isGoal(pLab, x, y)) {
					return temp_m->value + 1;
				}
			}
		}

		free(temp_q);
	} while (first_q);

	return 0;
}

int LabSolve(Lab* pLab, int search) {
	Field* startField = getStartField(pLab);
	int result_i = 0;
	if (!silent) {
		printf("Press enter to start solving...\n");
		for (int y = 0; y < pLab->yMax; y++) {
			for (int x = 0; x < pLab->xMax; x++) {
				printf("%c", getField(pLab, x, y));
			}
			printf("\n");
		}
		while (getchar() != '\n')
			;
		printf("\n");
	}
	switch (search) {
	case 0: // Escape
		result_i = escape(pLab, startField->x, startField->y);
		if (result_i)
			printf("Solved.\n");
		else
			printf("Impossible.\n");
		break;
	case 1: // Greedy
		result_i = greedy(pLab, startField->x, startField->y);
		if (result_i < INFINITY) {
			printf("Solved. Good way is %d steps.\n", result_i);
		} else {
			printf("Impossible.\n");
		}
		break;
	case 2: // BFS
		result_i = bfs(pLab, startField);
		if (result_i)
			printf("Solved. Best way is %d steps.\n", result_i);
		else
			printf("Impossible.\n");
		break;
	}
	free(startField);
	return result_i;
}

