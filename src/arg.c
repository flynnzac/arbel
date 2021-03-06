/* 
   WOB was Written On a Bus
   Copyright 2019 Zach Flynn <zlflynn@gmail.com>

   This file is part of WOB.

   WOB is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   WOB is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with WOB (in COPYING file).  If not, see <https://www.gnu.org/licenses/>.
   
*/

#include "wob.h"

arg
gen_arg (int length, int def_free)
{
  arg a;
  a.length = length;
  a.free_data = malloc(length*sizeof(int));
  a.arg_array = malloc(length*sizeof(data*));

  for (int i=0; i < length; i++)
    a.free_data[i] = def_free;

  return a;
}

data*
resolve (data* arg, registry* reg)
{
  if (arg->type == Expression)
    {
      ((instruction*) arg->data)->being_called = true;
      execute_code(((instruction*) arg->data)->stmt, reg);
      ((instruction*) arg->data)->being_called = false;
      return get(reg, reg->task->task->wob_hash_ans, 0);
    }
  else
    {
      return arg;
    }
}

void
check_length (arg* a, int length, char* op, task_vars* t)
{
  if (a->length < length)
    {
      char* error_msg = malloc
        (sizeof(char)*
         (strlen("Number of arguments to < > is less than .") +
          floor(log10(length-1)+1) + 
	  strlen(op) + 1));

      sprintf(error_msg, "Number of arguments to <%s> is less than %d.",
              op, length-1);
          
      do_error(error_msg, t);
      free(error_msg);
    }
}
