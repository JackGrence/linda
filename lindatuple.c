#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lindatuple.h"

void
tuple_init ()
{
  memset (&tuple_head, 0, sizeof (tuple_head));
  memset (&queue, 0, sizeof (queue));
}

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
tuple_list_add (linda_tuple *tuple)
{
  tuple_list *p;
  for (p = &tuple_head; p->next != NULL; p = p->next)
    /* move to tail */ ;
  p->next = malloc (sizeof (tuple_list));
  p = p->next;
  p->next = NULL;
  p->tuple = tuple;
  save_tuple ();
}

void
tuple_list_remove (tuple_list *p)
{
  tuple_remove (p->tuple);
  free (p);
  save_tuple ();
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
tuple_remove (linda_tuple *tuple)
{
  linda_tuple *rm;
  while (tuple != NULL)
    {
      rm = tuple;
      tuple = tuple->next;
      free (rm);
    }
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
save_tuple ()
{
  FILE *f;
  long pos;
  char buf[OUTPUT_STR_SIZE];
  int len;
  tuple_list *p;

  f = fopen ("server.txt", "w");
  fwrite ("(", 1, 1, f);
  p = tuple_head.next;
  if (p != NULL)
    {
      len = tuple_to_str (buf, p->tuple);
      fwrite (buf, 1, len, f);
      for (p = p->next; p != NULL; p = p->next)
	{
	  len = tuple_to_str (buf, p->tuple);
	  fwrite (",", 1, 1, f);
	  fwrite (buf, 1, len, f);
	}
    }
  fwrite (")\n", 1, 2, f);
  fclose (f);
}
