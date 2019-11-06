CFLAGS=-fopenmp -ggdb3
OBJECTS=lindavar.o lindatuple.o

0866002: ${OBJECTS}

indent:
	indent *.c *.h -T linda_tuple -T tuple_list -T linda_queue -T linda_variable -T list_t
