#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Struktura opisuj¹ca wektor liczb ca³kowitych.
//j-ta komórka wektora w (0 <= j < w.count) jest dostêpna jest jako w.ptr[j].
typedef struct {
    int allocated_size; // rozmiar zaalokowanego bufora
    int count;          // liczba elementów w wektorze
    int* ptr;           // wskaŸnik do pocz¹tku bufora
} vector_t;


// Inicjalizacja wektora *v. Pocz¹tkowo tablica bêdzie mia³a rozmiar 1.
void init(vector_t* v) {
    v->count = 0;
    v->allocated_size = 1;
    v->ptr = (int*)malloc(v->allocated_size * sizeof(int));
}

// Realokacja wektora *v, tak aby bufor mia³ pojemnoœæ reallocate_size.
void reallocate(vector_t* v, int reallocate_size) {
    fprintf(stderr, "Zmiana rozmiaru %d->%d [%d elementów u¿ywanych]\n",
        v->allocated_size, reallocate_size, v->count);
    v->allocated_size = reallocate_size;
    int* newPtr = (int*)malloc(v->allocated_size * sizeof(int));
    memcpy(newPtr, v->ptr, v->count * sizeof(int));
    free(v->ptr);
    v->ptr = newPtr;
}


// Umieszczenie wartoœci val jako nowego (koñcowego) elementu wektora *v.
void push_back(vector_t* v, int val) {
    //Realokacja wymagana, gdy bufor nie jest w stanie pomieœciæ wiêcej elementów.
    if (v->count == v->allocated_size)
        reallocate(v, 2 * v->allocated_size);
    v->ptr[v->count] = val;
    v->count++;
}


// Pobranie i usuniêcie wartoœci koñcowego elementu wektora *v.
int pop_back(vector_t* v) {
    v->count--;
    int retv = v->ptr[v->count];

    //Realokacja realizowana, gdy rozmiar bufora jest ponad dwukrotnie
    //nadmiarowy w stosunku do liczby przechowywanych elementów.
    if (4 * v->count <= v->allocated_size)
        reallocate(v, v->allocated_size / 2);
    return retv;
}