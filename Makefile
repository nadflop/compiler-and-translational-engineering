GCC = gcc $(CFLAGS) $(COVFLAGS) $(PROFFLAGS)
CFLAGS = -std=c99 -g - Wshadow -Wall --pedantic -Wvla -Werror
COVFLAGS = -fprofile-arcs -ftest-coverage
PROFFLAG = -pg
VALGRIND = valgrind --tool=memcheck --leak-check=full --verbose --log-file=

team: 
	@echo Team: cendol
	@echo 
	@echo Imanina Zaaim Redha
	@echo imaninazr
	@echo 
	@echo Nur Nadhira Aqilah Binti Mohd Shah
	@echo nadflop

.c.o: 
		$(GCC) -c $*.c

compiler: 
		flex scanner.l
		gcc lex.yy.c -lfl

test: compiler
		./runme inputs/fibonacci.micro fibonacci.out
		diff -b fibonacci.out outputs/fibonacci.out
		./runme inputs/loop.micro loop.out
		diff -b loop.out outputs/loop.out
		./runme inputs/nested.micro nested.out
		diff -b nested.out outputs/nested.out
		./runme inputs/sqrt.micro sqrt.out
		diff -b sqrt.out outputs/sqrt.out


memory: 
		$(GCC) $(VALGRIND) 

clean: 
		/bin/rm -f *.gcda *.gcno gmon.out *gcov
		/bin/rm -f *.o
		/bin/rm -f *.out
		/bin/rm -f lex.yy.c
