/*
 * OpenAL cross platform audio library
 *
 * Copyright (C) 1999-2000 by Authors.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public
 * License along with this library; if not, write to the
 * Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA  02111-1307, USA.
 */

#include <assert.h>
#include "Memory.h"
#include "Table.h"

struct ALtable {
	ALvoid *data;
	ALtable *next;
	ALtable *prev;
};

ALtable *alimCreateTable(void)
{
	ALtable *table = (ALtable *) alimMemAlloc(sizeof(ALtable));

	if (table != NULL) {
		table->data = NULL;
		table->next = table;
		table->prev = table;
	}

	return table;
}

ALvoid alimDeleteTable(ALtable *table)
{
	ALtable *entry;

	assert(table != NULL);

	while ((entry = table->next) != table) {
		table->next = entry->next;
		alimMemFree(entry);
	}
	alimMemFree(table);
}

ALvoid *alimTableGetEntries(ALtable *table, ALvoid **cookie)
{
	ALtable *entry = (ALtable *) *cookie;

	assert(table != NULL);

	if (entry != NULL)
		entry = entry->next;
	else
		entry = table->next;

	*cookie = entry;

	return entry->data;
}

static ALtable *alimTableFindEntry(ALtable *table, ALuint handle)
{
	ALtable *entry = table;
	
	assert(table != NULL);

	while ((entry = entry->next) != table) {
		if ((ALuint) entry == handle)
			return entry;
	}
	return NULL;
}

ALuint alimTableCreateEntry(ALtable *table)
{
	ALtable *entry = (ALtable *) alimMemAlloc(sizeof(ALtable));

	assert(table != NULL);

	if (entry != NULL) {
		entry->data = NULL;
		entry->next = table->next;
		entry->prev = table;
		entry->next->prev = entry;
		entry->prev->next = entry;
	}
	return (ALuint) entry;
}

ALvoid alimTableDeleteEntry(ALtable *table, ALuint handle)
{
	ALtable *entry = alimTableFindEntry(table, handle);
	
	if (entry != NULL) {
		entry->prev->next = entry->next;
		entry->next->prev = entry->prev;
		alimMemFree(entry);
	}
}

ALboolean alimTableHasEntry(ALtable *table, ALuint handle)
{
	ALtable *entry = alimTableFindEntry(table, handle);
	
	if (entry != NULL)
		return AL_TRUE;
	return AL_FALSE;
}

ALvoid *alimTableGetEntry(ALtable *table, ALuint handle)
{
	ALtable *entry = alimTableFindEntry(table, handle);
	
	if (entry != NULL)
		return entry->data;
	return NULL;
}

ALvoid alimTableSetEntry(ALtable *table, ALuint handle, ALvoid *data)
{
	ALtable *entry = alimTableFindEntry(table, handle);

	if (entry != NULL)
		entry->data = data;
}
