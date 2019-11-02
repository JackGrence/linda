CFLAGS=-fopenmp -ggdb3

0866002:

indent:
	indent *.c -T linda_tuple -T tuple_list -T linda_queue
