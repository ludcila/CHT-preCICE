#include "Helpers.h"

char * concat(char * prefix, char * string, char * suffix) {
	int nameLength = strlen(string) + strlen(prefix) + strlen(suffix);
	char * result = malloc(nameLength); // TODO: free memory somewhere?
	strcpy(result, prefix);
	strcat(result, string);
	strcat(result, suffix);
	return result;
}
