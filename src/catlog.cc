#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include "git.pb.h"
using namespace std;

int main(int argc, char **argv) {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
	fstream input0(argv[1], ios::in | ios::binary);
	fast::Log *log0 = new fast::Log();
	log0->ParseFromIstream(&input0);
	std::set<std::string> ids;
	std::set<std::string> authors;
	for (int i = 0; i<log0->commit_size(); i++ ){
		ids.insert(log0->commit(i).id());
	}
	for (int i = 0; i<log0->author_size(); i++ ){
		authors.insert(log0->author(i).email());
	}
	while (argc > 3) {
		argc --;
		argv = argv + 1;
		fstream input1(argv[1], ios::in | ios::binary);
		fast::Log *log1 = new fast::Log();
		log1->ParseFromIstream(&input1);
		for (int i = 0; i<log1->commit_size(); i++ ){
			std::string id = log1->commit(i).id(); 
			if (ids.find(id) == ids.end()) {
				// cout << id << endl;
				ids.insert(id);
				log0->add_commit()->MergeFrom(log1->commit(i));
			}
		}
		for (int i = 0; i<log1->author_size(); i++ ){
			std::string email = log1->author(i).email(); 
			if (authors.find(email) == authors.end()) {
				// cout << id << endl;
				authors.insert(email);
				log0->add_author()->MergeFrom(log1->author(i));
			}
		}
	}
	ofstream output(argv[2], ios::out);
        log0->SerializeToOstream(&output);
        google::protobuf::ShutdownProtobufLibrary();
        output.close();
	return 0;
}
