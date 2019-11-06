#include <omp.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include "lindavar.h"
#include "lindatuple.h"


linda_tuple *bowls[CLIENT_SIZE];
int terminate = 0;

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
      new->next = malloc (sizeof (*new));
      new = new->next;
      new->next = NULL;
      if (s[0] == '"')
	{
	  new->type = STR;
	  strcpy (new->data.buf, s);
	}
      else if (s[0] == '?')
	{
	  /* assign variable later */
	  new->type = VAR;
	  strcpy (new->data.buf, &s[1]);
	}
      else if (isdigit (s[0]))
	{
	  new->type = INT;
	  new->data.num = atoi (s);
	}
      else
	{
	  /* replace linda variable */
	  get_variable_value (s, new);
	}
      s = strtok (NULL, " ");
    }
  return head.next;
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
	  while (bowls[queue.ids[queue_index]])
	    /* wait */ ;
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
      if (s == NULL)
	continue;
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
      save_tuple ();
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
  while (!terminate || bowls[id] != NULL)
    {
      if (bowls[id] == NULL)
	continue;
      f = fopen (filename, "a+");
      tuple = bowls[id];
      bowls[id] = NULL;
      len = tuple_to_str (buf, tuple);
      fwrite (buf, 1, len, f);
      fwrite ("\n", 1, 1, f);
      // remove tuple
      tuple_remove (tuple);
      fclose (f);
    }
}

void
init ()
{
  memset (bowls, 0, sizeof (bowls));
  tuple_init ();
}

int
main (int argc, char *argv[])
{
  int id, nthreads;
  init ();
  scanf ("%d", &nthreads);
  nthreads++;
  if (nthreads > CLIENT_SIZE)
    {
      fprintf (stderr, "Too large\n");
      exit (-1);
    }
  omp_set_num_threads (nthreads);
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
