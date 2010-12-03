compile = g++ -O2

.PHONY: main clean

main: condor

condor: role.o concept.o disjunction.o factory.o parser.o tracker.h ontology.o formatter.o reasoner.cpp
	${compile} -o condor role.o concept.o disjunction.o factory.o ontology.o parser.o formatter.o reasoner.cpp

role: role.o;
role.o: role.h role.cpp
	${compile} -c role.cpp

concept: concept.o;
concept.o: role.o concept.h concept.cpp
	${compile} -c concept.cpp

formatter: formatter.o;
formatter.o: concept.o formatter.h formatter.cpp
	${compile} -c formatter.cpp

factory: factory.o;
factory.o: role.o concept.o tracker.h factory.h factory.cpp
	${compile} -c factory.cpp 

disjunction: disjunction.o;
disjunction.o: role.o concept.o disjunction.h disjunction.cpp
	${compile} -c disjunction.cpp

ontology: ontology.o;
ontology.o: role.o concept.o disjunction.o factory.o ontology.h ontology.cpp
	${compile} -c ontology.cpp

parser: parser.o;
parser.o: role.o concept.o factory.o ontology.o parser.h parser.cpp
	${compile} -c parser.cpp

clean:
	rm -f *.o condor
