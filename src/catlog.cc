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
	while (argc > 2) {
		argc --;
		argv = argv + 1;
		cout << "loading " << argv[0] << " ... " << endl;
		fstream input1(argv[0], ios::in | ios::binary);
		fast::Log *log1 = new fast::Log();
		log1->ParseFromIstream(&input1);
		for (int i = 0; i<log1->author_size(); i++ ){
			std::string email = log1->author(i).email(); 
			if (authors.find(email) == authors.end()) {
				authors.insert(email);
				fast::Log_Author * author = log0->add_author();
				author->MergeFrom(log1->author(i));
				author->set_id(log0->author_size());
			}
		}
		for (int i = 0; i<log1->commit_size(); i++ ){
			std::string id = log1->commit(i).id(); 
			if (ids.find(id) == ids.end()) {
				ids.insert(id);
				fast::Log_Commit *commit = log0->add_commit();
				commit->MergeFrom(log1->commit(i));
				int author_id = log1->commit(i).author_id(); 
				std::string email = log1->author(author_id - 1).email();
				for (int j = 0; j<log0->author_size(); j++ ){
					if (log0->author(j).email() != email) 
						continue;
					int new_author_id = log0->author(j).id();
					// if (new_author_id != author_id) { std::cout << author_id << "=>" << new_author_id << std::endl; }
					commit->set_author_id(new_author_id);
					break;
				}
			}
		}
	}
	ofstream output(argv[1], ios::out);
        log0->SerializeToOstream(&output);
        google::protobuf::ShutdownProtobufLibrary();
        output.close();
	return 0;
}
