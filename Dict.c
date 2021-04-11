// COMP2521 20T2 Assignment 1
// Dict.c ... implementsation of Dictionary ADT

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "Dict.h"
#include "WFreq.h"

#define isRed(tree)  ((tree) != NULL && (tree)->colour == RED)

typedef enum {RED,BLACK} Colour;
typedef struct _DictNode *Link;

typedef struct _DictNode {
    WFreq data;
    Link left;
    Link right;
    Colour colour;
} DictNode;

struct _DictRep {
    Link tree;
};

Link recursiveInsert(Link n, Link new, char *w, int isRight);
Link recursiveSearch(Link n, char *w);
void recursiveFind(Link n, WFreq *wfs, int size, int *i);
void formHeap(WFreq *wfs, int size, int i);
int wfreq_comp(WFreq a, WFreq b);
void showHelper(Link n, int space);

// create new empty Dictionary
Dict newDict() {
    Dict new_dict = malloc(sizeof(Dict));
    new_dict->tree = NULL;
    return new_dict;
}

// create new link containing given wfreq
Link newLink(WFreq data) {
    Link new_link = malloc(sizeof(DictNode));
    new_link->data = data;
    new_link->left = NULL;
    new_link->right = NULL;
    new_link->colour = RED;
    return new_link;
}

// create new wfreq from given word/freq
WFreq newWFreq(char *word, int freq) {
    WFreq new_wfreq;
    new_wfreq.word = strdup(word);
    new_wfreq.freq = freq;
    return new_wfreq;
}

// perform right rotation
Link rotateRight(Link n) {
    if (n == NULL || n->left == NULL)
        return n;

    Link n2 = n->left;
    n->left = n2->right;
    n2->right = n;
    return n2;
}

// perform left rotation
Link rotateLeft(Link n) {
    if (n == NULL || n->right == NULL)
        return n;

    Link n2 = n->right;
    n->right = n2->left;
    n2->left = n;
    return n2;
}

// insert new word into Dictionary
// return pointer to node containing word
WFreq *DictInsert(Dict d, char *w) {
    WFreq data = newWFreq(w, 1);
    Link new = newLink(data);
    d->tree = recursiveInsert(d->tree, new, w, 0);
    d->tree->colour = BLACK;
    return &new->data;
}

// perform recursive insertion on RB tree
Link recursiveInsert(Link n, Link new, char *w, int isRight) {
    if (n == NULL)
        return new;
    
    if (strcmp(n->data.word, w) == 0) {
        n->data.freq++;
        *new = *n;
        return n;
    }
    
    if (isRed(n->left) && isRed(n->right)) {
        n->colour = RED;
        n->left->colour = BLACK;
        n->right->colour = BLACK;
    }

    int c = strcmp(n->data.word, w);
    if (c > 0) {
        n->left = recursiveInsert(n->left, new, w, 0);
    } else if (c < 0) {
        n->right = recursiveInsert(n->right, new, w, 1);
    }

    if (isRed(n->left)) {
        if (isRed(n->left->right)) {
            n->left = rotateLeft(n->left);
        }

        if (isRed(n->left->left) || isRed(n->left->right)) {
            n = rotateRight(n);
            n->colour = BLACK;
            n->right->colour = RED;
        }
    } else if (isRed(n->right)) {
        if (isRed(n->right->left)) {
            n->left = rotateRight(n->left);
        }

        if (isRed(n->right->right) || isRed(n->right->left)) {
            n = rotateLeft(n);
            n->colour = BLACK;
            n->left->colour = RED;
        }
    }

    if (isRight && isRed(n) && isRed(n->left)) {
        n = rotateRight(n);
    } else if (!isRight && isRed(n) && isRed(n->right)) {
        n = rotateLeft(n);
    }

    return n;
}

// find Word in Dictionary
WFreq *DictFind(Dict d, char *w) {
    Link result = recursiveSearch(d->tree, w);

    if (result == NULL)
        return NULL;
    
    return &result->data;
}

// perform a bst search
Link recursiveSearch(Link n, char *w) {
    if (n == NULL)
        return n;

    int cmp = strcmp(w, n->data.word);

    if (cmp < 0) {
        return recursiveSearch(n->left, w);
    } else if (cmp > 0) {
        return recursiveSearch(n->right, w);
    } else if (cmp == 0) {
        return n;
    }

    return NULL;
}

// find top N frequently occurring words in Dict
// input: Dictionary, array of WFreqs, size of array
// returns: #WFreqs in array, modified array
int findTopN(Dict d, WFreq *wfs, int n) {
    int i = 0;
    recursiveFind(d->tree, wfs, n, &i);

    int size = n;

    // if values in tree less than wfs size form heap on smaller arr
    if (i > 0) {
        size = i;
        for (int j = size / 2 - 1; j >= 0; j--) {
            formHeap(wfs, i, j);
        }
    }

    // perform heapsort on min heap
    for (int j = size - 1; j >= 0; j--) {
        WFreq temp = wfs[j];
        wfs[j] = wfs[0];
        wfs[0] = temp;

        formHeap(wfs, j, 0);
    }

    return size;
}

// form heap of size n from tree
void recursiveFind(Link n, WFreq *wfs, int size, int *i) {
    if (n == NULL)
        return;

    if (*i < size && *i >= 0) {
        wfs[*i] = n->data;
        *i = *i + 1;
    } else if (*i == size) {
        for (int j = size / 2 - 1; j >= 0; j--) {
            formHeap(wfs, size, j);
        }
        *i = -1;
        recursiveFind(n, wfs, size, i);
        return;
    } else if (*i == -1) {
        if (wfreq_comp(wfs[0], n->data) <= 0) {
            wfs[0] = n->data;
            formHeap(wfs, size, 0);
        }
    }

    recursiveFind(n->left, wfs, size, i);
    recursiveFind(n->right, wfs, size, i);
}

// ensure heap is min heap
void formHeap(WFreq *wfs, int size, int i) {
    if (i >= size / 2)
        return;

    int least = i;
    int left = 2 * i + 1;
    int right = 2 * i + 2;

    least = wfreq_comp(wfs[left], wfs[i]) < 0 ? left : i;

    if (right < size) {
        least = wfreq_comp(wfs[right], wfs[least]) < 0 ? right : least;
    }

    if (least != i) {
        WFreq temp = wfs[i];
        wfs[i] = wfs[least];
        wfs[least] = temp;
        formHeap(wfs, size, least);
    }
}

// compare frequencies, if equal return whether word is smaller or not
int wfreq_comp(WFreq a, WFreq b) {
    if (a.freq > b.freq) {
        return 1;
    } else if (a.freq < b.freq) {
        return -1;
    }

    return -strcmp(a.word, b.word);
}

// print a dictionary
void showDict(Dict d)
{
    showHelper(d->tree, 0);
}

void showHelper(Link n, int space) {
    if (n == NULL)
        return;

    space += 10;

    showHelper(n->right, space);

    printf("\n");
    for (int i = 10; i < space; i++)
        printf(" ");
    printf("(%c: %s, %d)\n", n->colour == RED ? 'R' : 'B', n->data.word, n->data.freq);
    showHelper(n->left, space);
}