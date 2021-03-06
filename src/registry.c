/* 
   WOB is a REGISTER BASED ENVIRONMENT AND LANGUAGE
   Copyright 2019 Zach Flynn

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

content*
new_content ()
{
  content* c = malloc(sizeof(content));
  c->right = NULL;
  c->left = NULL;
  c->value = NULL;
  c->name = NULL;
  c->key = 0;
  c->do_not_free_data = 0;
  return c;
}

registry*
new_registry (registry* up, size_t hash_size, task* t)
{
  registry* r = malloc(sizeof(registry));
  r->up = up;
  r->hash_size = hash_size;
  r->objects = malloc(sizeof(content*)*hash_size);
  r->elements = 0;
  r->task = t;
  r->being_modified = false;
  
  for (int i = 0; i < hash_size; i++)
    {
      r->objects[i] = NULL;
    }

  return r;
}

bool
is_init_reg (content* r)
{
  return (r->right == NULL) && (r->left==NULL) && (r->value == NULL) &&
    (r->key == 0) && (r->name == NULL);
}

content*
set (registry* reg, data* d, const char* name, int rehash_flag)
{
  unsigned long hash_name = hash_str(name);
  content* c = del(reg,hash_name,-1,false);

  if (d != NULL && d->type == Registry)
    {
      ((registry*) d->data)->up = reg;
    }


  if (c == NULL)
    {
      if (reg->objects[hash_name % reg->hash_size] == NULL)
        {
          reg->objects[hash_name % reg->hash_size] = new_content();
        }
      c = reg->objects[hash_name % reg->hash_size];
      c = head(c);
      content* new_c = new_content();
      new_c->left = c;
      new_c->right = NULL;
      new_c->value = d;
      c->right = new_c;
      new_c->name = malloc(sizeof(char)*(strlen(name)+1));
      strcpy(new_c->name, name);
      new_c->key = hash_name;
      reg->elements++;
      if (reg->task->task->wob_rehash && rehash_flag &&
          (reg->elements > (WOB_LOAD_FACTOR*(reg->hash_size))))
        {
          rehash(reg);
        }

      return new_c;
    }
  else
    {
      c->do_not_free_data = 0;
      c->value = d;
      return c;
    }

  return NULL;

}

data*
get (registry* reg, unsigned long hash_name, int recursive)
{
  if (reg == NULL)
    return NULL;

  if (hash_name == reg->task->task->wob_hash_underscore)
    {
      data* d;
      assign_registry(&d, reg, false, reg->task);
      return d;
    }
  content* c = reg->objects[hash_name % reg->hash_size];
  if (c == NULL || is_init_reg(c))
    {
      if (recursive)
        {
          return get(reg->up, hash_name, recursive);
        }
      else
        {
          return NULL;
        }
    }
  
  c = c->right;

  while (c != NULL)
    {
      if (c->key == hash_name && c->value != NULL)
        {
          if (c->value->type == Registry)
            {
              ((registry*) c->value->data)->up = reg->up;
            }
          
          return c->value;
        }
      c = c->right;
    }

  if ((reg->up != NULL) && recursive)
    {
      return get(reg->up, hash_name, recursive);
    }
  else
    {
      return NULL;
    }
}

data*
lookup (registry* reg, unsigned long hash_name, int recursive)
{
  data* d = get(reg, hash_name, recursive);

  if (d == NULL)
    return NULL;

  d = resolve(d, reg);

  return d;

}

content*
mov (registry* reg, regstr* old, regstr* new)
{
  unsigned long old_element = old->key % reg->hash_size;

  content* cur = reg->objects[old_element];
  if (cur == NULL)
    return NULL;
  cur = cur->right;

  while (cur != NULL)
    {
      if (cur->key == old->key)
        {
          del(reg, new->key, 1, false);
          data* d = cur->value;
          int do_not_free_data = cur->do_not_free_data;
          del(reg, old->key, 0, false);
          content* c = set(reg, d, new->name, 0);
          c->do_not_free_data = do_not_free_data;
          return c;
        }
      cur = cur->right;
    }
  return NULL;
}

content*
del (registry* reg, unsigned long hash_name, int del_data, bool hard_free)
{
  content* cur = reg->objects[hash_name % reg->hash_size];
  
  if (cur == NULL)
    return NULL;
  
  if (is_init_reg(cur))
    return NULL;
  
  cur = tail(cur);

  while (cur != NULL)
    {
      if (cur->key == hash_name)
        {
          if (del_data >= 0)
            reg->elements--;
          
          if (cur->right != NULL && del_data >= 0)
            {
              cur->right->left = cur->left;
            }
          if (cur->left != NULL && del_data >= 0)
            {
              cur->left->right = cur->right;
            }

          if (del_data && cur->value != NULL && (!cur->do_not_free_data))
            {
#ifdef GARBAGE
              if (hard_free)
                {
#undef free_data
                  free_data(cur->value);
#define free_data(x)
                }
#endif
              free_data(cur->value);
              cur->value = NULL;
            }

          if (cur->name != NULL && del_data >= 0)
            {
              free(cur->name);
              cur->name = NULL;
            }


          if (del_data < 0)
            {
              return cur;
            }
          else
            {
              free(cur);
              
              if (is_init_reg(reg->objects[hash_name % reg->hash_size]))
                {
                  free(reg->objects[hash_name % reg->hash_size]);
                  reg->objects[hash_name % reg->hash_size] = NULL;
                }
                
              return NULL;
            }

        }
      cur = cur->right;
    }

  return NULL;
}

void
mark_do_not_free (registry* reg, unsigned long hash_name)
{

  content* c = reg->objects[hash_name % reg->hash_size];
  if (c==NULL || is_init_reg(c))
    return;

  c = c->right;
  while (c != NULL)
    {
      if (c->key == hash_name)
        {
          c->do_not_free_data = 1;
          return;
        }
      c = c->right;
    }

}

data*
get_by_levels (registry* reg, unsigned long* hash_name, int levels, int* is_regstr, char** name)
{
  data* d = get(reg, hash_name[0], 1);
  if (d == NULL)
    {
      char* msg = malloc(sizeof(char)*
                         (strlen("Value at register / not found.")
                          + strlen(name[0]) + 1));
      sprintf(msg, "Value at register /%s not found.", name[0]);
      do_error(msg, reg->task->task);
      free(msg);
    }
  else if (d->type != Registry && levels > 1)
    {
      do_error("Cannot get registers in non-registry.", reg->task->task);
    }
  else
    {
      for (int i=1; i < levels; i++)
        {
          if (d == NULL)
            {
              do_error("Register not found in registry.",
                       reg->task->task);
              return NULL;
            }

          if (d->type != Registry)
            {
              do_error("Cannot get registers in non-registry.",
                       reg->task->task);
              return NULL;
            }

          if (is_regstr[i])
            {
              d = get((registry*) d->data, hash_name[i], 0);
            }
          else
            {
              data* d1 = get(reg, hash_name[i], 1);
              if (d1 == NULL || d1->type != Register)
                {
                  do_error("Cannot use `:` with non-register.",
                           reg->task->task);
                  return NULL;
                }
              else
                {
                  d = get((registry*) d->data,
                          ((regstr*) d1->data)->key,
                          0);
                }
            }
        }

      if (d == NULL)
        {
          do_error("Register not found in registry.",
                   reg->task->task);
          return NULL;
        }
		  
    }

  return d;
}



content*
head (content* c)
{
  if (c == NULL)
    return NULL;

  while (c->right != NULL)
    {
      c = c->right;
    }

  return c;
}

content*
right_n (content* c, size_t n)
{
  if (c == NULL)
    return NULL;

  size_t i = 0;
  while (c->right != NULL && i <= n)
    {
      c = c->right;
      i++;
    }

  return c;
}


content*
tail (content* c)
{
  if (c == NULL)
    return NULL;
  
  while (c->left != NULL)
    {
      c = c->left;
    }

  return c->right;
}

int
update_hash_size (size_t elements, size_t hash_size)
{
  return elements > (hash_size*WOB_LOAD_FACTOR);
}
    
  

size_t
new_hash_size (size_t elements)
{
  size_t hash_size = ceil((double) elements / WOB_LOAD_FACTOR);
  size_t factor = (hash_size / WOB_HASH_SIZE) + 1;
  return WOB_HASH_SIZE*factor;
}
  
void
rehash (registry* r0)
{

  if (!update_hash_size(r0->elements, r0->hash_size))
    return;

  size_t old_size = r0->hash_size;
  r0->hash_size = 2*r0->hash_size;
  r0->elements = 0;
  content** objects = r0->objects;

  r0->objects = malloc(sizeof(content*)*r0->hash_size);

  for (int i = 0; i < r0->hash_size; i++)
    {
      r0->objects[i] = NULL;
    }


  for (int i = 0; i < old_size; i++)
    {
      /* copy */
      content* cur = objects[i];
      if (cur == NULL)
        continue;

      cur = tail(objects[i]);
      while (cur != NULL)
        {
          set(r0, cur->value, cur->name, 0);
          if (cur->do_not_free_data)
            mark_do_not_free(r0, cur->key);
          cur = cur->right;
        }

      /* clean */

      cur = objects[i];
      if (is_init_reg(cur))
        {
          free(cur);
          continue;
        }

      cur = tail(cur);
      if (cur != NULL)
        free(cur->left);

      content* tmp;

      while (cur != NULL)
        {
          free(cur->name);
          tmp = cur->right;
          free(cur);
          cur = tmp;
        }

    }

  free(objects);


}
      
