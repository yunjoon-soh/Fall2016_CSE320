#include "lott.h"

int main(int argc, char const* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "%s\n", "No query specified");
        HELP;
        exit(EXIT_FAILURE);
    }

    if(strcmp(argv[1], QUERY_STRINGS[LOW]) == 0){
        current_query = LOW;
    }else if(strcmp(argv[1], QUERY_STRINGS[HIGH]) == 0){
        current_query = HIGH;
    }else if(strcmp(argv[1], QUERY_STRINGS[AVG]) == 0){
        current_query = AVG;
    }else{
        fprintf(stderr, "%s: %s\n", "Not an acceptable query", argv[1]);
        HELP;
        exit(EXIT_FAILURE);
    }

    char* end;
    int ret = -1;

    if(argc < 3){
        fprintf(stderr, "%s\n", "No part specified");
        HELP;
        exit(EXIT_FAILURE);
    }

    if (argv[2][0] == '1') {
        current_part = PART1;
        ret = part1();
        goto end;
    }

    if (argc < 4) {
        fprintf(stderr, "%s\n", "Not enough arguments supplied");
        HELP;
        exit(EXIT_FAILURE);
    }
    size_t nthreads = (size_t)strtoul(argv[2], &end, 10);
    if (errno != 0) {
        perror("Invalid thread count specified");
        exit(0);
    }

    switch (argv[2][0]) {
        case '2': {
            current_part = PART2;
            ret = part2(nthreads);
        } break;
        case '3': {
            current_part = PART3;
            ret = part3(nthreads);
        } break;
        case '4': {
            current_part = PART4;
            ret = part4(nthreads);
        } break;
        case '5': {
            current_part = PART5;
            ret = part2(nthreads);
        } break;
        default: {
            ret = 0;
            fprintf(stderr, "%s\n", "Invalid Part Selction");
        } break;
    }

    end:
    if(ret < 0){
        fprintf(stderr, "Error during execution of %s with %s\n",
            PART_STRINGS[current_part], QUERY_STRINGS[current_query]);
    }

    return 0;
}
