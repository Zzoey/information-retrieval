compileCranfieldSplitter:
	javac -cp .:src/CranfieldSplitter/libs/*  src/CranfieldSplitter/CranfieldSplitter.java

runCranfieldSplitter:
	java -cp .:src/CranfieldSplitter/:src/CranfieldSplitter/libs/* CranfieldSplitter InputFiles/Cranfield/ OutputFiles/Cranfield/Documents/ OutputFiles/Cranfield/Queries/

cleanCranfieldSplitter:
	rm -rf src/CranfieldSplitter/*.class
	rm -rf OutputFiles/Cranfield/Documents/*
	rm -rf OutputFiles/Cranfield/Queries/*
