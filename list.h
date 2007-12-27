#ifndef INCLUDE_LIST_H
#define INCLUDE_LIST_H

/* list.h: A simple linked list implementation.
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

typedef struct list_t List;
struct list_t {
	void *entry;
	List *prev;
	List *next;
};

#define list_foreach(list, i) \
	for (List *i = (list)->next; i != (list); i = i->next)

#define list_foreach_safe(list, i) \
	for (List *i = (list)->next, *_next_##i = i->next; i != (list); \
	     i = _next_##i, _next_##i = i->next)

#define list_empty(list) ( (list)->next == (list) )

List *list_new        (void);
void  list_free       (List *list);
List *list_cut        (List *list);
List *list_add        (List *list, void *entry);
List *list_add_before (List *list, void *entry);

#endif /* INCLUDE_LIST_H */
