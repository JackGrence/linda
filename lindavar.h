#ifndef LINDAVAR
#define LINDAVAR

#include "lindatuple.h"
#include "list.h"

typedef struct _linda_variable linda_variable;
struct _linda_variable
{
  char name[1024];
  linda_tuple tuple;
  list_t list;
};

void get_variable_value (char *name, linda_tuple *tuple);
void set_variable (char *name, linda_tuple *tuple);

#endif /* ifndef LINDAVAR */
