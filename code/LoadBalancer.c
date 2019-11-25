#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#define BIG_ENOUGH 1000

void generate_benchmarks(int *benchmarks, int n);
void append_to_string(char *append_to, int number, int current_length);
int seach_string(char *desired, int length, char *fstring, int *pos_point, int look_for);
int not_number(char c);
void restart_string(char *string, int n);

int main(void){
  int *benchmarks, num_nodes, counter_bench = 0, counter_node = 0, i, ch_this_line, pos_point, ch_per_file;
  char string_c[] = "workload000.txt", desired[BIG_ENOUGH], ch, fstring[20];
  FILE *fp_reader, *fp_writer;
  
  srand(time(NULL));
  
  printf("Hey, how many nodes, would you like to do the work for you? ");
  scanf(" %d", &num_nodes);

  /* Assign memory for benchmarks. */
  benchmarks = malloc(num_nodes * sizeof(int));
  if(benchmarks == NULL){
    printf("Memory not allocated.");
    exit(EXIT_FAILURE);
  }
  generate_benchmarks(benchmarks, num_nodes);
  
  /* Start by opening the FILE from which to read. */
  fp_reader = fopen("workloads.txt", "r");
  fp_writer = fopen("temp.txt", "w");
  if(fp_reader == NULL || fp_writer == NULL){
    printf("Couldn't open one of the files.");
    exit(EXIT_FAILURE);
  }

  /* This just tells the user the specification on each node. */
  for(i = 0; i < num_nodes; i++){
    printf("This is node nr. %2d it has the benchmark %d\n", i + 1, benchmarks[i]);
  }

  /* This prints to the file temp, with standard '\[node nr.] [character]' and making sure the charcters per line doesn't exceed 15. */
  ch_this_line = 0;
  while((ch = getc(fp_reader)) != EOF){
    if(ch != '\n'){
      ch_this_line++;
      fprintf(fp_writer, "\\%d %c ", counter_node + 1, ch);
      counter_bench++;
      /* makes it such that when a specific node has recived as many tasks as its bencmark it goes to the next node. */
      if(counter_bench == benchmarks[counter_node]){
        counter_bench = 0;
        counter_node = (counter_node + 1) % num_nodes;
      }
      /* Makes sure the number of characters per line doesn't exceed 15. */
      if(ch_this_line > 15){
        ch_this_line = 0;
        fputc('\n', fp_writer);
      }
    }
  }

  /* closes the 2 files. */
  fclose(fp_reader);
  fclose(fp_writer);

  /* opens 'temp.txt' in read mode. */
  fp_reader = fopen("temp.txt", "r");
  if(fp_reader == NULL){
    printf("Couldn't open temp.txt");
    exit(EXIT_FAILURE);
  }

  /* Runs once for each node. */
  for(i = 1; i <= num_nodes; ++i){
    /* Sets fp_reader to be in the beginning of the file. */
    rewind(fp_reader);
    /* Sets the num for character per file, to 0. mostly for debugging. */
    ch_per_file = 0;
    /* appends a number to the text file with the format 'workload[node nr.].txt'. */
    append_to_string(string_c, i, 8);
    /* opens the file workload[node nr.]. */
    fp_writer = fopen(string_c, "w");
    /* Checks the file as indead been opened. */
    if(fp_writer == NULL){
      printf("Couldn't open %s", string_c);
      exit(EXIT_FAILURE);
    }
    /* goes aslong we haven't reached the end of the file */
    while((ch = getc(fp_reader)) != EOF){
      ungetc(ch, fp_reader);
      /* set the string to pure '\0's. */
      restart_string(desired, sizeof(desired)/sizeof(char));
      fgets(desired, BIG_ENOUGH, fp_reader);
      pos_point = 0;
      /* While there still is more instances where the node is meansured, by '\[node nr]', keep running. */
      while(seach_string(desired, sizeof(desired)/sizeof(char), fstring, &pos_point, i) == 1){
        fprintf(fp_writer, "%s", fstring);
        ch_per_file++;
      }
    }
    /* closes the file, which we are writing in, in the format 'workload[node nr.].txt'. */
    fclose(fp_writer);
  }

  /* Closes the reading file, 'temp.txt'. */
  fclose(fp_reader);

  /* I try to delete the file called temp.txt, but it doesn't seem to work. */
  if(remove("temp.txt") == 0)
    printf("Deleted 'temp.txt' successfully.\n");
  else
    printf("Unable to delete 'temp.txt'.\n");

  /* Start the nodes, such that they get their number and benchmark with. */
  for(i = 0; i < num_nodes; i++){
    restart_string(fstring, 20);
    snprintf(fstring, 20, "START node.exe %d %d\n", i + 1, benchmarks[i]);
    system(fstring);
  }

  /* frees the space which I've located in the heap. */
  free(benchmarks);

  return EXIT_SUCCESS;
}

/* Set a whole string to '\0'. */
void restart_string(char *string, int n){
  int i;
  for(i = 0; i < n; i++){
    string[i] = '\0';
  }
}

/* Looks for a specific number in a string. */
int seach_string(char *desired, int length, char *fstring, int *pos_point, int look_for){
  int i, j;
  if(*pos_point > length)
    return 0;
  
  j = 0;
  for(i = *pos_point; i < length; i++){
    if(desired[i] == (char)(look_for + 48)){
      i++;
      while((desired[i] != '\n' && desired[i] != '\\' && not_number(desired[i])) && i < length){
	if(desired[i] == ' ')
	  i++;
	else{
	  if(desired[i - 1] != '\\'){
  	    fstring[j] = desired[i];
            j++;
	  }
	  i++;
	}
      }
      fstring[j] = '\0';
      *pos_point = i;
      return 1;
    }
  }
  
  return 0;
}

/* Checks that the charcter isn't a number. */
int not_number(char c){
  int i;
  for(i = 0; i < 10; i++){
    if(c == (char)(i + 48))
      return 0;
  }
  return 1;
}

/* replaces charcters in a string with a number, this is usfull in the format 'workload000.txt', where the 000 is replaced with the number. */
void append_to_string(char *append_to, int number, int current_length){
  number = number;
  append_to[current_length]     = (char)(number / 100 + 48);
  append_to[current_length + 1] = (char)(number /  10 + 48);
  append_to[current_length + 2] = (char)(number       + 48);
}

/* This spits out random numbers between 1 and 5. */
void generate_benchmarks(int *benchmarks, int n){
  int i;
  for(i = 0; i < n; ++i){
    benchmarks[i] = (rand() % 5) + 1;
  }
}
