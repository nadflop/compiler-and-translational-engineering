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
		bison -d parser.y
		flex scanner.l
		gcc parser.tab.c lex.yy.c -lfl

test: compiler
		./runme step2/input/test1.micro test1.out
		diff -b test1.out step2/output/test1.out
		./runme step2/input/test2.micro test2.out
		diff -b test2.out step2/output/test2.out
		./runme step2/input/test3.micro test3.out
		diff -b test3.out step2/output/test3.out
		./runme step2/input/test4.micro test4.out
		diff -b test4.out step2/output/test4.out
		./runme step2/input/test5.micro test5.out
		diff -b test5.out step2/output/test5.out
		./runme step2/input/test6.micro test6.out
		diff -b test6.out step2/output/test6.out
		./runme step2/input/test7.micro test7.out
		diff -b test7.out step2/output/test7.out
		./runme step2/input/test8.micro test8.out
		diff -b test8.out step2/output/test8.out
		./runme step2/input/test9.micro test9.out
		diff -b test9.out step2/output/test9.out
		./runme step2/input/test10.micro test10.out
		diff -b test10.out step2/output/test10.out
		./runme step2/input/test11.micro test11.out
		diff -b test11.out step2/output/test11.out
		./runme step2/input/test12.micro test12.out
		diff -b test12.out step2/output/test12.out
		./runme step2/input/test13.micro test13.out
		diff -b test13.out step2/output/test13.out
		./runme step2/input/test14.micro test14.out
		diff -b test14.out step2/output/test14.out
		./runme step2/input/test15.micro test15.out
		diff -b test15.out step2/output/test15.out
		./runme step2/input/test16.micro test16.out
		diff -b test16.out step2/output/test16.out
		./runme step2/input/test17.micro test17.out
		diff -b test17.out step2/output/test17.out
		./runme step2/input/test18.micro test18.out
		diff -b test18.out step2/output/test18.out
		./runme step2/input/test19.micro test19.out
		diff -b test19.out step2/output/test19.out
		./runme step2/input/test20.micro test20.out
		diff -b test20.out step2/output/test20.out
		./runme step2/input/test21.micro test21.out
		diff -b test21.out step2/output/test21.out

clean: 
		/bin/rm -f *.gcda *.gcno gmon.out *gcov
		/bin/rm -f *.o
		/bin/rm -f *.out
		/bin/rm -f lex.yy.c
		/bin/rm -f parser.tab.h parser.tab.c

