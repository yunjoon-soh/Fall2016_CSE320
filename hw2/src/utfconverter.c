#include "utfconverter.h"
#define LITTLE_ENDIAN_SYSTEM
char* filename;
endianness source;
endianness conversion;

struct rusage usage;
struct timeval user_Start, user_End, sys_Start, sys_End;

clock_t R_Start, R_End, C_Start, C_End, W_Start, W_End;
double real_R_Time, real_C_Time, real_W_Time;
double user_R_Time, user_C_Time, user_W_Time;
double sys_R_Time, sys_C_Time, sys_W_Time;

int main(int argc, char** argv)
{
	unsigned int buf[2];
	int fd, rv;
	Glyph* glyph;
	struct stat statbuf;
	char* fullpath;
	char* hostname;
	struct utsname osname;

	/* After calling parse_args(), filename and conversion should be set. */
	parse_args(argc, argv);

	fd = open(filename, O_RDONLY);
	if(fd == NO_FD){
		fprintf(stderr, "%s: %s\n", "File not opened: ", strerror(errno));
		quit_converter(NO_FD);
	}

	if(opt_v >= 1){
		fstat(fd, &statbuf);
		fprintf(stderr, "Input file size: %lu bytes\n", statbuf.st_size);

		fullpath = (char*) malloc(PATH_MAX + 1);
		realpath(filename, fullpath);
		fprintf(stderr, "Input file path: %s\n", fullpath);
		free(fullpath);
	}

	glyph = (Glyph*) malloc(sizeof(Glyph) + 1); 
	glyph = memset(glyph, 0, sizeof(Glyph)+1);
	
	/* Handle BOM bytes for UTF16 specially. 
         * Read our values into the first and second elements. */
	rv = 0;
	buf[0] = 0;
	buf[1] = 0;
	real_R_Time = 0;
	real_C_Time = 0;
	real_W_Time = 0;

	R_Start = clock();
	rusage_start();

	if((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1) { 
		R_End = clock();
		real_R_Time += (R_End - R_Start);
		rusage_end(READ);

		/* CONVERT START */
		C_Start = clock();
		rusage_start();
		#ifndef LITTLE_ENDIAN_SYSTEM
		buf[0] = buf[0] >> 24;
		buf[1] = buf[1] >> 24;
		#endif

		glyph->bytes[FIRST] = buf[0];
		glyph->bytes[SECOND] = buf[1];

		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is little endian*/
			source = LITTLE;
		} else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is big endian*/
			source = BIG; 
		} else {
			/*file has no BOM*/
			fprintf(stderr, "File has no BOM. buf[%d]=%x, buf[%d]=%x\n", 0, buf[0], 1, buf[1]);
			free(glyph);
			quit_converter(fd);
		}

		/* for writing glyph */
		if(source != conversion){
			swap_endianness(glyph);
		}
		C_End = clock();
		real_C_Time += (C_End - C_Start);
		rusage_end(CONVERT);
		/* CONVERT END */

		/* WRITE START */
		W_Start = clock();
		rusage_start();

		write_glyph(glyph);

		W_End = clock();
		real_W_Time = (W_End - W_Start);
		rusage_end(WRITE);
		/* WRITE END */

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
	
	if(opt_v >= 1){
		fprintf(stderr, "Input file encoding: %s\n", (source==0)?"UTF16-LE":"UTF16-BE");
		fprintf(stderr, "Output file encoding: %s\n", (conversion==0)?"UTF16-LE":"UTF16-BE");
		
		hostname = (char*)malloc(MAXHOSTNAMELEN + 1);
		gethostname(hostname, MAXHOSTNAMELEN + 1);
		fprintf(stderr, "Hostmachine: %s\n", hostname);
		free(hostname);

		//osname = (char*) malloc(sysconf(_SC_HOST_NAME_MAX + 1));
		uname(&osname);
		fprintf(stderr, "Operating System: %s\n", osname.sysname);
	}

	/* [DEBUG] */
	/* printf("Done with first if statement\n"); */
	/*char tmp;
	while((rv=read(fd, &tmp, 1))==1){
		printf("%c\n", tmp);
	}*/

	/* Now deal with the rest of the bytes.*/
	R_Start = clock();
	rusage_start();
	while((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1) { 
		R_End = clock();
		real_R_Time += (R_End - R_Start);
		rusage_end(READ);

		C_Start = clock();
		rusage_start();

		fill_glyph(glyph, buf, source, &fd);

		if(source == conversion){
			// fprintf(stderr, "Source=%d, Conversion=%d swap started...\n", source, conversion);
			swap_endianness(glyph);
		}

		C_End = clock();
		real_C_Time += (C_End - C_Start);
		rusage_end(CONVERT);

		W_Start = clock();
		rusage_start();
		write_glyph(glyph);
		W_End = clock();
		real_W_Time = (W_End - W_Start);
		rusage_end(WRITE);

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

	if(opt_v >= 2){
		fprintf(stderr, "Reading: real=%f, user=%f, sys=%f\n", real_R_Time, user_R_Time, sys_R_Time);
		fprintf(stderr, "Converting: real=%f, user=%f, sys=%f\n", real_C_Time, user_C_Time, sys_C_Time);
		fprintf(stderr, "Writing: real=%f, user=%f, sys=%f\n", real_W_Time, user_W_Time, sys_W_Time);
		fprintf(stderr, "ASCII: %f%%\n", 100*(((double)ASCII_cnt)/((double)glyph_cnt)));
		fprintf(stderr, "Surrogates: %f%%\n", 100*(((double)surrogate_cnt)/((double)glyph_cnt)));
		fprintf(stderr, "Glyphs: %d\n", glyph_cnt);
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

	glyph->bytes[FIRST] = data[1];
	glyph->bytes[SECOND] = data[0];

	// fix#2: change '0' to 0
	//unsigned int bits = '0';
	bits = 0;
	if(end == LITTLE)
		bits |= ((glyph->bytes[FIRST] << 8) + glyph->bytes[SECOND]);
	else
		bits |= ((glyph->bytes[SECOND] << 8) + glyph->bytes[FIRST]);

	/* [DEBUG] */
	// fprintf(stderr, "\nFill_glyph, end is %d\n", end);
	// fprintf(stderr, "Address of data[%d]=%08x, val=%08x\n", 0, (int) &data[0], data[0]);
	// fprintf(stderr, "Address of data[%d]=%08x, val=%08x\n", 1, (int) &data[1], data[1]);
	// fprintf(stderr, "Bits: %08x\n", bits);


	/* Check high surrogate pair using its special value range.*/
	// fix#2: changed the range of none sur pair
	// if(bits > 0x000F && bits < 0xF8FF){
	if((bits <= 0xD7FF) || (bits >= 0xE000 && bits <= 0xFFFF))
	{
		// This block means no surrogate pair
		glyph->surrogate = false; 
		if(bits <= 0x007f){
			ASCII_cnt++;
		}

	} else {

		// This block means yes surrogate pair
		if(read(*fd, &data[0], 1) == 1 && read(*fd, &(data[1]), 1) == 1)
		{
			#ifndef LITTLE_ENDIAN_SYSTEM
			data[0] = data[0] >> 24;
			data[1] = data[1] >> 24;
			#endif

			glyph->bytes[THIRD] = data[1];
			glyph->bytes[FOURTH] = data[0];

			/* [DEBUG] */
			// fprintf(stderr, "Address of data[%d]=%08x, val=%08x\n", 0, (int) &data[0], data[0]);
			// fprintf(stderr, "Address of data[%d]=%08x, val=%08x\n", 1, (int) &data[1], data[1]);
			// fprintf(stderr, "Bits: %08x\n", bits);

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
		surrogate_cnt++;
	}

	glyph->end = end;
	glyph_cnt++;
	
	/* [DEBUG] */
	// fprintf(stderr, "bytes={0x%02x, 0x%02x, 0x%02x, 0x%02x} isLitte(0 if little)=%d\n", glyph->bytes[FIRST], glyph->bytes[SECOND], glyph->bytes[THIRD], glyph->bytes[FOURTH], glyph->end);
	return glyph;
}

void write_glyph(Glyph* glyph)
{
	write(STDOUT_FILENO, &glyph->bytes[FIRST], 1);
	write(STDOUT_FILENO, &glyph->bytes[SECOND], 1);

	if(glyph->surrogate){
		write(STDOUT_FILENO, &glyph->bytes[THIRD], 1);
		write(STDOUT_FILENO, &glyph->bytes[FOURTH], 1);
	}
	// if(glyph->surrogate){
	// 	write(STDOUT_FILENO, glyph->bytes, 4);
	// }
	// else {
	// 	write(STDOUT_FILENO, glyph->bytes, 2);
	// }
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
	while((c = getopt_long(argc, argv, "hu:v", long_options, &option_index)) != -1)
	{
		switch(c){ 
			case 'h':
				print_help();
				free(endian_convert);
				quit_converter(NO_FD);
				break;
			case 'u':
				opt_u++;
				endian_convert = strncpy(endian_convert, optarg, ENDIAN_MAX_LENGTH);

				/* [DEBUG] */
				// fprintf(stderr, "endian_convert is : %s\n", endian_convert);

				break;
			case 'v':
				opt_v++;
				break;
			case '?':
				print_help();
				free(endian_convert);
				quit_converter(NO_FD);
				break;
			default:
				fprintf(stderr, "%s: %s=%c\n", "Unrecognized argument.\n", strerror(errno), c);
				free(endian_convert);
				quit_converter(NO_FD);
				break;
		}
	}

	if(opt_u != 1){
		if(opt_u > 1){
			fprintf(stderr, "Too many UTF options set\n");
		} else if(opt_u == 0){
			fprintf(stderr, "Required argument not passed: -u or --UTF=OUT_ENC\n");
		}
		free(endian_convert);
		quit_converter(NO_FD);
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

	if(strcmp(endian_convert, "16LE") == 0){ 
		conversion = LITTLE;
	} else if(strcmp(endian_convert, "16BE") == 0){
		conversion = BIG;
	} else {
		free(endian_convert);
		quit_converter(NO_FD);
	}
	free(endian_convert);
}

void print_help(void) {
	int i;
	for(i = 0; i < USAGE_LENGTH; i++){
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

void rusage_start(void){
	int ret;
	ret = getrusage(RUSAGE_SELF, &usage);

	if(ret == -1)
		fprintf(stderr, "%s: %s\n", "getrusage() failed", strerror(errno));

	user_Start = usage.ru_utime;
	sys_Start = usage.ru_stime;
}

void rusage_end(measure m){
	int ret;

	ret = getrusage(RUSAGE_SELF, &usage);

	if(ret == -1)
		fprintf(stderr, "%s: %s\n", "getrusage() failed", strerror(errno));

	user_End = usage.ru_utime;
	sys_End = usage.ru_stime;

	switch(m){
		case READ:
			user_R_Time += ((user_End.tv_sec * 1000000 + user_End.tv_usec) - (user_Start.tv_sec * 1000000 + user_Start.tv_usec));
			sys_R_Time += ((sys_End.tv_sec * 1000000 + sys_End.tv_usec) - (sys_Start.tv_sec * 1000000 + sys_Start.tv_usec)) ;
			break;
		case CONVERT:
			user_C_Time += ((user_End.tv_sec * 1000000 + user_End.tv_usec) - (user_Start.tv_sec * 1000000 + user_Start.tv_usec));
			sys_C_Time += ((sys_End.tv_sec * 1000000 + sys_End.tv_usec) - (sys_Start.tv_sec * 1000000 + sys_Start.tv_usec)) ;
			break;
		case WRITE:
			user_W_Time += ((user_End.tv_sec * 1000000 + user_End.tv_usec) - (user_Start.tv_sec * 1000000 + user_Start.tv_usec));
			sys_W_Time += ((sys_End.tv_sec * 1000000 + sys_End.tv_usec) - (sys_Start.tv_sec * 1000000 + sys_Start.tv_usec)) ;
			break;
	}

	/* [DEBUG] */
	//fprintf(stderr,"This is user_End: %lu, %lu\n", user_End.tv_sec, user_End.tv_usec);
	//fprintf(stderr, "This is sys_End: %lu, %lu\n", sys_End.tv_sec, sys_End.tv_usec);
}
