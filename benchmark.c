#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MATRIX_SIZE  128
#define ARRAY_SIZE   65536
#define RANDOM_ITERS 50000
#define STRIDE       16
#define LIST_SIZE    8192

typedef struct Node { int value; struct Node* next; } Node;

static int A[MATRIX_SIZE][MATRIX_SIZE];
static int B[MATRIX_SIZE][MATRIX_SIZE];
static int C[MATRIX_SIZE][MATRIX_SIZE];
static int arr[ARRAY_SIZE];
static int result_arr[ARRAY_SIZE];

static unsigned int seed = 12345;
static unsigned int pseudo_rand() {
    seed ^= seed << 13; seed ^= seed >> 17; seed ^= seed << 5;
    return seed;
}

void test_matrix_multiply() {
    int i, j, k;
    for (i = 0; i < MATRIX_SIZE; i++)
        for (j = 0; j < MATRIX_SIZE; j++) {
            A[i][j] = (i * MATRIX_SIZE + j) % 100;
            B[i][j] = (i + j) % 100; C[i][j] = 0;
        }
    for (i = 0; i < MATRIX_SIZE; i++)
        for (k = 0; k < MATRIX_SIZE; k++)
            for (j = 0; j < MATRIX_SIZE; j++)
                C[i][j] += A[i][k] * B[k][j];
    volatile int sink = C[MATRIX_SIZE/2][MATRIX_SIZE/2]; (void)sink;
    printf("[1] Matrix Multiply done. C[64][64] = %d\n", C[MATRIX_SIZE/2][MATRIX_SIZE/2]);
}

void test_sequential_access() {
    int i; long sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) arr[i] = i % 1000;
    for (i = 0; i < ARRAY_SIZE; i++) sum += arr[i];
    for (i = 0; i < ARRAY_SIZE; i++) result_arr[i] = arr[i] * 2;
    printf("[2] Sequential Access done. Sum = %ld\n", sum);
}

void test_random_access() {
    int i; long sum = 0; unsigned int idx;
    for (i = 0; i < ARRAY_SIZE; i++) arr[i] = i;
    for (i = 0; i < RANDOM_ITERS; i++) { idx = pseudo_rand() % ARRAY_SIZE; sum += arr[idx]; }
    for (i = 0; i < RANDOM_ITERS; i++) { idx = pseudo_rand() % ARRAY_SIZE; arr[idx] = (int)(sum % 1000); }
    printf("[3] Random Access done. Sum = %ld\n", sum);
}

void test_strided_access() {
    int i; long sum = 0;
    for (i = 0; i < ARRAY_SIZE; i++) arr[i] = i % 500;
    for (i = 0; i < ARRAY_SIZE; i += STRIDE) sum += arr[i];
    for (i = 0; i < ARRAY_SIZE; i += STRIDE) arr[i] = (int)(sum % 1000);
    printf("[4] Strided Access done (stride=%d). Sum = %ld\n", STRIDE, sum);
}

void test_linked_list() {
    int i; long sum = 0;
    Node* nodes = (Node*)malloc(LIST_SIZE * sizeof(Node));
    if (!nodes) { printf("[5] malloc failed\n"); return; }
    for (i = 0; i < LIST_SIZE - 1; i++) { nodes[i].value = i; nodes[i].next = &nodes[i+1]; }
    nodes[LIST_SIZE-1].value = LIST_SIZE-1; nodes[LIST_SIZE-1].next = NULL;
    for (i = LIST_SIZE-1; i > 0; i--) {
        unsigned int j = pseudo_rand() % (i+1);
        Node* tmp = nodes[i].next; nodes[i].next = nodes[j].next; nodes[j].next = tmp;
    }
    Node* cur = &nodes[0];
    while (cur != NULL) { sum += cur->value; cur = cur->next; }
    printf("[5] Linked List Traversal done. Sum = %ld\n", sum);
    free(nodes);
}

void test_hot_loop() {
    int i, j; long sum = 0; int hot[512];
    for (i = 0; i < 512; i++) hot[i] = i;
    for (j = 0; j < 200; j++) for (i = 0; i < 512; i++) sum += hot[i];
    printf("[6] Hot Loop done. Sum = %ld\n", sum);
}

void test_memory_copy() {
    int i;
    for (i = 0; i < ARRAY_SIZE; i++) arr[i] = i % 256;
    for (i = 0; i < ARRAY_SIZE; i++) result_arr[i] = arr[i];
    int ok = 1;
    for (i = 0; i < 100; i++) if (result_arr[i] != arr[i]) { ok = 0; break; }
    printf("[7] Memory Copy done. Verify: %s\n", ok ? "PASS" : "FAIL");
}

int main() {
    printf("==============================================\n");
    printf("  COA Cache Benchmark\n");
    printf("==============================================\n\n");
    printf("--- Starting Benchmark Suite ---\n\n");
    test_sequential_access();
    test_hot_loop();
    test_strided_access();
    test_matrix_multiply();
    test_memory_copy();
    test_random_access();
    test_linked_list();
    printf("\n--- Benchmark Complete ---\n");
    printf("Check m5out/stats.txt for cache metrics.\n");
    return 0;
}
