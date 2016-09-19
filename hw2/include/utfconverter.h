#ifndef _UTFCONVERTER_H_
#define _UTFCONVERTER_H_
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

// for stderror
#include <string.h>
// for errno
#include <errno.h>

// fix#2: MAX_BYTES from 2 to 4
#define MAX_BYTES 4

// fix#2: SURROGATE_SIZE 2 to 4, NON_SURROGATE_SIZE 1 to 2
#define SURROGATE_SIZE 4
#define NON_SURROGATE_SIZE 2

#define NO_FD -1
// fix#2: OFFSET from 4 to 2
#define OFFSET 2

#define FIRST  10000000
#define SECOND 20000000
#define THIRD  30000000
#define FOURTH 40000000

#ifdef __STDC__
#define P(x) x
#else
#define P(x) ()
#endif

/** The enum for endianness. */
typedef enum {LITTLE, BIG} endianness;

/** The struct for a codepoint glyph. */
typedef struct Glyph {
	unsigned char bytes[MAX_BYTES];
	endianness end;
	bool surrogate;
} Glyph;

/** The given filename. */
extern char* filename;

/** The usage statement. */
const char* USAGE[4] = { 
"Usage:  ./utfconverter FILENAME [OPTION]\n\t",
"./utfconverter -h\t\t\tDisplays this usage statement.\n\t",
"./utfconverter --help\t\t\tDisplays this usage statement.\n\t"
"./utfconverter --UTF-16=ENDIANNESS\tEndianness to convert to.\n"
};

/** Which endianness to convert to. */
extern endianness conversion;

/** Which endianness the source file is in. */
extern endianness source;

/**
 * A function that swaps the endianness of the bytes of an encoding from
 * LE to BE and vice versa.
 *
 * @param glyph The pointer to the glyph struct to swap.
 * @return Returns a pointer to the glyph that has been swapped.
 */
Glyph* swap_endianness (Glyph* glyph);

/**
 * Fills in a glyph with the given data in data[2], with the given endianness 
 * by end.
 *
 * @param glyph 	The pointer to the glyph struct to fill in with bytes.
 * @param data[2]	The array of data to fill the glyph struct with.
 * @param end	   	The endianness enum of the glyph.
 * @param fd 		The int pointer to the file descriptor of the input 
 * 			file.
 * @return Returns a pointer to the filled-in glyph.
 */
Glyph* fill_glyph (Glyph*, unsigned int data[2], endianness end, int* fd);

/**
 * Writes the given glyph's contents to stdout.
 *
 *@param glyph The pointer to the glyph struct to write to stdout.
 */
void write_glyph (Glyph*);

/**
 * Calls getopt() and parses arguments.
 *
 * @param argc The number of arguments.
 * @param argv The arguments as an array of string.
 */
void parse_args (int argc, char** argv);

/**
 * Prints the usage statement.
 */
void print_help (void);

/**
 * Closes file descriptors and frees list and possibly does other
 * bookkeeping before exiting.
 *
 * @param The fd int of the file the program has opened. Can be given
 * the macro value NO_FD (-1) to signify that we have no open file
 * to close.
 */
void quit_converter(int fd);

static struct option long_options[] = {
		{"help", no_argument, 0, 'h'},
		{"h", no_argument, 0, 'h'},
		{0, 0, 0, 0}
};

// my constants
const int ENDIAN_MAX_LENGTH=5;

#endif
