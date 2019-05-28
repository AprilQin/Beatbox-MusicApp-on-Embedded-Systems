#include "file.h"

int readFromFile(char file[])
{
	//creat a buff
	const int max_length = 1024;
	char buff[max_length]; 

	//open the file
	FILE *f = fopen(file, "r");
	if (f == NULL)
		printf("file opening error\n");
	
	//read the return the value
	fgets(buff, max_length, f); 
	fclose(f);
	return atoi(buff);	
}

void writeToFile(char file[], char value[])
{
	//open file 
	FILE *f = fopen(file, "w");
	if (f == NULL)
		printf("file opening error\n");

	//write to file
	int charWritten = fprintf(f, "%s", value);
	if (charWritten <= 0 ){
		printf("file writig error\n");
		//exit(-1);
	}

	//close file
	fclose(f);
}
