#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

typedef struct Player {
    int id;
    int seed;            
    bool in_match;       
    int heap_idx;        
    struct Player *next; 
} Player;

typedef struct PlayerList {
    Player *head;
} PlayerList;

typedef struct MinHeap {
    Player **arr;
    int size;
    int capacity;
} MinHeap;

typedef struct Match {
    int match_id;
    Player *p1;
    Player *p2;
    struct Match *next;
} Match;

typedef struct MatchQueue {
    Match *head;
    Match *tail;
    int next_match_id;
} MatchQueue;

PlayerList g_players = { .head = NULL };
MinHeap *g_heap = NULL;
MatchQueue g_match_queue = { .head = NULL, .tail = NULL, .next_match_id = 1 };

MinHeap *create_heap(int capacity) {
    MinHeap *h = malloc(sizeof(MinHeap));
    h->arr = malloc(sizeof(Player*) * capacity);
    h->size = 0;
    h->capacity = capacity;
    return h;
}

void heap_resize_if_needed(MinHeap *h) {
    if (h->size >= h->capacity) {
        int newcap = h->capacity * 2;
        Player **n = realloc(h->arr, sizeof(Player*) * newcap);
        if (!n) return; 
        h->arr = n;
        h->capacity = newcap;
    }
}

void heap_swap(MinHeap *h, int i, int j) {
    Player *tmp = h->arr[i];
    h->arr[i] = h->arr[j];
    h->arr[j] = tmp;
    h->arr[i]->heap_idx = i;
    h->arr[j]->heap_idx = j;
}

void heapify_up(MinHeap *h, int idx) {
    while (idx > 0) {
        int parent = (idx - 1) / 2;
        if (h->arr[idx]->seed < h->arr[parent]->seed) {
            heap_swap(h, idx, parent);
            idx = parent;
        } else break;
    }
}

void heapify_down(MinHeap *h, int idx) {
    while (1) {
        int left = 2*idx + 1;
        int right = 2*idx + 2;
        int smallest = idx;
        if (left < h->size && h->arr[left]->seed < h->arr[smallest]->seed)
            smallest = left;
        if (right < h->size && h->arr[right]->seed < h->arr[smallest]->seed)
            smallest = right;
        if (smallest != idx) {
            heap_swap(h, idx, smallest);
            idx = smallest;
        } else break;
    }
}

void heap_push(MinHeap *h, Player *p) {
    heap_resize_if_needed(h);
    int idx = h->size++;
    h->arr[idx] = p;
    p->heap_idx = idx;
    heapify_up(h, idx);
}

Player *heap_pop(MinHeap *h) {
    if (h->size == 0) return NULL;
    Player *top = h->arr[0];
    h->arr[0] = h->arr[--h->size];
    if (h->size > 0) {
        h->arr[0]->heap_idx = 0;
        heapify_down(h, 0);
    }
    top->heap_idx = -1;
    return top;
}

Player *create_player(int id, int seed) {
    Player *p = malloc(sizeof(Player));
    p->id = id;
    p->seed = seed;
    p->in_match = false;
    p->heap_idx = -1;
    p->next = NULL;
    return p;
}

Player *find_player(int id) {
    Player *cur = g_players.head;
    while (cur) {
        if (cur->id == id) return cur;
        cur = cur->next;
    }
    return NULL;
}

void player_list_add(Player *p) {
    p->next = g_players.head;
    g_players.head = p;
}

Match *create_match(Player *p1, Player *p2) {
    Match *m = malloc(sizeof(Match));
    m->match_id = g_match_queue.next_match_id++;
    m->p1 = p1;
    m->p2 = p2;
    m->next = NULL;
    return m;
}

void match_queue_push(Match *m) {
    if (!g_match_queue.head) {
        g_match_queue.head = g_match_queue.tail = m;
    } else {
        g_match_queue.tail->next = m;
        g_match_queue.tail = m;
    }
}

bool addPlayer(int playerId, int seed) {
    if (!g_heap) {
        g_heap = create_heap(16);
    }
    if (find_player(playerId) != NULL) {
        fprintf(stderr, "addPlayer: Player with id %d already exists. Skipping.\n", playerId);
        return false;
    }
    Player *p = create_player(playerId, seed);
    player_list_add(p);
    heap_push(g_heap, p);
    printf("Player   details=>added    id=%d seed=%d\n", playerId, seed);
    return true;
}

Match *scheduleNextMatch(void) {
    if (!g_heap || g_heap->size < 2) {
        return NULL;
    }
    Player *p1 = heap_pop(g_heap);
    Player *p2 = heap_pop(g_heap);
    if (!p1 || !p2) {
        if (p1) heap_push(g_heap, p1);
        if (p2) heap_push(g_heap, p2);
        return NULL;
    }
    p1->in_match = true;
    p2->in_match = true;
    Match *m = create_match(p1, p2);
    match_queue_push(m);
    printf("Match scheduled:Match details=> Match %d: %d(seed %d) vs %d(seed %d)\n",
           m->match_id,
           p1->id, p1->seed,
           p2->id, p2->seed);
    return m;
}

int main(void) {
    addPlayer(101, 3);
    addPlayer(102, 1);
    addPlayer(103, 2);
    addPlayer(104, 8);
    addPlayer(105, 5);
    Match *m;
    while ((m = scheduleNextMatch()) != NULL) {
        (void)m;
    }
    return 0;
}
