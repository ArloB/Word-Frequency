// tw.c ... compute top N most frequent words in file F
// Usage: ./tw [Nwords] File

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include <ctype.h>
#include "Dict.h"
#include "WFreq.h"
#include "stemmer.h"

#define STOPWORDS "stopwords"
#define MAXLINE 1500
#define MAXWORD 100

#define STARTING "*** START OF"
#define ENDING   "*** END OF"

#define isWordChar(c) (isalnum(c) || (c) == '\'' || (c) == '-')

typedef struct token {
    char *word;
    char *rest;
} token_t;

token_t tokenizer(char *w);

int main(int argc, char *argv[]) {
    int found_start = false;

    FILE *in;       // currently open file
    Dict stopwords; // dictionary of stopwords
    Dict wfreqs;    // dictionary of words from book
    WFreq *results; // array of top N (word,freq) pairs
                    // (dynamically allocated)

    char *fileName; // name of file containing book text
    int nWords;     // number of top frequency words to show

    char line[MAXLINE]; // current input line
    char word[MAXWORD]; // current word

    // process command-line args
    switch (argc) {
    case 2:
        nWords = 10;
        fileName = argv[1];
        break;
    case 3:
        nWords = atoi(argv[1]);
        if (nWords < 10)
            nWords = 10;
        fileName = argv[2];
        break;
    default:
        fprintf(stderr, "Usage: %s [Nwords] File\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    // build stopword dictionary
    stopwords = newDict();
    in = fopen("stopwords", "r");

    if (in == NULL) {
        fprintf(stderr, "Can't open stopwords\n");
        exit(EXIT_FAILURE);
    }

    while (fscanf(in, "%s", word) != EOF) {
        DictInsert(stopwords, word);
    }

    // scan File, up to start of text
    in = fopen(fileName, "r");

    if (in == NULL) {
        fprintf(stderr, "Can't open %s\n", fileName);
        exit(EXIT_FAILURE);
    }

    while (fgets(line, MAXLINE, in) != NULL) {
        if (strstr(line, STARTING) != NULL) {
            found_start = true;
            break;
        }
    }

    if (!found_start) {
        fprintf(stderr, "Not a Project Gutenberg book\n");
        exit(EXIT_FAILURE);
    }

    // scan File reading words and accumualting counts
    wfreqs = newDict();

    while (fgets(line, MAXLINE, in) != NULL && strstr(line, ENDING) == NULL) {
        token_t token_val = tokenizer(line);
        char *token = token_val.word;

        while (token != NULL) {
            for (int i = 0; token[i]; i++) {
                token[i] = tolower(token[i]);
            }

            int len = strlen(token) - 1;

            if (len < 1) {
                token_val = tokenizer(token_val.rest);
                token = token_val.word;
                continue;
            }

            if (DictFind(stopwords, token) == NULL) {
                int end = stem(token, 0, strlen(token) - 1);
                token[end + 1] = '\0';
                DictInsert(wfreqs, token);
            }

            token_val = tokenizer(token_val.rest);
            token = token_val.word;
        }
    }

    // compute and display the top N words
    results = malloc(sizeof(WFreq) * nWords);

    int num = findTopN(wfreqs, results, nWords);

    for (int i = 0; i < num; i++) {
        printf("%7d %s\n", results[i].freq, results[i].word);
    }

    free(results);

    // done
    return EXIT_SUCCESS;
}

token_t tokenizer(char *w)
{
    token_t new;
    int i = 0;

    while ((!isWordChar(w[i]) || iscntrl(w[i])) && w[i] != 0) {
        i++;
    }

    if (w[i] != 0) {
        new.word = &w[i];

        while (isWordChar(w[i])) {
            i++;
        }

        w[i] = '\0';
        new.rest = &w[i + 1];
    } else {
        new.word = NULL;
    }

    return new;
}