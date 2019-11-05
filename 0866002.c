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
  linda_tuple *tuple;
  tuple_list *next;
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
  queue.ids[queue.tail] = id;
  queue.action[queue.tail] = action;
  queue.tuple[queue.tail] = tuple;
  queue.tail = (queue.tail + 1) % CLIENT_SIZE;
}

int
queue_exist (int id)
{
  int i;
  if (queue.head == queue.tail)
    return 0;
  for (i = queue.head; i != queue.tail; i = (i + 1) % CLIENT_SIZE)
    {
      if (queue.ids[i] == id)
	return 1;
    }
  return 0;
}

void
tuple_list_add (linda_tuple *tuple)
{
  tuple_list *p;
  for (p = &tuple_head; p->next != NULL; p = p->next)
    /* move to tail */ ;
  p->next = malloc (sizeof (tuple_list));
  p = p->next;
  p->next = NULL;
  p->tuple = tuple;
}

/* Return 0 or 1 if equal or not */
int
tuplecmp (linda_tuple *t1, linda_tuple *t2)
{
  while (t1->type == t2->type)
    {
      if (t1->type == INT)
	{
	  if (t1->data.num != t2->data.num)
	    break;
	}
      else if (t1->type == STR)
	{
	  if (strcmp (t1->data.buf, t2->data.buf))
	    break;
	}
      t1 = t1->next;
      t2 = t2->next;
      if (t1 == NULL && t2 == NULL)
	return 0;
    }
  return 1;
}

void
tuple_list_remove (tuple_list *p)
{
  linda_tuple *tuple;
  linda_tuple *rm;
  tuple = p->tuple;
  while (tuple != NULL)
    {
      rm = tuple;
      tuple = tuple->next;
      free (rm);
    }
  free (p);
}

void
queue_remove (int index)
{
  int cur;
  int next;

  cur = index;
  next = (cur + 1) % CLIENT_SIZE;
  while (next != queue.tail)
    {
      queue.action[cur] = queue.action[next];
      queue.ids[cur] = queue.ids[next];
      queue.tuple[cur] = queue.tuple[next];
      cur = (cur + 1) % CLIENT_SIZE;
      next = (cur + 1) % CLIENT_SIZE;
    }
  queue.tail--;
  if (queue.tail < 0)
    queue.tail += CLIENT_SIZE;
}

void
grab_tuple ()
{
  int queue_index;
  int found;
  int index;
  linda_tuple *tuple;
  tuple_list *p;
  tuple_list *rm;

  queue_index = queue.head;
  while (queue_index != queue.tail)
    {
      // search tuple
      found = 0;
      for (p = &tuple_head; p->next != NULL; p = p->next)
	{
	  if (!tuplecmp (p->next->tuple, queue.tuple[queue_index]))
	    {
	      found = 1;
	      break;
	    }
	}
      if (found)
	{
	  tuple = p->next->tuple;
	  /* move tuple to bowls, the tuple will be freed by client */
	  bowls[queue.ids[queue_index]] = queue.tuple[queue_index];
	  if (queue.action[queue_index] == IN)
	    {
	      // remove the global tuple
	      rm = p->next;
	      p->next = rm->next;
	      tuple_list_remove (rm);
	    }
	  else if (queue.action[queue_index] == READ)
	    {
	    }
	  queue_remove (queue_index);
	}
      else
	{
	  queue_index = (queue_index + 1) % CLIENT_SIZE;
	}
    }
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
  while (!terminate || bowls[id] != NULL)
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
