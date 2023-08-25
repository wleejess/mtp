#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <string.h>

/*
 * A program with a pipeline of 4 threads that interact with each other as producers and consumers.
 *
 * - Input thread: Reads in lines of characters from standard input.
 * - Line separator thread: Replaces every line separator in the input with a space.
 * - Plus sign thread: Replaces every pair of plus signs "++" with a "^".
 * - Output thread: Writes processed data to stdout as lines of 80 characters */

// Program requirements state input will never be longer than 1000 chars, and never more than 49 lines before the stop-processing line.

#define MAX_CHARS 1001
#define MAX_LINES 50

// ----------------- Create buffers
// Buffer 1 - shared resource between input thread and line separator.
char buffer_1[MAX_LINES][MAX_CHARS];    
// Number of lines
int line_count = 0;
// Number of characters
int count_1 = 0; 
// Index where the input thread will put the next item.
int prod_idx_1 = 0;
// Index where the line separator will pick up the next item.
int con_idx_1 = 0;
// Initialize the mutex for buffer 1
pthread_mutex_t input_mutex = PTHREAD_MUTEX_INITIALIZER;
// Initialize the condition variable for buffer 1
pthread_cond_t input_full = PTHREAD_COND_INITIALIZER;

// Buffer 2 - shared resource between the line separator and the plus sign thread.
char buffer_2[MAX_LINES][MAX_CHARS];
int count_2 = 0;
int prod_idx_2 = 0;
int con_idx_2 = 0;
pthread_mutex_t line_separator_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t line_separator_full = PTHREAD_COND_INITIALIZER;

// Buffer 3 - shared resource between the plus sign thread and the output thread.
char buffer_3[MAX_LINES][MAX_CHARS];
int count_3 = 0;
int prod_idx_3 = 0;
int con_idx_3 = 0;
pthread_mutex_t plus_sign_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t plus_sign_full = PTHREAD_COND_INITIALIZER;

// ------------------ Create functions and threads

void put_buff_1(char* item) {
  // Lock mutex before putting item in buffer
  pthread_mutex_lock(&input_mutex);
  // Put item in buffer
  strcpy(buffer_1[prod_idx_1], item);
  // Increment index where next item will be put
  prod_idx_1++;
  count_1++;
  // Signal to the consumer that the buffer is no longer empty
  pthread_cond_signal(&input_full);
  pthread_mutex_unlock(&input_mutex);
}

char* fetch_line() {
  char* line = malloc(MAX_CHARS);
  fgets(line, MAX_CHARS, stdin);
  return line;
}

void *get_input(void *arg) {
  while (1) {
    char* line = fetch_line();
    put_buff_1(line);
    if (strcmp(line, "STOP\n") == 0) {
      break;
    }
    if (strcmp(line, "NULL") == 0) break;
  }
  return NULL;
}

char* get_buff_1() {
  pthread_mutex_lock(&input_mutex);
  while (count_1 == 0)
    // Buffer is empty. Wait for producer to signal that buffer has data.
    pthread_cond_wait(&input_full, &input_mutex);
  char* item = buffer_1[con_idx_1];
  // Increment index from which item will be picked up
  con_idx_1++;
  count_1--;
  pthread_mutex_unlock(&input_mutex);
  return item;
}

void put_buff_2(char* item) {
  pthread_mutex_lock(&line_separator_mutex);
  strcpy(buffer_2[prod_idx_2], item);
  prod_idx_2++;
  count_2++;
  pthread_cond_signal(&line_separator_full);
  pthread_mutex_unlock(&line_separator_mutex);
}

// Line separator thread - replaces every \n with a space.
void *replace_spaces(void *arg) {
  char* item;

  while (1) {
    item = get_buff_1();
    for (int i=0; i < strlen(item); i++) {
      if ( item[i] == '\n') {
        item[i] = ' ';
      }
    }
    put_buff_2(item);
    if (strcmp(item, "STOP ") == 0) break;
  }
  return NULL;
}

char* get_buff_2() {
  pthread_mutex_lock(&line_separator_mutex);
  while (count_2 == 0)
    pthread_cond_wait(&line_separator_full, &line_separator_mutex);
  char* line = buffer_2[con_idx_2];
  con_idx_2++;
  count_2--;
  pthread_mutex_unlock(&line_separator_mutex);
  return line;
}

void put_buff_3(char* item) {
  pthread_mutex_lock(&plus_sign_mutex);
  strcpy(buffer_3[prod_idx_3], item);
  prod_idx_3++;
  count_3++;
  pthread_cond_signal(&plus_sign_full);
  pthread_mutex_unlock(&plus_sign_mutex);
}

// Plus sign thread - replaces every pair of plus signs with a carrot.
void *replace_signs(void *arg) {
  char* item;

  while (1) {
    item = get_buff_2();
    for (int i=0; i < strlen(item); i++) {
      if (item[i] == '+' && item[i + 1] == '+') {
        item[i] = '^';
        memmove(&item[i+1], &item[i+2], strlen(item) - i - 1);
      }
    }
    put_buff_3(item);
    if (strcmp(item, "STOP ") == 0) break;
  }
  return NULL;
}

char* get_buff_3() {
  pthread_mutex_lock(&plus_sign_mutex);
  while (count_3 == 0)
    pthread_cond_wait(&plus_sign_full, &plus_sign_mutex);
  char* item = buffer_3[con_idx_3];
  con_idx_3++;
  count_3--;
  pthread_mutex_unlock(&plus_sign_mutex);
  return item;
}

// Output thread - writes processed data to stdout as 80 chars.
void *print_output(void *arg) {
  char* item;
  int char_count = 0;
  char* pending = calloc(sizeof(MAX_CHARS + 1), sizeof(char));

  while (1) {
    item = get_buff_3();
    if (strcmp(item, "STOP ") == 0) {
;
      break;
    }
    
    for (int i=0; i < strlen(item); i++) {
      pending[char_count] = item[i];
      char_count++;

      if (char_count == 80) {
        for (int j=0; j < 80; j++) {
          write(STDOUT_FILENO, &pending[j], 1);
        }
        write(STDOUT_FILENO, "\n", 1);
        char_count = 0;
        line_count++;
      }
    }
  }
  return NULL;
}


int main() {
  pthread_t thread1, thread2, thread3, thread4;

  // Create the threads
  pthread_create(&thread1, NULL, get_input, NULL);
  pthread_create(&thread2, NULL, replace_spaces, NULL);
  pthread_create(&thread3, NULL, replace_signs, NULL);
  pthread_create(&thread4, NULL, print_output, NULL);

  // Wait for the threads to terminate
  pthread_join(thread1, NULL);
  pthread_join(thread2, NULL);
  pthread_join(thread3, NULL);
  pthread_join(thread4, NULL);

  return EXIT_SUCCESS;
}

