objetivos: ./bin/SVMTagger ./bin/SVMTeval ./bin/SVMTlearn
./bin/SVMTeval:  ./bin/SVMTeval.o  ./bin/dict.o  ./bin/common.o  ./bin/hash.o  ./bin/list.o ./bin/er.o
	g++  ./bin/SVMTeval.o  ./bin/dict.o  ./bin/common.o  ./bin/hash.o  ./bin/list.o  ./bin/er.o -o  ./bin/SVMTeval
./bin/SVMTlearn:  ./bin/list.o  ./bin/swindow.o  ./bin/dict.o   ./bin/SVMTlearn.o  ./bin/hash.o  ./bin/weight.o  ./bin/learner.o  ./bin/stack.o  ./bin/mapping.o  ./bin/common.o  ./bin/er.o
	g++  ./bin/SVMTlearn.o  ./bin/dict.o  ./bin/list.o  ./bin/hash.o  ./bin/swindow.o  ./bin/weight.o  ./bin/learner.o  ./bin/stack.o  ./bin/mapping.o  ./bin/common.o  ./bin/er.o -o  ./bin/SVMTlearn
./bin/SVMTagger:  ./bin/list.o  ./bin/swindow.o  ./bin/SVMTagger.o  ./bin/hash.o  ./bin/learner.o  ./bin/stack.o  ./bin/weight.o  ./bin/dict.o  ./bin/tagger.o  ./bin/common.o ./bin/er.o
	g++  ./bin/SVMTagger.o  ./bin/dict.o  ./bin/list.o  ./bin/hash.o  ./bin/swindow.o  ./bin/weight.o  ./bin/tagger.o  ./bin/stack.o  ./bin/common.o  ./bin/er.o  -o  ./bin/SVMTagger
./bin/SVMTlearn.o:  ./lib/SVMTlearn.cc  ./lib/swindow.h  ./lib/learner.h  ./lib/er.h
	g++ -g -c -ansi  ./lib/SVMTlearn.cc -o ./bin/SVMTlearn.o
./bin/SVMTagger.o:  ./lib/SVMTagger.cc  ./lib/swindow.h  ./lib/tagger.h  ./lib/er.h
	g++ -g -c -ansi  ./lib/SVMTagger.cc -o ./bin/SVMTagger.o
./bin/learner.o:   ./lib/learner.h  ./lib/swindow.h  ./lib/learner.cc  ./lib/stack.h  ./lib/dict.h  ./lib/list.h  ./lib/common.h
	g++ -g  -c   ./lib/learner.cc -o ./bin/learner.o
./bin/tagger.o:   ./lib/tagger.h  ./lib/weight.h  ./lib/swindow.h  ./lib/dict.h  ./lib/tagger.cc  ./lib/stack.h  ./lib/common.h ./lib/strategies.h
	g++ -g  -c   ./lib/tagger.cc -o ./bin/tagger.o
./bin/swindow.o:  ./lib/swindow.cc  ./lib/swindow.h  ./lib/dict.h  ./lib/hash.h  ./lib/list.h  ./lib/weight.h  ./lib/er.h
	g++ -g -c   ./lib/swindow.cc -o ./bin/swindow.o
./bin/mapping.o:  ./lib/mapping.cc  ./lib/mapping.h  ./lib/hash.h
	g++ -g -c   ./lib/mapping.cc -o ./bin/mapping.o
./bin/dict.o:  ./lib/dict.cc  ./lib/dict.h  ./lib/hash.h  ./lib/common.h  ./lib/er.h
	g++ -g -c -ansi  ./lib/dict.cc -o ./bin/dict.o
./bin/list.o:  ./lib/list.cc  ./lib/list.h
	g++ -g -c -ansi  ./lib/list.cc -o ./bin/list.o
./bin/weight.o:  ./lib/weight.h  ./lib/weight.cc
	g++ -g -c  ./lib/weight.cc -o ./bin/weight.o
./bin/hash.o:  ./lib/hash.h  ./lib/hash.cc
	g++ -g -c -ansi  ./lib/hash.cc -o ./bin/hash.o
./bin/stack.o:  ./lib/stack.h  ./lib/stack.cc
	g++ -g -c -ansi  ./lib/stack.cc -o ./bin/stack.o
./bin/er.o:  ./lib/er.h  ./lib/er.cc 
	g++ -g -c -ansi  ./lib/er.cc -o ./bin/er.o
./bin/SVMTeval.o:  ./lib/SVMTeval.cc  ./lib/dict.h  ./lib/common.h  ./lib/hash.h  ./lib/list.h  ./lib/er.h
	g++ -g -c -ansi  ./lib/SVMTeval.cc -o ./bin/SVMTeval.o
./bin/common.o:  ./lib/common.h  ./lib/common.cc
	g++ -g -c -ansi  ./lib/common.cc -o ./bin/common.o
clean:
	rm ./bin/*.o
	rm ./bin/SVMTeval
	rm ./bin/SVMTagger
	rm ./bin/SVMTlearn


