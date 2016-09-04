//Struct to make analysis more intuitive.
struct Analysis {
	int ascii[128]; //space to store counts for each ASCII character.
	int lnlen; //the longest line’s length
	int lnno; //the longest line’s line number.
	char* filename; //the file corresponding to the struct.
};