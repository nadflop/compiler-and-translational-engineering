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

ast: clean 
		gcc -g ast.c
		./a.out

test: clean compiler
		#./runme test1.c test1.out
		./runme step5/input/step5_test1.micro step5_test1.out

testcheck: clean compiler
		$(VALGRIND) ./runme step5/input/step5_test2.micro step5_test2.out


testall: clean compiler
		./runme step4/input/test_combination.micro test_combination.out
		./runme step4/input/test_complex.micro test_complex.out
		./runme step4/input/test_expr.micro test_expr.out
		./runme step4/input/test_mult.micro test_mult.out

testallcheck: clean compiler
		$(VALGRIND) ./runme step4/input/test_combination.micro test_combination.out
		$(VALGRIND) ./runme step4/input/test_complex.micro test_combination.out
		$(VALGRIND) ./runme step4/input/test_expr.micro test_combination.out
		$(VALGRIND) ./runme step4/input/test_mult.micro test_combination.out
	
clean: 
		/bin/rm -f *.gcda *.gcno gmon.out *gcov
		/bin/rm -f *.o
		/bin/rm -f *.out
		/bin/rm -f lex.yy.c
		/bin/rm -f parser.tab.h parser.tab.c

