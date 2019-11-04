#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define CLIENT_SIZE (10 + 1)	// server
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

typedef struct _tuple_list tuple_list;
struct _tuple_list
{
  linda_tuple *head;
  linda_tuple *next;
};

typedef struct _linda_queue linda_queue;
struct _linda_queue
{
  enum linda_action
  { IN, READ } action[CLIENT_SIZE];
  int ids[CLIENT_SIZE];
  int head;
  int tail;
  linda_tuple *tuple[CLIENT_SIZE];
};

linda_tuple *bowls[CLIENT_SIZE];
linda_queue queue;
tuple_list tuple_head;
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

linda_tuple *
new_tuple ()
{
  linda_tuple head;
  linda_tuple *new;
  char *s;

  head.next = NULL;
  new = &head;
  s = strtok (NULL, " ");
  while (s != NULL)
    {
      printf (":%s\n", s);
      new->next = malloc (sizeof (linda_tuple));
      new = new->next;
      new->next = NULL;
      if (s[0] == '"')
	{
	  new->type = STR;
	  strcpy (new->data.buf, s);
	}
      else
	{
	  new->type = INT;
	  new->data.num = atoi (s);
	}
      s = strtok (NULL, " ");
    }
  return head.next;
}

void
queue_add (int id, enum linda_action action, linda_tuple *tuple)
{
}

void
server ()
{
  char buf[OUTPUT_STR_SIZE];
  char cmd[CMD_SIZE];
  char *s;
  char *action;
  int client_id;
  int len;
  linda_tuple *tuple;
  // parse command
  while (!terminate)
    {
      fgets (cmd, CMD_SIZE, stdin);
      len = strlen (cmd);
      if (cmd[len - 1] == '\n')
	cmd[len - 1] = '\0';
      s = strtok (cmd, " ");
      if (!strcmp (s, "exit"))
	{
	  terminate = 1;
	  return;
	}
      client_id = strtol (s, NULL, 10);
      if (queue_exist (client_id))
	continue;
      s = strtok (NULL, " ");
      action = s;
      tuple = new_tuple ();
      tuple_to_str (buf, tuple);
      printf ("%s\n", buf);
      if (!strcmp (action, "in"))
	{
	  // move tuple to bowl
	  queue_add (client_id, IN, tuple);
	}
      else if (!strcmp (action, "out"))
	{
	  // move new tuple to tuple list
	  tuple_list_add (tuple);
	}
      else if (!strcmp (action, "read"))
	{
	  // copy new tuple to bowl
	  queue_add (client_id, READ, tuple);
	}
      grab_tuple ();
    }
}

void
client (int id)
{
  FILE *f;
  char buf[OUTPUT_STR_SIZE];
  char filename[FIELD_SIZE];
  int len;
  linda_tuple *tuple;

  snprintf (filename, FIELD_SIZE, "%d.txt", id);
  f = fopen (filename, "a+");
  while (!terminate)
    {
      if (bowls[id] == NULL)
	continue;
      tuple = bowls[id];
      bowls[id] = NULL;
      len = tuple_to_str (buf, tuple);
      fwrite (buf, 1, len, f);
      // remove tuple
    }
  fclose (f);
}

void
init ()
{
  memset (bowls, 0, sizeof (bowls));
  memset (&tuple_head, 0, sizeof (tuple_head));
  memset (&queue, 0, sizeof (queue));
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
