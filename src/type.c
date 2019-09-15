/* 
   ARBEL is a REGISTER BASED ENVIRONMENT AND LANGUAGE
   Copyright 2019 Zach Flynn

   This file is part of ARBEL.

   ARBEL is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   ARBLE is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with ARBEL (in COPYING file).  If not, see <https://www.gnu.org/licenses/>.
   
*/

#include "arbel.h"

int
is_integer (const char* str)
{
  int i;
  for (i=0; i < strlen(str); i++)
    {
      if (!(isdigit((int) str[i]) || ((i==0) && str[i]=='-')))
        return 0;
    }
  return 1;
}

int
is_real (const char* str)
{
  int i;
  int decimals = 0;
  for (i=0; i < strlen(str); i++)
    {
      if (!(isdigit((int) str[i]) || ((i==0) && str[i]=='-') ||
            str[i] == '.'))
        return 0;

      if (str[i] == '.' && decimals == 1)
        return 0;

      if (str[i] == '.')
        decimals++;
    }
  return decimals==1;
}


int
is_numeric (data* d)
{
  if (d->type == REAL ||
      d->type == INTEGER)
    {
      return 1;
    }
  else
    {
      return 0;
    }
}

int
is_register (const char* str)
{
  return (str[0] == '/');
}

int
is_reference (const char* str)
{
  return (str[0] == '\\');
}

int
is_whitespace (const char c)
{
  if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r'))
    return 1;
  else
    return 0;
}

const char*
str_type (data_type type)
{
  const char* s = "";

  switch (type)
    {
    case INTEGER:
      s = "Integer";
      break;
    case REAL:
      s = "Real";
      break;
    case STRING:
      s = "String";
      break;
    case REGISTER:
      s = "Register";
      break;
    case REGISTRY:
      s = "Registry";
      break;
    case INSTRUCTION:
      s = "Instruction";
      break;
    case ACTIVE_INSTRUCTION:
      s = "Active-Instruction";
      break;
    case OPERATION:
      s = "Instruction";
      break;
    case REFERENCE:
      s = "Reference";
      break;
    case NOTHING:
      s = "Nothing";
      break;
    case ARBEL_FILE:
      s = "File";
    }

  return s;
}