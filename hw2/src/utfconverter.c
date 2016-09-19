#include "utfconverter.h"
#define LITTLE_ENDIAN_SYSTEM
char* filename;
endianness source;
endianness conversion;

int main(int argc, char** argv)
{
	unsigned int buf[2];
	int fd, rv;
	char chr[2];
	Glyph* glyph;

	/* After calling parse_args(), filename and conversion should be set. */
	parse_args(argc, argv);

	fd = open(filename, O_RDONLY);
	if(fd == NO_FD){
		fprintf(stderr, "%s: %s\n", "File not opened: ", strerror(errno));
		quit_converter(NO_FD);
	}

	glyph = (Glyph*) malloc(sizeof(Glyph) + 1); 
	glyph = memset(glyph, 0, sizeof(Glyph)+1);
	
	/* Handle BOM bytes for UTF16 specially. 
         * Read our values into the first and second elements. */
	rv = 0;
	buf[0] = 0;
	buf[1] = 0;
	if((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1) { 
		#ifndef LITTLE_ENDIAN_SYSTEM
		buf[0] = buf[0] >> 24;
		buf[1] = buf[1] >> 24;
		#endif

		glyph->bytes[FIRST] = buf[0];
		glyph->bytes[SECOND] = buf[1];

		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is big endian*/
			source = BIG; 
		} else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is little endian*/
			source = LITTLE;
		} else {
			/*file has no BOM*/

			fprintf(stderr, "File has no BOM. buf[%d]=%x, buf[%d]=%x\n", 0, buf[0], 1, buf[1]);
			free(glyph);
			quit_converter(fd);
		}

		/* [DEBUG] */
		/* printf("Done with finding BOM\n"); */

		/* for writing glyph */
		if(source != conversion){
			swap_endianness(glyph);
		}
		write_glyph(glyph);

		void* memset_return = memset(glyph, 0, sizeof(Glyph)+1);
		
		/* Memory write failed, recover from it: */
		if(memset_return == NULL){
		/* tweak write permission on heap memory. */
			//asm("movl $8, %esi\n\t"
			//    "movl $.LC0, %edi\n\t"
			//    "movl $0, %eax");
			/* Now make the request again. */
			 memset(glyph, 0, sizeof(Glyph)+1);
		}
	}

	/* [DEBUG] */
	/* printf("Done with first if statement\n"); */
	/*char tmp;
	while((rv=read(fd, &tmp, 1))==1){
		printf("%c\n", tmp);
	}*/

	/* Now deal with the rest of the bytes.*/
	while((rv = read(fd, &chr, 2)) == 2){
		buf[0] = chr[0] & 0xff;
		buf[1] = chr[1] & 0xff;

		/* [DEBUG] */
		// printf("Inside while loop: 0x%02x 0x%02x\n", buf[0], buf[1]);

		fill_glyph(glyph, buf, source, &fd);

		/* [DEBUG] */
		// fprintf(stderr, "source_end = %u, conversion_end = %u", source, conversion);
		if(source != conversion){
			swap_endianness(glyph);
		}

		write_glyph(glyph);

		void* memset_return = memset(glyph, 0, sizeof(Glyph)+1);

	        /* Memory write failed, recover from it: */
	         if(memset_return == NULL){
		        /* tweak write permission on heap memory. */
		        //asm("movl $8, %esi\n\t"
		        //    "movl $.LC0, %edi\n\t"
		        //    "movl $0, %eax");
		        /* Now make the request again. */
		        memset(glyph, 0, sizeof(Glyph)+1);
	        }
	}

	free(glyph);
	quit_converter(fd);
	return 0;
}

Glyph* swap_endianness(Glyph* glyph)
{
	unsigned char tmp;

	tmp = glyph->bytes[FIRST];
	/* Use XOR to be more efficient with how we swap values. */
	glyph->bytes[FIRST] = glyph->bytes[SECOND];
	glyph->bytes[SECOND] = tmp;
	if(glyph->surrogate){  /* If a surrogate pair, swap the next two bytes. */
		tmp = glyph->bytes[THIRD];
		glyph->bytes[THIRD] = glyph->bytes[FOURTH];
		glyph->bytes[FOURTH] = tmp;
	}
	glyph->end = conversion;
	return glyph;
}

