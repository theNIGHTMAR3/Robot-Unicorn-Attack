#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//Struktura opisuj�ca wektor liczb ca�kowitych.
//j-ta kom�rka wektora w (0 <= j < w.count) jest dost�pna jest jako w.ptr[j].
typedef struct {
    int allocated_size; // rozmiar zaalokowanego bufora
    int count;          // liczba element�w w wektorze
    int* ptr;           // wska�nik do pocz�tku bufora
} vector_t;


// Inicjalizacja wektora *v. Pocz�tkowo tablica b�dzie mia�a rozmiar 1.
void init(vector_t* v) {
    v->count = 0;
    v->allocated_size = 1;
    v->ptr = (int*)malloc(v->allocated_size * sizeof(int));
}

// Realokacja wektora *v, tak aby bufor mia� pojemno�� reallocate_size.
void reallocate(vector_t* v, int reallocate_size) {
    fprintf(stderr, "Zmiana rozmiaru %d->%d [%d element�w u�ywanych]\n",
        v->allocated_size, reallocate_size, v->count);
    v->allocated_size = reallocate_size;
    int* newPtr = (int*)malloc(v->allocated_size * sizeof(int));
    memcpy(newPtr, v->ptr, v->count * sizeof(int));
    free(v->ptr);
    v->ptr = newPtr;
}


// Umieszczenie warto�ci val jako nowego (ko�cowego) elementu wektora *v.
void push_back(vector_t* v, int val) {
    //Realokacja wymagana, gdy bufor nie jest w stanie pomie�ci� wi�cej element�w.
    if (v->count == v->allocated_size)
        reallocate(v, 2 * v->allocated_size);
    v->ptr[v->count] = val;
    v->count++;
}


// Pobranie i usuni�cie warto�ci ko�cowego elementu wektora *v.
int pop_back(vector_t* v) {
    v->count--;
    int retv = v->ptr[v->count];

    //Realokacja realizowana, gdy rozmiar bufora jest ponad dwukrotnie
    //nadmiarowy w stosunku do liczby przechowywanych element�w.
    if (4 * v->count <= v->allocated_size)
        reallocate(v, v->allocated_size / 2);
    return retv;
}