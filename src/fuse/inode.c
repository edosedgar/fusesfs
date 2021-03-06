/*
<FUSE-based implementation of SFS (Simple File System)>
    Copyright (C) 2016  <Edgar Kaziahmedov>

 This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <fuse/inode.h>
#include <stdlib.h>
#include <stdio.h>
#include <pthread.h>

inode_map_t* inode_map = NULL;

static int inode_lock_init(vino_t vino) 
{
        return pthread_rwlock_init(&(inode_map->inode_table[vino].lock), NULL); 
}

static int inode_lock_destroy(vino_t vino) 
{
        return pthread_rwlock_destroy(&(inode_map->inode_table[vino].lock)); 
}

static int inode_map_lock_init() 
{
        return pthread_rwlock_init(&(inode_map->lock), NULL); 
}

static int inode_map_lock_destroy() 
{
        return pthread_rwlock_destroy(&(inode_map->lock)); 
}

static int inode_map_resize(void) 
{ 
        file_t* buffer = inode_map->inode_table;
        inode_map->inode_table = (file_t*)realloc(inode_map->inode_table, 
                                 (inode_map->c_size) * 2 * sizeof(file_t));
        if (inode_map->inode_table == NULL) {
                inode_map->inode_table = buffer;
                return -1;
        }
        inode_map->c_size *= 2;
        return 0;                
}

int inode_map_create(void) 
{
        inode_map = (inode_map_t*)malloc(sizeof(inode_map_t));
        if (inode_map == NULL)
                return -1;
        inode_map->max_vino = 1;
        inode_map->inode_table = (file_t*)malloc(sizeof(file_t) *
                                                 INODE_MAP_DEFAULT_SIZE);
        if (inode_map->inode_table == NULL) {
                free(inode_map);
                return -1;
        }
        inode_map->c_size = INODE_MAP_DEFAULT_SIZE;

        if (inode_map_lock_init() == -1)
                return -1;

        return 0;
}

int inode_map_delete(void) 
{
        long i = 0;
        for (i = 1; i < inode_map->max_vino; i++) {
                inode_lock_destroy(i);
        }
        inode_map_lock_destroy();
        free(inode_map->inode_table);
        free(inode_map);
        return 0;
}

vino_t pino_add(pino_t pino) 
{
        if (inode_map->max_vino == inode_map->c_size)
                if (inode_map_resize() != 0)
                        return 0;
        inode_map->inode_table[inode_map->max_vino].pino = pino;
        inode_map->inode_table[inode_map->max_vino].dirty = 0; 
        inode_map->inode_table[inode_map->max_vino].openbit = 0;
        if (inode_lock_init(inode_map->max_vino) == -1)
                return -1;

        (inode_map->max_vino)++;
        return inode_map->max_vino - 1;
}
       
vino_t get_vino(pino_t pino) 
{
        uint64_t i = 1;
        for (i = 1; i < inode_map->max_vino; i++) {
                if (inode_map->inode_table[i].pino == pino)
                        return i;
        }
        return 0;
}

int vino_dump(vino_t vino) 
{
#ifndef INODE_DEBUG
        if (vino >= inode_map->max_vino) 
                return -1;
#endif
#ifdef INODE_DEBUG
        if (vino >= inode_map->max_vino) {
                fprintf(stderr, "Virtual  inode: So big\n");
                return -1;
        } else
                fprintf(stderr, "Virtual  inode: %lu\n", vino);
        fprintf(stderr, "Physical inode: %lu\n", 
                        inode_map->inode_table[vino].pino);
        fprintf(stderr, "Dirty flag: %d\n\n",
                        inode_map->inode_table[vino].dirty);
#endif
        return 0;
}

int inode_map_dump(void) 
{
#ifdef INODE_DEBUG
        fprintf(stderr, "Maximal virtual inode: %lu\n",
                        inode_map->max_vino);
        fprintf(stderr, "Current inode map size: %lu\n",
                        inode_map->c_size);
        fprintf(stderr, "Inode table pointer: %p\n\n",
                        inode_map->inode_table);
#endif
        return 0;
}
