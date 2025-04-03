#include <pthread.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t cond = PTHREAD_COND_INITIALIZER;
const int BUFFER_SIZE = 100000;

typedef struct {
    bool isFull;
    char* buffer;
    int size;
}SharedBuffer;

void *reader(void *arg) {
    SharedBuffer* buf = (SharedBuffer*) arg;
    while(1) {
        pthread_mutex_lock(&mutex);
        while(buf -> isFull) {
            pthread_cond_wait(&cond, &mutex);
        }
        size_t sum = 0;
        while (sum != BUFFER_SIZE) {
            size_t to_read = BUFFER_SIZE - sum;
            size_t c = read(0, buf -> buffer + sum, to_read);
            if (c == -1) {
                perror("Error in reading");
                return NULL;
            }
            if (c == 0) {
                break;
            }
            sum += c;
        }
        buf -> size = sum;
        buf -> isFull = true;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        if (buf -> size != BUFFER_SIZE) {
            break;
        }
    }
}

void *writer(void *arg) {
    SharedBuffer* buf = (SharedBuffer*) arg;
    while(1) {
        pthread_mutex_lock(&mutex);
        while(!(buf -> isFull)) {
            pthread_cond_wait(&cond, &mutex);
        }
        size_t sum = 0;
        while(sum < buf -> size) {
            size_t to_write = buf -> size - sum;
            size_t written = write(1, buf -> buffer + sum, to_write);
            if (written == -1) {
                perror("Error in writing");
                break;
            }
            sum += written;
        }
        buf -> isFull = false;
        pthread_cond_signal(&cond);
        pthread_mutex_unlock(&mutex);
        if (buf -> size != BUFFER_SIZE) {
            break;
        }
    }
}

int main() {
    pthread_t t1,t2;
    SharedBuffer* buf = malloc(sizeof(SharedBuffer));
    buf -> isFull = false;
    buf -> buffer = malloc(10);
    pthread_create(&t1, NULL, reader,  buf);
    pthread_create(&t2, NULL, writer,  buf);

    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    return 0;
}