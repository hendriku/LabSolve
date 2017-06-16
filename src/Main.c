#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#define INFINITY 1000000000
#define DELAY 0
// For vertical and horizontal ONLY
#define MOVE_SIZE 4
#define NOISY true

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
Lab* LabSolve(Lab*, int);
Field* getStartField(Lab*);
Field* escape(Lab*, int, int);
Field* greedy(Lab*, int, int);
int bfs(Lab*, Field*);
int getLabWidth(FILE*);
int getLabHeight(FILE*);
void initField(Lab*);
void printOutputField(Lab*);
void rewindOutputField();
void setEvaluated(Lab*, int, int, int);
void setChar(Lab*, int, int, char);

QueueElement *first_q, *last_q;

int main(int argc, char* argv[]) {
	FILE* in = stdin;
	Lab* pLab = NULL;
	int search;
	if (argc != 3) {
		fprintf(stderr, "Usage: %s [<file>] [<search>]"
				"\nSearches:"
				"\t\n-escape Just finds a path to escape"
				"\t\n-greedy Finds mostly a good path"
				"\t\n-bfs Finds best path via breadth first search\n", argv[0]);
		return 1;
	} else {
		in = fopen(argv[1], "r");
		if (!in) {
			perror(argv[1]);
			return 1;
		}

		if (!strcmp(argv[2], "-escape")) {
			search = 0;
		} else if (!strcmp(argv[2], "-greedy")) {
			search = 1;
		} else if (!strcmp(argv[2], "-bfs")) {
			search = 2;
		} else {
			printf("Unknown search %s", argv[2]);
			return 1;
		}
	}
	pLab = LabRead(in);

	pLab = LabSolve(pLab, search);

	free(pLab->fields);
	fclose(in);
	return EXIT_SUCCESS;
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

	if (!first_q || !first_q->behind) {
		last_q = first_q;
	}
	return one;
}

int getLabWidth(FILE* pFile) {
	int xMax = 0;
	int x = 0;
	char temp;
	while ((temp = fgetc(pFile)) != EOF) {
		if (temp == '\n') {
			if (x > xMax)
				xMax = x;
			x = 0;
		} else {
			x++;
		}
	}
	rewind(pFile);
	return xMax;
}

int getLabHeight(FILE* pFile) {
	int yMax = 0;
	char temp;
	while ((temp = fgetc(pFile)) != EOF)
		if (temp == '\n')
			yMax++;
	rewind(pFile);
	return yMax;
}

void initField(Lab* pLab) {
	for (int y = 0; y < pLab->yMax; y++) {
		for (int x = 0; x < pLab->xMax; x++) {
			*(pLab->fields + y * pLab->xMax + x) = '.';
		}
	}
}

