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

stack: 
		gcc actrec.c
		./a.out

stackcheck: 
		gcc actrec.c
		$(VALGRIND) ./a.out

test: clean compiler
		./runme test1.c test1.out

test1: clean compiler
		./runme step7/step7_test1.micro test1.out

test2: clean compiler
		./runme step7/step7_test2.micro test2.out

test3: clean compiler
		./runme step7/step7_test3.micro test3.out

test4: clean compiler
		./runme step7/step7_test4.micro test4.out

test5: clean compiler
		./runme step7/step7_test5.micro test5.out

test6: clean compiler
		./runme step7/step7_test6.micro test6.out

test7: clean compiler
		./runme step7/step7_test7.micro test7.out

test12: clean compiler
		./runme step7/step7_test12.micro test12.out

testall: clean compiler
		for value in {1..21} ; do \
			echo Testing test$$value ; \
			./runme step7/step7_test$$value.micro test$$value.out ; \
		done

tiny1: test1
		./tiny step6/output/step6_test1.out < step6/input/step6_test1.input
		./tiny test1.out < step6/input/step6_test1.input

tiny2: test2
		./tiny step6/output/step6_test2.out < step6/input/step6_test2.input
		./tiny test2.out < step6/input/step6_test2.input

tiny3: test3
		./tiny step6/output/step6_test3.out < step6/input/step6_test3.input
		./tiny test3.out < step6/input/step6_test3.input

testcheck: clean compiler
		$(VALGRIND) ./runme step6/input/step6_test2.micro step5_test2.out

	
clean: 
		/bin/rm -f *.gcda *.gcno gmon.out *gcov
		/bin/rm -f *.o
		/bin/rm -f *.out
		/bin/rm -f lex.yy.c
		/bin/rm -f parser.tab.h parser.tab.c

