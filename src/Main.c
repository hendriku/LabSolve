#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#define MAX_SIZE 50

typedef struct {
	int x, y;
} Field;

typedef struct {
	char* fields;
	int xMax;
	int yMax;
} Lab;

Lab* LabRead(FILE*);
Lab* LabSolve(Lab*);
Field* getStartField(Lab*);
Field* getFieldToTarget(Lab*, int, int);
int getLabWidth(FILE*);
int getLabHeight(FILE*);
void initField(Lab*);
void printOutputField(Lab*);

int main(int argc, char* argv[]) {
	FILE* in = stdin;
	Lab* pLab;
	if (argc > 2) {
		fprintf(stderr, "Usage: %s [<file>]\n", argv[0]);
		return 1;
	}
	if (argc == 2) {
		in = fopen(argv[1], "r");
		if (!in) {
			perror(argv[0]);
			return 1;
		}
	}
	pLab = LabRead(in);

	pLab = LabSolve(pLab);

	free(pLab->fields);
	fclose(in);
	return EXIT_SUCCESS;
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
			*(pLab->fields + y * pLab->yMax + x) = '.';
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
			printf("%c", *(pLab->fields + y * pLab->yMax + x));
		}
		printf("\n");
	}
}

Field* getStartField(Lab* pLab) {
	Field* field = malloc(sizeof(Field));
	for (int y = 0; y < pLab->yMax; y++) {
		for (int x = 0; x < pLab->xMax; x++) {
			if (*(pLab->fields + y * pLab->yMax + x) == 'S') {
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
			*(pLab->fields + y * pLab->yMax + x) = temp;
			x++;
		}
	}
	rewind(file);

	return pLab;
}

Field* getMoves(Lab* pLab, int currX, int currY) {
	Field* fields = malloc(sizeof(Field) * 8);
	Field* temp;
	int xShift[] = { -1, 0, 1, -1, 1, -1, 0, 1 };
	int yShift[] = { -1, -1, -1, 0, 0, 1, 1, 1 };
	for (int i = 0; i < 8; i++) {
		temp = fields + i;
		temp->x = currX + xShift[i];
		temp->y = currY + yShift[i];
	}
	return fields;
}

bool exists(Lab* pLab, int x, int y) {
	return x > 0 && y > 0 && x < pLab->xMax && y < pLab->yMax;
}

bool isAvailable(Lab* pLab, int x, int y) {
	return *(pLab->fields + pLab->yMax * y + x) == ' ';
}

bool isGoal(Lab* pLab, int x, int y) {
	return *(pLab->fields + pLab->yMax * y + x) == 'X';
}

void setVisited(Lab* pLab, int x, int y) {
	*(pLab->fields + y * pLab->yMax + x) = '.';
}

Field* isNextToGoal(Lab* pLab, Field* moves) {
	Field* temp;
	for (int i = 0; i < 8; i++) {
		temp = (moves + i);
		if (isGoal(pLab, temp->x, temp->y)) {
			return temp;
		}
	}
	return 0;
}

Field* getFieldToTarget(Lab* pLab, int currX, int currY) {
	Field* moves = getMoves(pLab, currX, currY);
	Field* temp = isNextToGoal(pLab, moves);

	setVisited(pLab, currX, currY);
	usleep(20000);
	rewindOutputField(pLab);
	printOutputField(pLab);

	if (temp) return temp;

	for (int i = 0; i < 8; i++) {
		temp = (moves + i);
		if (exists(pLab, temp->x, temp->y) && isAvailable(pLab, temp->x, temp->y)) {
			if (getFieldToTarget(pLab, temp->x, temp->y)) {
				return temp;
			}
		}
	}
	free(moves);
	return 0;
}

Lab* LabSolve(Lab* pLab) {
	Field* startField = getStartField(pLab);
	printf("Start field is %d|%d.\n", startField->x, startField->y);
	printf("Press enter to start solving...\n");
	for (int y = 0; y < pLab->yMax; y++) {
		for (int x = 0; x < pLab->xMax; x++) {
			printf("%c", *(pLab->fields + y * pLab->yMax + x));
		}
		printf("\n");
	}
	while (getchar() != '\n')
		;
	printf("\n");
	if (getFieldToTarget(pLab, startField->x, startField->y))
		printf("Solved.\n");
	else
		printf("Impossible.\n");
	free(startField);
	return pLab;
}


