/* main.c */

/*
    This program demonstrates a simple application of json_checker. It reads
    a json text from STDIN, producing an error message if the text is rejected.

        % json_checker <test/pass1.json
*/

#include <stdlib.h>
#include <stdio.h>
#include "json_checker.h"

int main(int argc, char* argv[]) {
/*
    Read STDIN. Exit with a message if the input is not well-formed json text.

    jc will contain a json_checker with a maximum depth of 20.
*/
    json_checker jc = new_json_checker(20);
    for (;;) {
        intptr_t next_char = getchar();
        if (next_char <= 0) {
            break;
        }
        if (!json_checker_char(jc, next_char)) {
            fprintf(stderr, "json_checker_char: syntax error\n");
            exit(1);
        }
    }
    if (!json_checker_done(jc)) {
        fprintf(stderr, "json_checker_end: syntax error\n");
        exit(1);
    }

    return EXIT_SUCCESS;
}
