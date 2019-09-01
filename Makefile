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
		./runme step1/inputs/fibonacci.micro fibonacci.out
		diff -b fibonacci.out step1/outputs/fibonacci.out
		./runme step1/inputs/loop.micro loop.out
		diff -b loop.out step1/outputs/loop.out
		./runme step1/inputs/nested.micro nested.out
		diff -b nested.out step1/outputs/nested.out
		./runme step1/inputs/sqrt.micro sqrt.out
		diff -b sqrt.out step1/outputs/sqrt.out

clean: 
		/bin/rm -f *.gcda *.gcno gmon.out *gcov
		/bin/rm -f *.o
		/bin/rm -f *.out
		/bin/rm -f lex.yy.c

