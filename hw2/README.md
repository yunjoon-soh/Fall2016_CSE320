# Homework 2
## Yun Joon (Daniel) Soh (108256259)

### Funtionality
* Convert from UTF8, UTF16LE, UTF16BE to UTF8, UTF16LE, UTF16BE

### Usage
* Makefile implemented
	* all 	: create the binary in release mode
	* debug	: create the binary in debug mode

* Binary usage
	* ./utf [-h|--help] -u OUT_ENC | --UTF=OUT_ENC IN_FILE [OUT_FILE]
		* -h, --help	Displays this usage.
		* -v, -vv   	Toggles the verbosity of the program to level 1 or 2.

	* Mandatory argument:
		* -u OUT_ENC, --UTF=OUT_ENC	 Sets the output encoding.
			* 8	UTF-8
			* 16LE	UTF-16 Little Endian
			* 16BE	UTF-16 Big Endian

	* Positional Arguments:
		* IN_FILE   	The file to convert.
		* [OUT_FILE]	Output file name. If not present, defaults to stdout.

### Structure
* include
Header files
* src
Source files
	* filemanager.c : file related functions only
	* utfconverter.c : convert related funtions/main function
* rsrc
Test files
	* Suffix
		*16be: file is in UTF-16BE (Big Endian)
		*16le: file is in UTF-16BE (Litte Endian)
		*8: file is in UTF-8
	* Note. some files are invalid files with truncated bytes.
* test
Testing scripts (works in both Ubuntu and Sparky)
	* argtest.sh : tests arguments only
	* stdout_test.sh : tests printing out to default stdout
	* outfile_test.sh: tests writing to actual files
	* vtest.sh : tests verbosity functionaliy
	* valgrind_test.sh : ONLY IN LINUX, test everything with valgrind --track-fds=yes
* utility
Java utility for creating test files

### Special Cases
* Symbolic link
	* If linked to valid file, treats as normanl file
	* If the original file is deleted, treat as non existing file
* Hard link
	* Treat as normal file
* Truncated Files
	* Abort with dangling result(to stdout or to file), because a lot of the case, files are correct upto the truncated location.
	* Therefore, removing manually the last dangling conversion, you will save at least some part of file conversion.
	* Assume it was a really large file, it is very inefficient to delete correct conversion so far because of the very last byte.
* Files with no BOM
	* Abort
