#include "utfconverter.h"
#include "filemanager.h"

char* filename, *outfilename;
endianness source, conversion;
int infile_fd, outfileFd;
double real_R_Time, real_C_Time, real_W_Time;
double user_R_Time, user_C_Time, user_W_Time;
double sys_R_Time, sys_C_Time, sys_W_Time;

struct rusage usage;
struct timeval user_Start, user_End, sys_Start, sys_End;
clock_t clock_Start, clock_End;

int main(int argc, char** argv)
{
	unsigned char buf[2];
	int rv, total_bytes, i, codePoint;
	Glyph glyph;
	struct stat statbuf;
	char* fullpath;
	char* hostname;
	struct utsname osname;

	/* initialize global variable*/
	infile_fd = -1;
	outfileFd = -1;
	filename = NULL;
	outfilename = NULL;
	real_R_Time = 0;
	real_C_Time = 0;
	real_W_Time = 0;
	user_R_Time = 0;
	user_C_Time = 0;
	user_W_Time = 0;
	sys_R_Time = 0;
	sys_C_Time = 0;
	sys_W_Time = 0;
	resetGlyph(&glyph);

	/* After calling parse_args(), filename, outfilename, infile_fd, outfileFd and conversion should be set. */
	parse_args(argc, argv);

	/* open infile */
	infile_fd = openToRead(filename);
	if(infile_fd == NO_FD){
		quit_converter(NO_FD);
		exit(EXIT_FAILURE);
	}
	
	/* checkBom of infile */
	source = checkBom(infile_fd);
	if(source == NO_BOM){
		quit_converter(infile_fd);
		exit(EXIT_FAILURE);
	}

	/* open outfile */
	outfileFd = openToWrite(outfilename, conversion);
	if(outfileFd == NO_FD){
		quit_converter(infile_fd);
		exit(EXIT_FAILURE);
	}

	/* check if both files have the same fd */
	if(infile_fd == outfileFd){
		fprintf(stderr, "Cannot target the same file as the input file\n");
		close(infile_fd);
		quit_converter(outfileFd);
		exit(EXIT_FAILURE);
	}	

	/* Now deal with the rest of the bytes.*/
	buf[0] = 0;
	buf[1] = 0;
	rv = 0;

	if(source == LITTLE || source == BIG) {
		rusage_start();
		while(((rv = read(infile_fd, &buf[0], 1)) == 1) && ((rv = read(infile_fd, &buf[1], 1)) == 1)) { 
			rusage_end(READ);

			rusage_start();

			/* Can fill UTF16LE/BE */
			fill_glyph(&glyph, buf, source);

			if(conversion == LITTLE || conversion == BIG){
				if(source != conversion){
					swap_endianness(&glyph);
				}
				rusage_end(CONVERT);

				rusage_start();
				write_glyph(&glyph, (glyph.surrogate)?SURROGATE_SIZE:NON_SURROGATE_SIZE);
				rusage_end(WRITE);
				
			} else if (conversion == EIGHT){
				codePoint = calCodePoint(&glyph, -1); /* In this case we can ignore the total_byte size */

				convert(&glyph, codePoint, EIGHT);

				rusage_end(CONVERT);

				total_bytes = howManyMoreByte(glyph.bytes[0]);

				rusage_start();
				write_glyph(&glyph, total_bytes);
				rusage_end(WRITE);
			} else {
				fprintf(stderr, "Should not reach here, i.e. conversion=NO_BOM\n");
			}			

			resetGlyph(&glyph);
		}
	} else if(source == EIGHT) {
		rusage_start();
		while((rv = read(infile_fd, &buf[0], 1)) == 1) { 
			total_bytes = howManyMoreByte(buf[0]);

			glyph.bytes[0] = buf[0];
			for(i = 1; i < total_bytes; i++){
				rv = read(infile_fd, &buf[0], 1);
				glyph.bytes[i] = buf[0];
			}
			glyph.end = EIGHT;
			glyph.surrogate = false;
			glyph_cnt++;
			rusage_end(READ);
			

			rusage_start();
			codePoint = calCodePoint(&glyph, total_bytes);
			if(codePoint <= 0x7F){
				ASCII_cnt++;
			}
			
			convert(&glyph, codePoint, conversion);
			
			rusage_end(CONVERT);

			rusage_start();
			if(conversion == BIG || conversion == LITTLE)
				write_glyph(&glyph, (glyph.surrogate)?(SURROGATE_SIZE):(NON_SURROGATE_SIZE));
			else
				write_glyph(&glyph, total_bytes);
			rusage_end(WRITE);

			resetGlyph(&glyph);
		}
	} else {
		fprintf(stderr, "Should not reach here, i.e. conversion=NO_BOM\n");
	}

	if(opt_v >= 1){
		fstat(infile_fd, &statbuf);
		fprintf(stderr, "Input file size: %lu bytes\n", statbuf.st_size);

		fullpath = (char*) malloc(PATH_MAX + 1);
		realpath(filename, fullpath);
		fprintf(stderr, "Input file path: %s\n", fullpath);
		free(fullpath);

		switch(source){
			case LITTLE:
			fprintf(stderr, "Input file encoding: %s\n", "UTF16-LE");
			break;
			case BIG:
			fprintf(stderr, "Input file encoding: %s\n", "UTF16-BE");
			break;
			case EIGHT:
			fprintf(stderr, "Input file encoding: %s\n", "UTF8");
			break;
			default:
			fprintf(stderr, "Input file encoding: %s\n", "NO_BOM");
			break;
		}
		
		switch(conversion){
			case LITTLE:
			fprintf(stderr, "Output file encoding: %s\n", "UTF16-LE");
			break;
			case BIG:
			fprintf(stderr, "Output file encoding: %s\n", "UTF16-BE");
			break;
			case EIGHT:
			fprintf(stderr, "Output file encoding: %s\n", "UTF8");
			break;
			default:
			fprintf(stderr, "Input file encoding: %s\n", "NO_BOM");
			break;
		}
		
		hostname = (char*)malloc(MAXHOSTNAMELEN + 1);
		gethostname(hostname, MAXHOSTNAMELEN + 1);
		fprintf(stderr, "Hostmachine: %s\n", hostname);
		free(hostname);

		uname(&osname);
		fprintf(stderr, "Operating System: %s\n", osname.sysname);
	}

	if(opt_v >= 2){
		fprintf(stderr, "Reading: real=%.1f, user=%.1f, sys=%.1f\n", real_R_Time/1000000, user_R_Time/1000000, sys_R_Time/1000000);
		fprintf(stderr, "Converting: real=%.1f, user=%.1f, sys=%.1f\n", real_C_Time/1000000, user_C_Time/1000000, sys_C_Time/1000000);
		fprintf(stderr, "Writing: real=%.1f, user=%.1f, sys=%.1f\n", real_W_Time/1000000, user_W_Time/1000000, sys_W_Time/1000000);
		fprintf(stderr, "ASCII: %.0f%%\n", 100*(((double)ASCII_cnt)/((double)glyph_cnt)));
		fprintf(stderr, "Surrogates: %.0f%%\n", 100*(((double)surrogate_cnt)/((double)glyph_cnt)));
		fprintf(stderr, "Glyphs: %d\n", glyph_cnt);
	}

	quit_converter(infile_fd);
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

Glyph* fill_glyph(Glyph* glyph, unsigned char data[2], endianness end)
{
	unsigned int bits;

	glyph->bytes[FIRST] = data[0];
	glyph->bytes[SECOND] = data[1];

	bits = 0;
	if(end == BIG)
		bits |= ((glyph->bytes[FIRST] << 8) + glyph->bytes[SECOND]);
	else
		bits |= ((glyph->bytes[SECOND] << 8) + glyph->bytes[FIRST]);

	/* Check high surrogate pair using its special value range.*/
	if((bits <= 0xD7FF) || (bits >= 0xE000 && bits <= 0xFFFF))
	{
		/* This block means no surrogate pair */
		glyph->surrogate = false; 
		if(bits <= 0x007f){
			ASCII_cnt++;
		}
	}
	else {
		/* This block means yes surrogate pair */
		if(read(infile_fd, &data[0], 1) == 1 && read(infile_fd, &(data[1]), 1) == 1)
		{
			glyph->bytes[THIRD] = data[0];
			glyph->bytes[FOURTH] = data[1];

			glyph->surrogate = true;
		}
		glyph->surrogate = true;
	}

	if(!glyph->surrogate){
		glyph->bytes[THIRD] = 0;
		glyph->bytes[FOURTH] = 0;
	} else {
		surrogate_cnt++;
	}

	glyph->end = end;
	glyph_cnt++;
	
	return glyph;
}

void write_glyph(Glyph* glyph, int n)
{
	int i;
	i = 0;
	if(++i <= n)
		write(outfileFd, &glyph->bytes[FIRST], 1);
	if(++i <= n)
		write(outfileFd, &glyph->bytes[SECOND], 1);
	if(++i <= n)
		write(outfileFd, &glyph->bytes[THIRD], 1);
	if(++i <= n)
		write(outfileFd, &glyph->bytes[FOURTH], 1);
}

void parse_args(int argc, char** argv)
{
	int option_index, c;
	char* endian_convert;
	
	/* If getopt() returns with a valid (its working correctly) 
	 * return code, then process the args! */
	endian_convert = (char*) malloc(ENDIAN_MAX_LENGTH * sizeof(char));

	while((c = getopt_long(argc, argv, "hu:v", long_options, &option_index)) != -1)
	{
		switch(c){ 
			case 'h':
				print_help();
				free(endian_convert);
				quit_converter(NO_FD);
				exit(EXIT_SUCCESS);
				break;
			case 'u':
				opt_u++;
				
				endian_convert = strncpy(endian_convert, optarg, ENDIAN_MAX_LENGTH);
				break;
			case 'v':
				opt_v++;
				break;
			case '?':
				print_help();
				free(endian_convert);
				quit_converter(NO_FD);
				exit(EXIT_FAILURE);
				break;
			default:
				fprintf(stderr, "%s: %s=%c\n", "Unrecognized argument.\n", strerror(errno), c);
				free(endian_convert);
				quit_converter(NO_FD);
				exit(EXIT_FAILURE);
				break;
		}
	}

	/* Check for utf conversion */
	if(opt_u != 1){
		if(opt_u > 1){
			fprintf(stderr, "Too many UTF options set\n");
			opt_u=-1;
		} else if(opt_u == 0){
			fprintf(stderr, "Required argument not passed: -u or --UTF=OUT_ENC\n");
			opt_u=-1;
		}
	}
	else {
		if(strcmp(endian_convert, "16LE") == 0){
			conversion = LITTLE;
		} else if(strcmp(endian_convert, "16BE") == 0){
			conversion = BIG;
		} else if(strcmp(endian_convert, "8") == 0){
			conversion = EIGHT;
		} else{
			fprintf(stderr, "Invalid UTF option passed: %s\n", endian_convert);
			opt_u=-1;
		}
	}
	free(endian_convert);

	if(opt_u == -1){
		print_help();
		quit_converter(NO_FD);
		exit(EXIT_FAILURE);
	}

	/* Check for filenames */
	if((argc-optind) > 0 && (argc-optind) <= 2){
		filename = (char*) malloc(strlen(argv[optind]) + 1);
		filename = (char*) memset(filename, 0, strlen(argv[optind]) + 1);
		strncpy(filename, argv[optind], strlen(argv[optind]));

		optind++;

		if(argc - optind == 1){
			outfilename = (char*) malloc(strlen(argv[optind]) + 1);
			outfilename = (char*) memset(outfilename, 0, strlen(argv[optind]) + 1);
			strncpy(outfilename, argv[optind], strlen(argv[optind]));
		}
	}
	else if((argc-optind) > 2){
		fprintf(stderr, "Too many arguments passed, aborting...\n");
		print_help();
		quit_converter(NO_FD);
		exit(EXIT_FAILURE);
	}
	else {
		fprintf(stderr, "Filename not given.\n");
		print_help();
		quit_converter(NO_FD);
		exit(EXIT_FAILURE);
	}
}

void print_help(void) {
	int i;
	for(i = 0; i < USAGE_LENGTH; i++){
		printf("%s", USAGE[i]); 
	}
}

void quit_converter(int fd)
{
	if(filename != NULL){
		free(filename);
	}
	if(outfilename != NULL){
		free(outfilename);
	}

	if(outfileFd != STDOUT_FILENO)
		close(outfileFd);

	if(fd != NO_FD)
		close(fd);

	close(STDIN_FILENO);
	close(STDERR_FILENO);
	close(STDOUT_FILENO);
	/* Ensure that the file is included regardless of where we start compiling from. */
}

void rusage_start(){
	int ret;
	ret = getrusage(RUSAGE_SELF, &usage);

	if(ret == -1)
		fprintf(stderr, "%s: %s\n", "getrusage() failed", strerror(errno));

	clock_Start = clock();
	user_Start = usage.ru_utime;
	sys_Start = usage.ru_stime;
}

void rusage_end(measure m){
	int ret;

	clock_End = clock();
	ret = getrusage(RUSAGE_SELF, &usage);

	if(ret == -1)
		fprintf(stderr, "%s: %s\n", "getrusage() failed", strerror(errno));

	user_End = usage.ru_utime;
	sys_End = usage.ru_stime;

	switch(m){
		case READ:
			real_R_Time += (clock_End - clock_Start);
			user_R_Time += ((user_End.tv_sec * 1000000 + user_End.tv_usec) - (user_Start.tv_sec * 1000000 + user_Start.tv_usec));
			sys_R_Time += ((sys_End.tv_sec * 1000000 + sys_End.tv_usec) - (sys_Start.tv_sec * 1000000 + sys_Start.tv_usec)) ;
			break;
		case CONVERT:
			real_C_Time += (clock_End - clock_Start);
			user_C_Time += ((user_End.tv_sec * 1000000 + user_End.tv_usec) - (user_Start.tv_sec * 1000000 + user_Start.tv_usec));
			sys_C_Time += ((sys_End.tv_sec * 1000000 + sys_End.tv_usec) - (sys_Start.tv_sec * 1000000 + sys_Start.tv_usec)) ;
			break;
		case WRITE:
			real_W_Time += (clock_End - clock_Start);
			user_W_Time += ((user_End.tv_sec * 1000000 + user_End.tv_usec) - (user_Start.tv_sec * 1000000 + user_Start.tv_usec));
			sys_W_Time += ((sys_End.tv_sec * 1000000 + sys_End.tv_usec) - (sys_Start.tv_sec * 1000000 + sys_Start.tv_usec)) ;
			break;
	}
}

void resetGlyph(Glyph* glyph){
	void* memset_return = memset(glyph, 0, sizeof(Glyph));
		
	/* Memory write failed, recover from it: */
	if(memset_return == NULL){
		 /* Manually reset the glyph*/
		glyph->bytes[FIRST] = 0;
		glyph->bytes[SECOND] = 0;
		glyph->bytes[THIRD] = 0;
		glyph->bytes[FOURTH] = 0;
		glyph->surrogate = 0;
		glyph->end = 0;
	}
}

/* Checks the BOM and return the value of it*/
endianness checkBom(int fd){
	unsigned char buf[3];
	int rv = 0;

	buf[0] = 0;
	buf[1] = 0;
	if((rv = read(fd, &buf[0], 1)) == 1 && (rv = read(fd, &buf[1], 1)) == 1) { 
		if(buf[0] == 0xff && buf[1] == 0xfe){
			/*file is little endian*/
			return LITTLE;
		} else if(buf[0] == 0xfe && buf[1] == 0xff){
			/*file is big endian*/
			return BIG;
		} else if(buf[0] == 0xEF && buf[1] == 0xBB){
			if((rv = read(fd, &buf[2], 1))==1 && buf[2] == 0xBF){
				return EIGHT;
			}
			else {
				fprintf(stderr, "File has no BOM. buf[%d]=%x, buf[%d]=%x, buf[%d]=%x\n", 0, buf[0], 1, buf[1], 2, buf[2]);	
			}
		} else {
			/*file has no BOM*/
			fprintf(stderr, "File has no BOM. buf[%d]=%x, buf[%d]=%x\n", 0, buf[0], 1, buf[1]);
			quit_converter(NO_FD);
		}
	}
	else{
		fprintf(stderr, "File has nothing to read.");
	}

	return NO_BOM;
}

void writeBom(int fd, endianness end){
	unsigned char ff = 0xff;
	unsigned char fe = 0xfe;
	unsigned char ef = 0xef;
	unsigned char bb = 0xbb;
	unsigned char bf = 0xbf;

	/* 0xef, 0xbb 0xbf is for utf8 */

	switch(end){
		case LITTLE:
			write(fd, &ff, 1);
			write(fd, &fe, 1);
			break;
		case BIG:
			write(fd, &fe, 1);
			write(fd, &ff, 1);
			break;
		case EIGHT:
			write(fd, &ef, 1);
			write(fd, &bb, 1);
			write(fd, &bf, 1);
			break;
		default:
			fprintf(stderr, "%s: %d\n", "writeBom reached default value", end);
	}
}

int calCodePoint(Glyph* glyph, int total_bytes){
	unsigned int codePoint, tmp_i;
	unsigned char tmp;

	codePoint = 0;
	
	if(glyph->end == EIGHT){	
		if(total_bytes == 1){
			codePoint += glyph->bytes[FIRST];
		} else if(total_bytes == 2){
			tmp = glyph->bytes[FIRST] & 0x1F; 
			codePoint += tmp << 6;
			tmp = glyph->bytes[SECOND] & 0x3F;
			codePoint += tmp;
		} else if(total_bytes == 3){
			tmp = glyph->bytes[FIRST] & 0x0F;
			codePoint += tmp << (6 * 2);
			tmp = glyph->bytes[SECOND] & 0x3F;
			codePoint += tmp << 6;
			tmp = glyph->bytes[THIRD] & 0x3F;
			codePoint += tmp;
		} else if(total_bytes == 4){
			tmp = glyph->bytes[FIRST] & 0x07;
			codePoint += tmp << (6 * 3);
			tmp = glyph->bytes[SECOND] & 0x3F;
			codePoint += tmp << (6 * 2);
			tmp = glyph->bytes[THIRD] & 0x3F;
			codePoint += tmp << 6;
			tmp = glyph->bytes[FOURTH] & 0x3F;
			codePoint += tmp;
		}
	} else if(glyph->end == LITTLE){
		if(glyph->surrogate){
			tmp_i = (glyph->bytes[SECOND] << 8) + glyph->bytes[FIRST];
			tmp_i -= 0xD800;
			tmp_i = tmp_i << 10;

			codePoint += tmp_i;

			tmp_i = (glyph->bytes[FOURTH] << 8) + glyph->bytes[THIRD];
			tmp_i -= 0xDC00;
			tmp_i = tmp_i & 0x3FF;

			codePoint += tmp_i;

			codePoint += 0x10000;
		}
		else { /* not surrogate */
			codePoint = (glyph->bytes[SECOND] << 8) + glyph->bytes[FIRST];
		}
	} else if(glyph->end == BIG){
		if(glyph->surrogate){
			tmp_i = (glyph->bytes[FIRST] << 8) + glyph->bytes[SECOND];
			tmp_i -= 0xD800;
			tmp_i = tmp_i << 10;

			codePoint += tmp_i;

			tmp_i = (glyph->bytes[THIRD] << 8) + glyph->bytes[FOURTH];
			tmp_i -= 0xDC00;
			tmp_i = tmp_i & 0x3FF;

			codePoint += tmp_i;

			codePoint += 0x10000;
		}
		else { /* not surrogate */
			codePoint = (glyph->bytes[FIRST] << 8) + glyph->bytes[SECOND];
		}
	}

	return codePoint;
}

void convert(Glyph* glyph, unsigned int codePoint, endianness end){
	unsigned int VAL = 0x10000;
	unsigned int tmp, tmp2;

	glyph->end = end;

	switch(end){
	case LITTLE:
	case BIG:
		if(codePoint > VAL){
			tmp = codePoint - VAL;
			glyph->bytes[0] = (tmp >> 18) + 0xD8;
			glyph->bytes[1] = (tmp >> 10) & 0x000000FF; 
			tmp2 = (tmp & 0x3FF) + 0xDC00;
			glyph->bytes[2] = (tmp2 >> 8);
			glyph->bytes[3] = (tmp2 & 0x000000FF);
			glyph->surrogate = true;
		} else {
			tmp = codePoint;
			glyph->bytes[0] = (tmp >> 8);
			glyph->bytes[1] = ((tmp << 24) >> 24);
		}
		break;
	case EIGHT:
		tmp = codePoint;
		if(codePoint <= 0x7F){
			glyph->bytes[0] = codePoint;
		} else if(codePoint <= 0x7FF){
			glyph->bytes[0] = (tmp >> (6 * 1)) + 0xC0 ; /*1100 0000 */
			glyph->bytes[1] = ((tmp << (32 - (6 * 1))) >> 26) + 0x80 ; /* 1000 0000 */
		} else if(codePoint <= 0xFFFF){
			glyph->bytes[0] = (tmp >> (6 * 2)) + 0xE0 ; /*1110 0000 */
			glyph->bytes[1] = ((tmp << (32 - (6 * 2))) >> 26) + 0x80 ; /* 1011 1111*/
			glyph->bytes[2] = ((tmp << (32 - (6 * 1))) >> 26) + 0x80 ; /* 1011 1111*/
		} else if(codePoint <= 0x1FFFFF){
			glyph->bytes[0] = (tmp >> (6 * 3)) + 0xF0 ; /*1111 0000*/
			glyph->bytes[1] = ((tmp << (32 - (6 * 3))) >> 26) + 0x80 ; /* 1011 1111*/
			glyph->bytes[2] = ((tmp << (32 - (6 * 2))) >> 26) + 0x80 ; /* 1011 1111*/
			glyph->bytes[3] = ((tmp << (32 - (6 * 1))) >> 26) + 0x80 ; /* 1011 1111*/
		}
		break;
	default:
		fprintf(stderr, "covert got %d as endianness, which is not valid\n", end);
		resetGlyph(glyph);
		return;
	}

	if(end == LITTLE){
		tmp = glyph->bytes[0];
		glyph->bytes[0] = glyph->bytes[1];
		glyph->bytes[1] = tmp;
		tmp = glyph->bytes[2];
		glyph->bytes[2] = glyph->bytes[3];
		glyph->bytes[3] = tmp;
	}
}

int howManyMoreByte(unsigned char c){
	unsigned char cmp;

	cmp = c >> 7;
	if(cmp == 0){
		return 1;
	}
	cmp = c >> 5;
	if(cmp == 6){ /* 110 = 6*/
		return 2;
	}
	cmp = c >> 4;
	if(cmp == 14){ /* 1110 = 14 */
		return 3;
	}
	cmp = c >> 3;
	if(cmp == 30){ /* 11110 = 30 */
		return 4;
	}

	return -1;
}

void print_Glyph(Glyph* glyph){
	printf("Glyph->end %d\n", glyph->end);
	printf("Glyph->surrogate %d\n", glyph->surrogate);
	printf("glyph->bytes[0] %08x, glyph->bytes[1] %08x\n", glyph->bytes[0], glyph->bytes[1]);
	printf("glyph->bytes[2] %08x, glyph->bytes[3] %08x\n", glyph->bytes[2], glyph->bytes[3]);
}