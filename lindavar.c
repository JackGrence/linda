#include <stdlib.h>
#include <string.h>
#include "lindavar.h"
#include "list.h"

LIST_HEAD (var_list);

void
get_variable_value (char *name, linda_tuple *tuple)
{
  list_t *cur;
  linda_variable *var;
  list_for_each (cur, &var_list)
  {
    var = list_entry (cur, linda_variable, list);
    if (!strcmp (name, var->name))
      {
	*tuple = var->tuple;
	tuple->next = NULL;
	return;
      }
  }
}

void
set_variable (char *name, linda_tuple *tuple)
{
  list_t *cur;
  linda_variable *var;
  list_for_each (cur, &var_list)
  {
    var = list_entry (cur, linda_variable, list);
    if (!strcmp (name, var->name))
      {
	var->tuple = *tuple;
	return;
      }
  }
  // create variable
  var = malloc (sizeof (*var));
  strcpy (var->name, name);
  var->tuple = *tuple;
  // add into list
  list_add (&var->list, &var_list);
}
