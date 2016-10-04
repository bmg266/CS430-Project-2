#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>

// a helper function to check the extension of a filename
bool check_file_extension(char filename[], char sought_ext[]) {
  int filename_length = strlen(filename);
  int extension_length = strlen(sought_ext);

  unsigned char filename_ext[6] = {'\0'};
  
  int m = 0;
  
  // copy from nth-to-last char of the given filename up to last char into an array
  // where n is the length of the extension we are checking for
  for (int i = filename_length - extension_length; i < filename_length; i++) {
    filename_ext[m] = filename[i];
    m++;
  }
  
  return(strcmp(filename_ext, sought_ext) == 0);
}

int main(int argc, char *argv[]) {

	// check for enough command line arguments
	if (argc != 5) {
		fprintf(stderr, "Error: Incorrect number of arguments (must be exactly 4).\n");
		return(1);
	}

	// check that provided width is valid
	char *width_non_digits;
	int width = strtol(argv[1], &width_non_digits, 10);
	// check that width is a number, and nothing else
	if (strcmp(width_non_digits, "") != 0) {
		fprintf(stderr, "Error: Invalid width argument (must be a number).\n");
		return(1);
	}
	// check that width is greater than 0
	if (width <= 0) {
		fprintf(stderr, "Error: Incorrect width value (must be greater than 0).\n");
		return(1);
	}

	// check that provided height is valid
	char *height_non_digits;
	int height = strtol(argv[2], &height_non_digits, 10);
	// check that height is a number, and nothing else
	if (strcmp(height_non_digits, "") != 0) {
		fprintf(stderr, "Error: Invalid height argument (must be a number).\n");
		return(1);
	}
	// check that height is greater than 0
	if (height <= 0) {
		fprintf(stderr, "Error: Incorrect height value (must be greater than 0).\n");
		return(1);
	}

	// check that the input file extension is .json
	unsigned char input_name[30];
	strcpy(input_name, argv[3]);
	if (check_file_extension(input_name, ".json") != true) {
		fprintf(stderr, "Error: Invalid input filename (must have .json extension).\n");
		return(1);
	}

	FILE* input = fopen(input_name, "r");

	// check that input file exists
	if (!input) {
		fprintf(stderr, "Error: Invalid input filename (file does not exist).\n");
		fclose(input);
		return(1);
	}

	// check that the output file extension is .ppm
	unsigned char output_name[30];
	strcpy(output_name, argv[4]);
	if (check_file_extension(output_name, ".ppm") != true) {
		fprintf(stderr, "Error: Invalid output filename (must have .ppm extension).\n");
		fclose(input);
		return(1);
	}

	fprintf(stdout, "Hey hey! The program didn't crash!\n");
	return(0);
}