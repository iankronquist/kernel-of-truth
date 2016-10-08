OBJ += file.c.o
#OBJ += hashtable.c.o
OBJ += heap.c.o
OBJ += log.c.o
OBJ += main.c.o
OBJ += physical_allocator.c.o
OBJ += region_vector.c.o
OBJ += slab.c.o
OBJ += sorted_list.c.o
OBJ += string.c.o
# FIXME: Only include in debug builds.
OBJ += ubsan.c.o
