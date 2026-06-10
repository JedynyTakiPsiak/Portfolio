// Jakub Ćwiek
// temat programu: Wyszukanie i usunięcie najmniejszego elementu z kopca
// data pierwszej wersji: 11.12.2023r
// data ostatniej modyfikacji: 09.01.2024r



// skrypt na zrobienie wiele danych
// let n=100000;while [ $n -lt 640000000 ];do echo $n > input; echo l >> input; ./das < input;let n=$n*2;echo $n;done;
// xmgrace -> data -> import -> wybierz plik <= aby zrobić wykres 

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_HEAP_SIZE 999999999 // Maksymalna wielkość kopca

struct MinHeap {
    int *heap;
    int size;
};

void swap(int *a, int *b) {
    int temp = *a;
    *a = *b;
    *b = temp;
}

void heapify(struct MinHeap *minHeap, int idx) {
    int smallest = idx;
    int left = 2 * idx + 1;
    int right = 2 * idx + 2;

    if (left < minHeap->size && minHeap->heap[left] < minHeap->heap[smallest]) {
        smallest = left;
    }

    if (right < minHeap->size && minHeap->heap[right] < minHeap->heap[smallest]) {
        smallest = right;
    }

    if (smallest != idx) {
        swap(&minHeap->heap[idx], &minHeap->heap[smallest]);
        heapify(minHeap, smallest);
    }
}

int extractMin(struct MinHeap *minHeap) {
    if (minHeap->size <= 0) {
        return -1; // Kopiec jest pusty
    }

    if (minHeap->size == 1) {
        minHeap->size--;
        return minHeap->heap[0];
    }

    int root = minHeap->heap[0];
    minHeap->heap[0] = minHeap->heap[minHeap->size - 1];
    minHeap->size--;
    heapify(minHeap, 0);

    return root;
}

void insert(struct MinHeap *minHeap, int value, char przypadek) {
    if (minHeap->size >= MAX_HEAP_SIZE) {
        return; // Kopiec jest pełny
    }

    int idx = minHeap->size;
    minHeap->heap[minHeap->size++] = value;

    switch(przypadek) {
        case 'o': // Optymistyczny - utrzymuje kopiec zbudowany z najmniejszym elementem na szczycie
            while (idx > 0 && minHeap->heap[(idx - 1) / 2] > minHeap->heap[idx]) {
                swap(&minHeap->heap[(idx - 1) / 2], &minHeap->heap[idx]);
                idx = (idx - 1) / 2;
            }
            break;
        case 'p': // Pesymistyczny - odwrócenie kopca, aby największy element był na szczycie
            while (idx > 0 && minHeap->heap[(idx - 1) / 2] < minHeap->heap[idx]) {
                swap(&minHeap->heap[(idx - 1) / 2], &minHeap->heap[idx]);
                idx = (idx - 1) / 2;
            }
            break;
        case 'l': // Losowy - naprawa kopca, aby spełniał właściwości kopca
            while (idx > 0 && minHeap->heap[(idx - 1) / 2] > minHeap->heap[idx]) {
                swap(&minHeap->heap[(idx - 1) / 2], &minHeap->heap[idx]);
                idx = (idx - 1) / 2;
            }
            break;
        default:
            printf("Błędny wybór przypadku generowania liczb.\n");
            break;
    }
}

void printHeap(struct MinHeap *minHeap) {
    printf("Kopiec:\n");

    int elementsToPrint = minHeap->size > 63 ? 63 : minHeap->size; // Ustalanie ilości elementów do wyświetlenia
    int poziomy = 2 + floor(log(elementsToPrint + 1)); // Obliczanie liczby poziomów kopca
    int potega = 1;
    int spacja = pow(2, poziomy) - 2; // Obliczanie liczby spacji między wartościami

    for (int i = 0; i < elementsToPrint; i++) {
        for (int j = 0; j < spacja / 2; j++) {
            printf(" ");
        }
        if (i == potega - 1) {
            for (int j = 0; j < spacja; j++) {
                printf(" ");
            }
            for (int j = potega - 1; j < elementsToPrint && j < 2 * potega - 1; j++) {
                printf("%d", minHeap->heap[j]);
                for (int k = 0; k < spacja; k++) {
                    printf("  ");
                }
            }
            printf("\n");
            spacja /= 2;
            potega *= 2;
        }
    }
    printf("\n");
}

