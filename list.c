/* list.c: A simple linked list implementation.
 * Copyright © 2007  Johan Kiviniemi
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
 * more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 51
 * Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* Why yes, I do have the NIH syndrome. */

#include <stdlib.h>
#include <assert.h>

#include "list.h"

List *
list_new (void)
{
	List *list = calloc (1, sizeof (List));
	if (! list)
		return NULL;

	list->prev = list->next = list;

	return list;
}

void
list_free (List *list)
{
	assert (list != NULL);

	list_foreach_safe (list, i) {
		list_cut (i);
		free (i);
	}

	free (list);
}

List *
list_cut (List *list)
{
	assert (list != NULL);

	list->next->prev = list->prev;
	list->prev->next = list->next;

	list->next = list->prev = list;

	return list;
}

List *
list_add (List *list,
          void *entry)
{
	List *item;

	assert (list != NULL);

	item = list_new ();
	if (! item)
		return NULL;

	item->entry = entry;

	list->next->prev = item;
	item->prev = list;
	item->next = list->next;
	list->next = item;

	return item;
}

List *
list_add_before (List *list,
                 void *entry)
{
	assert (list != NULL);

	return list_add (list->prev, entry);
}

