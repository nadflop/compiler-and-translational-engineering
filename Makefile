GCC = gcc $(CFLAGS) $(COVFLAGS) $(PROFFLAGS)
CFLAGS = -std=c99 -g - Wshadow -Wall --pedantic -Wvla -Werror
COVFLAGS = -fprofile-arcs -ftest-coverage
PROFFLAG = -pg
VALGRIND = valgrind --tool=memcheck --leak-check=full --verbose

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

hashtest: clean 
		gcc hash_table.c
		./a.out

hashcheck: hashtest
		$(VALGRIND) ./a.out

test1: clean compiler
		./runme step4/input/test_combination.micro test_combination.out

test1check: 
		$(VALGRIND) ./runme step4/input/test_combination.micro test_combination.out

test: clean compiler
		./runme step3/input/test1.micro test1.out
		diff -b test1.out step3/output/test1.out
		./runme step3/input/test5.micro test5.out
		diff -b test5.out step3/output/test5.out
		./runme step3/input/test6.micro test6.out
		diff -b test6.out step3/output/test6.out
		./runme step3/input/test7.micro test7.out
		diff -b test7.out step3/output/test7.out
		./runme step3/input/test8.micro test8.out
		diff -b test8.out step3/output/test8.out
		./runme step3/input/test11.micro test11.out
		diff -b test11.out step3/output/test11.out
		./runme step3/input/test13.micro test13.out
		diff -b test13.out step3/output/test13.out
		./runme step3/input/test14.micro test14.out
		diff -b test14.out step3/output/test14.out
		./runme step3/input/test16.micro test16.out
		diff -b test16.out step3/output/test16.out
		./runme step3/input/test18.micro test18.out
		diff -b test18.out step3/output/test18.out
		./runme step3/input/test19.micro test19.out
		diff -b test19.out step3/output/test19.out
		./runme step3/input/test20.micro test20.out
		diff -b test20.out step3/output/test20.out
		./runme step3/input/test21.micro test21.out
		diff -b test21.out step3/output/test21.out
		./runme step3/input/test22.micro test22.out
		diff -b test22.out step3/output/test22.out
	
clean: 
		/bin/rm -f *.gcda *.gcno gmon.out *gcov
		/bin/rm -f *.o
		/bin/rm -f *.out
		/bin/rm -f lex.yy.c
		/bin/rm -f parser.tab.h parser.tab.c