int main() {
    struct MinHeap minHeap;
    minHeap.size = 0;
    int heapSize;
    do {
        printf("Podaj wielkość kopca (od 3 do %d): ", MAX_HEAP_SIZE);
        scanf("%d", &heapSize);
    } while (heapSize < 3 || heapSize > MAX_HEAP_SIZE - 1);
	minHeap.heap = new int [heapSize];

    char przypadek;
    do {
        printf("\nWybierz przypadek generowania liczb:\nOptymistyczny == o\nPesymistyczny == p\nLosowy == l\nWybieram: ");
        scanf(" %c", &przypadek);
    } while (przypadek != 'o' && przypadek != 'p' && przypadek != 'l');

    //clock_t startProgram = clock(); // Start stopera czasowego dla całego programu

    srand(time(NULL));
    clock_t startHeapOperations = clock(); // Start stopera czasowego dla operacji na kopcu

    //printf("\nGenerowanie %d liczb...\n", heapSize);
	for (int i = 0; i < heapSize; i++) {
        int value = (rand()); // Losowanie liczby od 100 do 999
        insert(&minHeap, value, przypadek);
    }

    clock_t endHeapOperations = clock(); // Stop stopera czasowego dla operacji na kopcu
    double timeHeapOperations = ((double)(endHeapOperations - startHeapOperations)) / CLOCKS_PER_SEC;

    //printf("Początkowy kopiec zbudowany zgodnie z wybranym przypadkiem:\n");
    //printHeap(&minHeap);

    clock_t startExtraction = clock(); // Start stopera czasowego dla usuwania najmniejszego elementu

	extractMin(&minHeap);    
	//int smallest = extractMin(&minHeap);
    //printf("Usunięcie najmniejszego elementu z kopca: %d\n", smallest);

    // Sprawdzenie czy kopiec jest poprawnie ułożony
    for (int i = (minHeap.size - 1) / 2; i >= 0; i--) {
        heapify(&minHeap, i);
    }

    clock_t endExtraction = clock(); // Stop stopera czasowego dla usuwania najmniejszego elementu
    double timeExtraction = ((double)(endExtraction - startExtraction)) / CLOCKS_PER_SEC;

    //printf("Kopiec po usunięciu najmniejszego elementu:\n");
    //printHeap(&minHeap);

    //clock_t endProgram = clock(); // Stop stopera czasowego dla całego programu
    //double timeProgram = ((double)(endProgram - startProgram)) / CLOCKS_PER_SEC;

    printf("\nCzas operacji na kopcu: %f sekund\n", timeHeapOperations);
    printf("Czas usuwania najmniejszego elementu z kopca: %f sekund\n", timeExtraction);
    //printf("Czas trwania całego programu: %f sekund\n", timeProgram);

    //fprintf(file, "Wybrana wielkość kopca: %d\n", heapSize);

    //fprintf(file, "Przypadek generowania liczb: ");
	FILE *file;
    switch (przypadek) {
        case 'o':
    		file = fopen("optymistyczny.txt", "a");
            //fprintf(file, "Optymistyczny\n");
            break;
        case 'p':
    		file = fopen("pesymistyczny.txt", "a");
            //fprintf(file, "Pesymistyczny\n");
            break;
        case 'l':
    		file = fopen("losowy.txt", "a");
            //fprintf(file, "Losowy\n");
            break;
        default:
    		file = fopen("notdate.txt", "a");
            //fprintf(file, "Nieznany przypadek\n");
            break;
    }

	fprintf(file, "%d %f\n", heapSize, timeExtraction);

    //fprintf(file, "Czas operacji na kopcu: %f sekund\n", timeHeapOperations);
    //fprintf(file, "Czas usuwania najmniejszego elementu z kopca: %f sekund\n", timeExtraction);
    //fprintf(file, "Czas trwania całego programu: %f sekund\n", timeProgram);
    fclose(file); // Zamknięcie pliku

    return 0;
}

