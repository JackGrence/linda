#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLIENT_SIZE (10 + 1) // server
#define DATA_SIZE 1024
#define FIELD_SIZE 200
#define OUTPUT_STR_SIZE (4 + FIELD_SIZE + FIELD_SIZE * DATA_SIZE)
#define CMD_SIZE (OUTPUT_STR_SIZE * 2)


typedef struct _linda_tuple linda_tuple;
struct _linda_tuple
{
  enum linda_types
  { INT, STR } type;
  union linda_data
  {
    char buf[DATA_SIZE];
    int num;
  } data;
  linda_tuple *next;
};

linda_tuple bowls[CLIENT_SIZE];
linda_tuple tuple_head;
int terminate = 0;

int
tuple_to_str (char *buf, linda_tuple *tuple)
{
  char *start = buf;
  linda_tuple *p;
  int len;
  int round;

  *buf++ = '(';
  for (p = tuple, round = 0; p != NULL && round < FIELD_SIZE;
       p = p->next, round++)
    {
      if (p->type == INT)
	len = snprintf (buf, DATA_SIZE, "%d,", p->data.num);
      else if (p->type == STR)
	len = snprintf (buf, DATA_SIZE, "%s,", p->data.buf);
      buf += len;
    }
  *(buf - 1) = ')';
  return (buf - start);
}

void
server ()
{
  char buf[OUTPUT_STR_SIZE];
  char cmd[CMD_SIZE];
  char *s;
  int client_id;
  // parse command
  while (!terminate)
    {
      fgets (cmd, CMD_SIZE, stdin);
      s = strtok (cmd, " ");
      client_id = strtol (s, NULL, 10);
      s = strtok (NULL, " ");
      if (!strcmp (s, "in"))
	{
	  // move tuple to bowl
	}
      else if (!strcmp (s, "out"))
	{
	  // new tuple
	}
      else if (!strcmp (s, "read"))
	{
	  // read tuple
	}
      while (s != NULL)
	{
	  printf ("%s\n", s);
	  s = strtok (NULL, " ");
	}
    }
}

void
client (int id)
{
  FILE *f;
  char buf[OUTPUT_STR_SIZE];
  int len;
  if (bowls[id].next == NULL)
    return;
  snprintf (buf, OUTPUT_STR_SIZE, "%d.txt", id);
  f = fopen (buf, "a+");
  len = tuple_to_str (buf, bowls[id].next);
  fclose (f);
}

void
init ()
{
  memset (bowls, 0, sizeof (bowls));
  memset (&tuple_head, 0, sizeof (tuple_head));
  omp_set_num_threads (CLIENT_SIZE);
}

int
main (int argc, char *argv[])
{
  int id, nthreads;
  init ();
#pragma omp parallel private(id)
    {
      id = omp_get_thread_num ();
      printf ("Hello World from thread %d\n", id);
      if (id == 0)
	{
	  server ();
	}
      else
	{
	  client (id);
	}
#pragma omp barrier
      if (id == 0)
	{
	  nthreads = omp_get_num_threads ();
	  printf ("There are %d threads\n", nthreads);
	}
    }
}