void rewindOutputField() {
	printf("\033[5;1H");
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
	printf("Reading lab...\n");
	Lab* pLab = malloc(sizeof(Lab));
	pLab->xMax = getLabWidth(file);
	pLab->yMax = getLabHeight(file);
	printf("Dimensions are %d x %d.\n", pLab->xMax, pLab->yMax);
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

Field* getMoves(Lab* pLab, int currX, int currY) {
	Field* fields = malloc(sizeof(Field) * MOVE_SIZE);
	Field* temp = NULL;
	// Not diagonal
	int xShift[] = { 0, -1, 1, 0 };
	int yShift[] = { -1, 0, 0, 1 };
	int x, y;
	for (int i = 0; i < MOVE_SIZE; i++) {
		x = currX + xShift[i];
		y = currY + yShift[i];
		if (x > 0 && y > 0 && x < pLab->xMax && y < pLab->yMax
				&& pLab->fields[pLab->xMax * y + x] == ' ') {
			temp = fields + i;
			temp->value = -1;
			temp->x = x;
			temp->y = y;
		}
	}
	return fields;
}

bool exists(Lab* pLab, int x, int y) {
	return x > 0 && y > 0 && x < pLab->xMax && y < pLab->yMax;
}

bool isAvailable(Lab* pLab, int x, int y) {
	return *(pLab->fields + pLab->xMax * y + x) == ' ';
}

bool isGoal(Lab* pLab, int x, int y) {
	return *(pLab->fields + pLab->xMax * y + x) == 'X';
}

void setVisited(Lab* pLab, int x, int y) {
	*(pLab->fields + y * pLab->xMax + x) = '.';
}

void setUnvisited(Lab* pLab, int x, int y) {
	*(pLab->fields + y * pLab->xMax + x) = ' ';
}

void setChar(Lab* pLab, int x, int y, char value) {
	*(pLab->fields + y * pLab->xMax + x) = value;
}

void setEvaluated(Lab* pLab, int x, int y, int value) {
	*(pLab->fields + y * pLab->xMax + x) = value + '0';
}

Field* isNextToGoal(Lab* pLab, Field* moves) {
	Field* temp = NULL;
	for (int i = 0; i < MOVE_SIZE; i++) {
		temp = (moves + i);
		if (isGoal(pLab, temp->x, temp->y)) {
			temp->value = 1;
			return temp;
		}
	}
	return 0;
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

Field* greedy(Lab* pLab, int currX, int currY) {
	Field *moves = getMoves(pLab, currX, currY), *temp = isNextToGoal(pLab,
			moves), *nextTemp = NULL, *min = dummyMove();

	setVisited(pLab, currX, currY);
	usleep(DELAY);
	if (NOISY) {
		rewindOutputField();
		printOutputField(pLab);
	}

	if (temp) {
		min = cloneMove(temp);
		free(moves);
		return min;
	}

	for (int i = 0; i < MOVE_SIZE; i++) {
		temp = (moves + i);
		if (exists(pLab, temp->x, temp->y)
				&& isAvailable(pLab, temp->x, temp->y)) {
			nextTemp = greedy(pLab, temp->x, temp->y);
			if (nextTemp && nextTemp->value < min->value) {
				temp->value = nextTemp->value;
				min = temp;
			}
			free(nextTemp);
		}
	}

	Field* clone = cloneMove(min);
	setChar(pLab, clone->x, clone->y, '+');
	clone->value++;
	free(moves);
	return clone;
}

Field* escape(Lab* pLab, int currX, int currY) {
	Field* moves = getMoves(pLab, currX, currY);
	Field* temp = isNextToGoal(pLab, moves);

	setVisited(pLab, currX, currY);
	usleep(DELAY);
	if (NOISY) {
		rewindOutputField();
		printOutputField(pLab);
	}

	if (temp)
		return temp;

	for (int i = 0; i < MOVE_SIZE; i++) {
		temp = (moves + i);
		if (exists(pLab, temp->x, temp->y)
				&& isAvailable(pLab, temp->x, temp->y)) {
			if (escape(pLab, temp->x, temp->y)) {
				return temp;
			}
		}
	}
	free(moves);
	return 0;
}

int bfs(Lab* pLab, Field* startField) {
	QueueElement *start = malloc(sizeof(QueueElement)), *temp_q = NULL,
			*temp_q_next = NULL;
	first_q = NULL;
	last_q = NULL;
	Field *temp_m = NULL, *temp_m_next = NULL;

	startField->value = 0;
	start->content = startField;
	start->behind = NULL;
	enqueue(start);

	do {
		temp_q = dequeue();
		temp_m = temp_q->content;

		usleep(DELAY);
		if (NOISY) {
			rewindOutputField();
			printOutputField(pLab);
		}

		int xShift[] = { 0, -1, 1, 0 };
		int yShift[] = { -1, 0, 0, 1 };
		int x, y;
		for (int i = 0; i < MOVE_SIZE; i++) {
			x = temp_m->x + xShift[i];
			y = temp_m->y + yShift[i];
			if (x > 0 && y > 0 && x < pLab->xMax && y < pLab->yMax) {
				if (pLab->fields[y * pLab->xMax + x] == ' ') {
					temp_m_next = malloc(sizeof(Field));
					temp_m_next->x = x;
					temp_m_next->y = y;
					temp_m_next->value = temp_m->value + 1;
					setVisited(pLab, temp_m_next->x, temp_m_next->y);
					temp_q_next = malloc(sizeof(QueueElement));
					temp_q_next->behind = NULL;
					temp_q_next->content = temp_m_next;
					enqueue(temp_q_next);
				} else if (pLab->fields[y * pLab->xMax + x] == 'X') {
					return temp_m->value + 1;
				}
			}
		}

		free(temp_q);
	} while (first_q);

	return 0;
}

Lab* LabSolve(Lab* pLab, int search) {
	Field* startField = getStartField(pLab), *result = NULL;
	int result_i;
	printf("Start field is %d|%d.\n", startField->x, startField->y);
	printf("Press enter to start solving...\n");
	for (int y = 0; y < pLab->yMax; y++) {
		for (int x = 0; x < pLab->xMax; x++) {
			printf("%c", *(pLab->fields + y * pLab->xMax + x));
		}
		printf("\n");
	}
	while (getchar() != '\n')
		;
	printf("\n");

	switch (search) {
	case 0: // Escape
		result = escape(pLab, startField->x, startField->y);
		if (result)
			printf("Solved.\n");
		else
			printf("Impossible.\n");
		break;
	case 1: // Greedy
		result = greedy(pLab, startField->x, startField->y);
		if (result)
			printf("Solved. Good way is %d steps.\n", result->value);
		else
			printf("Impossible.\n");
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
	return pLab;
}