// fix#2: changed FIRST SECOND THIRD FOURTH to indices
Glyph* fill_glyph(Glyph* glyph, unsigned int data[2], endianness end, int* fd)
{
	unsigned int bits;
	glyph->bytes[FIRST] = data[0];
	glyph->bytes[SECOND] = data[1];

	// fix#2: change '0' to 0
	//unsigned int bits = '0';
	bits = 0;
	bits |= (data[0] + (data[1] << 8));


	/* Check high surrogate pair using its special value range.*/
	// fix#2: changed the range of none sur pair
	// if(bits > 0x000F && bits < 0xF8FF){
	if((bits <= 0xD7FF) || (bits >= 0xE000 && bits <= 0xFFFF))
	{
		// This block means no surrogate pair
		glyph->surrogate = false; 

	} else {
		// This block means yes surrogate pair
		if(read(*fd, &data[0], 1) == 1 && read(*fd, &(data[1]), 1) == 1)
		{
			#ifndef LITTLE_ENDIAN_SYSTEM
			data[0] = data[0] >> 24;
			data[1] = data[1] >> 24;
			#endif
			// fix#2
			//bits = '0'; /* bits |= (bytes[FIRST] + (bytes[SECOND] << 8)) */
			bits = 0; /* bits |= (bytes[FIRST] + (bytes[SECOND] << 8)) */
			bits |= (data[0] + (data[1] << 8));
		
			glyph->surrogate = true;
			//if(bits > 0xDAAF && bits < 0x00FF){ /* Check low surrogate pair.*/
			//} else {
			//	lseek(*fd, -OFFSET, SEEK_CUR); 
			//}
		}
	}

	if(!glyph->surrogate){
		glyph->bytes[THIRD] = 0;
		glyph->bytes[FOURTH] = 0;
	} else {
		glyph->bytes[THIRD] = data[0]; 
		glyph->bytes[FOURTH] = data[1];
	}

	glyph->end = end;

	/* [DEBUG] */
//	fprintf(stderr, "\nbytes={0x%02x, 0x%02x, 0x%02x, 0x%02x}", glyph->bytes[0], glyph->bytes[1], glyph->bytes[2], glyph->bytes[3]);
	return glyph;
}

void write_glyph(Glyph* glyph)
{
	if(glyph->surrogate){
		write(STDOUT_FILENO, glyph->bytes, SURROGATE_SIZE);
		
	} else {
		write(STDOUT_FILENO, glyph->bytes, NON_SURROGATE_SIZE);
	}
}

void parse_args(int argc, char** argv)
{
	int option_index, c;
	char* endian_convert;

	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	// fix#1: malloc space for endian_convert
	endian_convert = (char*) malloc(ENDIAN_MAX_LENGTH * sizeof(char));
	// fix#1: added ':' after "hu"
	if((c = getopt_long(argc, argv, "hu:", long_options, &option_index)) != -1)
	{
		switch(c){ 
			case 'u':
				endian_convert = strncpy(endian_convert, optarg, ENDIAN_MAX_LENGTH);

				/* [DEBUG] */
				// fprintf(stderr, "endian_convert is : %s\n", endian_convert);

				break;
			default:
				fprintf(stderr, "Unrecognized argument.: %c\n", c);
				free(endian_convert);
				quit_converter(NO_FD);
				break;
		}
	}
	
	/* [DEBUG] */
	// printf("filename? argv[%d]=%s\n", optind, argv[optind]);

	if(optind < argc){
		// fix#1: allocate memory for filenameA
		filename = (char*) malloc(strlen(argv[optind]) + 1);
		filename = (char*) memset(filename, 0, strlen(argv[optind])+1);
		strncpy(filename, argv[optind], strlen(argv[optind]));
	} else {
		fprintf(stderr, "Filename not given.\n");
		free(endian_convert);
		print_help();
	}

	if(endian_convert == NULL){
		fprintf(stderr, "Converson mode not given.\n");
		free(endian_convert);
		print_help();
	}

	if(strcmp(endian_convert, "16LE")){ 
		conversion = LITTLE;
	} else if(strcmp(endian_convert, "16BE")){
		conversion = BIG;
	} else {
		free(endian_convert);
		quit_converter(NO_FD);
	}
	free(endian_convert);
}

void print_help(void) {
	int i;
	for(i = 0; i < 4; i++){
		printf("%s", USAGE[i]); 
	}
	quit_converter(NO_FD);
}

void quit_converter(int fd)
{
	free(filename);

	close(STDERR_FILENO);
	close(STDIN_FILENO);
	close(STDOUT_FILENO);
	if(fd != NO_FD)
		close(fd);
	exit(0);
	/* Ensure that the file is included regardless of where we start compiling from. */
}
