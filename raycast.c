#include <ctype.h>
#include <math.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>


typedef struct object {
	// type will be a value used to point into the union below
	// 0 = camera, 1 = sphere, 2 = plane
	char *type;
	double color[3];

	union {

		struct {
			double width;
			double height;
		} camera;

		struct {
			double position[3];
			double radius;
		} sphere;

		struct {
			double position[3];
			double normal[3];
		} plane;
	
	};
} object;


typedef struct objectBuffer {
	object *objectArray;
	size_t objectCount;
} objectBuffer;


typedef struct ray {
	double origin[3];
	double direction[3];
} ray;


typedef struct imageData {
	double width;
	double height;
	char *color;
} imageData;


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


// check_arguments is a function that checks for any errors in the command line arguments
// if an error is encountered, throws an appropriate error and exits the program
void check_arguments(int argc, char *argv[]) {
	// check for enough command line arguments
	if (argc != 5) {
		fprintf(stderr, "Error: Incorrect number of arguments (must be exactly 4).\n");
		exit(1);
	}

	// check that provided width is valid
	char *width_non_digits;
	int width = strtol(argv[1], &width_non_digits, 10);
	// check that width is a number, and nothing else
	if (strcmp(width_non_digits, "") != 0) {
		fprintf(stderr, "Error: Invalid width argument (must be a number).\n");
		exit(1);
	}
	// check that width is greater than 0
	if (width <= 0) {
		fprintf(stderr, "Error: Invalid width value (must be greater than 0).\n");
		exit(1);
	}

	// check that provided height is valid
	char *height_non_digits;
	int height = strtol(argv[2], &height_non_digits, 10);
	// check that height is a number, and nothing else
	if (strcmp(height_non_digits, "") != 0) {
		fprintf(stderr, "Error: Invalid height argument (must be a number).\n");
		exit(1);
	}
	// check that height is greater than 0
	if (height <= 0) {
		fprintf(stderr, "Error: Invalid height value (must be greater than 0).\n");
		exit(1);
	}

	// check that the input file extension is .json
	unsigned char input_name[30];
	strcpy(input_name, argv[3]);
	if (check_file_extension(input_name, ".json") != true) {
		fprintf(stderr, "Error: Invalid input filename (must have .json extension).\n");
		exit(1);
	}

	FILE* input = fopen(input_name, "r");

	// check that input file exists
	if (!input) {
		fprintf(stderr, "Error: Invalid input filename (file does not exist).\n");
		fclose(input);
		exit(1);
	}

	// check that the output file extension is .ppm
	unsigned char output_name[30];
	strcpy(output_name, argv[4]);
	if (check_file_extension(output_name, ".ppm") != true) {
		fprintf(stderr, "Error: Invalid output filename (must have .ppm extension).\n");
		fclose(input);
		exit(1);
	}

	fclose(input);
	return;
}


int line = 1; // global unpreferable, but simplifies reading the json file


// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json) {
  int c = fgetc(json);
  
  #ifdef DEBUG
    printf("next_c: '%c'\n", c);
  #endif
  
  if (c == '\n') {
    line += 1;
  }
  
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  
  return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* json, int d) {
  int c = next_c(json);
  if (c != d) {
    fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
    exit(1);
  }
  
  return;  
}


// skip_ws() skips white space in the file.
void skip_ws(FILE* json) {
  int c = next_c(json);
  
  while (isspace(c)) {
    c = next_c(json);
  }
  
  ungetc(c, json);
}


// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json) {
  char buffer[129];
  int c = next_c(json);
  
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }  
  
  c = next_c(json);
  
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);      
    }
  
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);      
    }
  
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
  
    buffer[i] = c;
    i += 1;
    c = next_c(json);
  }
  
  buffer[i] = 0;
  return strdup(buffer);
}


double next_number(FILE* json) {
  double value;
  fscanf(json, "%lf", &value);
  // Error check this..
  return value;
}


double* next_vector(FILE* json) {
  double* v = malloc(3*sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  
  v[0] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  
  v[1] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  
  v[2] = next_number(json);
  skip_ws(json);
  expect_c(json, ']');
  
  return v;
}


void read_scene(char* filename) {
  int c;
  FILE* json = fopen(filename, "r");

  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }
  
  skip_ws(json);
  
  // Find the beginning of the list
  expect_c(json, '[');

  skip_ws(json);

  // Find the objects

  while (1) {
    c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
      fclose(json);
      return;
    }

    if (c == '{') {
      skip_ws(json);
    
      // Parse the object
      char* key = next_string(json);
      if (strcmp(key, "type") != 0) {
        fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
        exit(1);
      }

      skip_ws(json);
      expect_c(json, ':');
      skip_ws(json);

      char* value = next_string(json);

      if (strcmp(value, "camera") == 0) {
        fprintf(stdout, "value is: %s\n", value);
        // draw a camera (?)
      } else if (strcmp(value, "sphere") == 0) {
        fprintf(stdout, "value is: %s\n", value);
        // draw a sphere
      } else if (strcmp(value, "plane") == 0) {
        fprintf(stdout, "value is: %s\n", value);
        // draw a plane
      } else {
        fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
        exit(1);
      }

      skip_ws(json);

      while (1) {
        c = next_c(json);
        if (c == '}') {
          // stop parsing this object
          break;
        } else if (c == ',') {
          // read another field
          skip_ws(json);
          char* key = next_string(json);
          skip_ws(json);
          expect_c(json, ':');
          skip_ws(json);

          if ((strcmp(key, "width") == 0) || (strcmp(key, "height") == 0) || (strcmp(key, "radius") == 0)) {
            double value = next_number(json);

            // check that value is a valid number
            if (value <= 0) {
              fprintf(stderr, "Error: Invalid %s value (must be greater than 0).\n", key);
              exit(1);
            }

          } else if ((strcmp(key, "color") == 0) || (strcmp(key, "position") == 0) || (strcmp(key, "normal") == 0)) {
            double* value = next_vector(json);

          } else {
            fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n", key, line);
            exit(1);
            //char* value = next_string(json);
          }
          
          skip_ws(json);
        
        } else {
          fprintf(stderr, "Error: Unexpected value on line %d\n", line);
          exit(1);
        }
      }
      
      skip_ws(json);
      c = next_c(json);
      if (c == ',') {
        // noop
        skip_ws(json);
      } else if (c == ']') {
        fclose(json);
        return;
      } else {
        fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
        exit(1);
      }
    }
  }
}


int main(int argc, char *argv[]) {
	check_arguments(argc, argv);

	//FILE *json = fopen(argv[3], "r");

	char *filename = argv[3];

	read_scene(filename);

	fprintf(stdout, "Hey hey! The program didn't crash!\n");
	return(0);
}