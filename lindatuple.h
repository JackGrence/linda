#ifndef LINDATUPLE
#define LINDATUPLE

#define CLIENT_SIZE (1024)	// server
#define DATA_SIZE 1024
#define FIELD_SIZE 200
#define OUTPUT_STR_SIZE (4 + FIELD_SIZE + FIELD_SIZE * DATA_SIZE)
#define CMD_SIZE (OUTPUT_STR_SIZE * 2)

typedef struct _linda_tuple linda_tuple;
struct _linda_tuple
{
  enum linda_types
  { INT, STR, VAR } type;
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

int tuple_to_str (char *buf, linda_tuple *tuple);
void tuple_list_add (linda_tuple *tuple);
void tuple_list_remove (tuple_list *p);
int tuplecmp (linda_tuple *t1, linda_tuple *t2);
void tuple_remove (linda_tuple *tuple);
void queue_remove (int index);
void queue_add (int id, enum linda_action action, linda_tuple *tuple);
int queue_exist (int id);
void save_tuple ();
void tuple_init ();

linda_queue queue;
tuple_list tuple_head;

#endif /* ifndef LINDATUPLE */
