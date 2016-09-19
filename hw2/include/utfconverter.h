#ifndef _UTFCONVERTER_H_
#define _UTFCONVERTER_H_
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>
#include <string.h>
#include <stdbool.h>

/* for stderror */
#include <string.h>
/* for errno*/
#include <errno.h>
/* for file size */
#include <sys/stat.h>
#include <sys/types.h>
/* for fullpath*/
#include <limits.h>
#include <dirent.h>
/* for hostname*/
#include <netdb.h>
/* for uname*/
#include <sys/utsname.h>
#include <rpc/rpc.h>

/* fix#2: MAX_BYTES from 2 to 4*/
#define MAX_BYTES 4

/* fix#2: SURROGATE_SIZE 2 to 4, NON_SURROGATE_SIZE 1 to 2*/
#define SURROGATE_SIZE 4
#define NON_SURROGATE_SIZE 2

#define NO_FD -1
/* fix#2: OFFSET from 4 to 2*/
#define OFFSET 2

#define FIRST  0
#define SECOND 1
#define THIRD  2
#define FOURTH 3

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
#define USAGE_LENGTH 13
const char* USAGE[USAGE_LENGTH] = {
"Command line utility for converting files from UTF-16LE to UTF-16BE or vice versa.\n\n",
"Usage:\n",
"\t./utf [-h|--help] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]\n",
"\t\t-h, --help\tDisplays this usage.\n",
"\t\t-v, -vv   \tToggles the verbosity of the program to level 1 or 2.\n\n",
"\tMandatory argument:\n",
"\t\t-u OUT_ENC, --UTF=OUT_ENC\t Sets the output encoding.\n",
"\t\t\t 8\tUTF-8\n",
"\t\t\t 16LE\tUTF-16 Little Endian\n",
"\t\t\t 16BE\tUTF-16 Big Endian\n\n"
"\tPositional Arguments:\n",
"\t\tIN_FILE   \tThe file to convert.\n",
"\t\t[OUT_FILE]\tOutput file name. If not present, defaults to stdout.\n"
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
		{"verbose", no_argument, 0, 'v'},
		{"UTF", required_argument, 0, 'u'},
		{0, 0, 0, 0}
};

/* my constants */
const int ENDIAN_MAX_LENGTH=5;

/* my variables*/
int opt_v = 0;
int opt_u = 0;
#endif
