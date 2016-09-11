#/bin/sh
# heredoc example
(cat << END > tester.c
int cat(FILE* f, void* res, char* filename) {
char c;
int n = 0;
printf("%s\n", filename);
while((c = fgetc(f)) != EOF) {
    printf("%c", c);
    n++;
}
printf("\n");
return n;
}
int main(int argc, char** argv) {
    int validationValue = map(argv[1], );
    fprintf(stderr, "validationValue is %d\n", validationValue);
    return 0;
}
END
)