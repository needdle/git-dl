#include <fstream>
#include <iostream>
#include <string>
#include <stdlib.h>
#include "git.pb.h"
using namespace std;

#define SEPARATOR "~~~~~~~~~~~~"
#define DIFF_PREFIX "diff --git "
#define INDEX_PREFIX "index "
#define HUNK_PREFIX "@@ "
#define SKIP_PREFIX1 "--- "
#define SKIP_PREFIX2 "+++ "
#define SKIP_PREFIX3 "new file mode "
#define SKIP_PREFIX4 "deleted file mode "

#define IS_CODE(x) (x=="cpp" || x=="hpp" || x=="java" || x=="cc" || x=="h" || x=="m" || x=="cs" )

int mainRoutine(int argc, char**argv);

int lookup_author(fast::Log *log, std::string name, std::string email) {
	for (int i=0; i<log->author().size(); i++) {
		const fast::Log_Author author = log->author().Get(i);
		if (author.name() == name && author.email() == email) {
			return author.id();
		}
	}
	fast::Log_Author *author = log->add_author();
	author->set_id(log->author().size());
	author->set_name(name);
	author->set_email(email);
	return author->id();
}

/** 
 * convert text into a protobuf element
 */
void srcML(fast::Element *unit, std::string text, std::string ext) {
	char buf[100];
	strcpy(buf, "temp.XXXXXX");
	mkstemp(buf);
	remove(buf);
	std::string fn = "/tmp/";
	fn += buf;
	remove(fn.c_str());
	strcat(buf, ".");
	strcat(buf, ext.c_str());
	string src_filename = buf;
	ofstream output(src_filename, ios::out);
	output << text << endl;
	output.close();
	strcpy(buf, "temp.XXXXXX");
	mkstemp(buf);
	remove(buf);
	strcat(buf, ".pb");
	string pb_filename = buf;
	char **argv = (char**) malloc(3*sizeof(char*));
	argv[1] = (char*) src_filename.c_str();
	argv[2] = (char*) pb_filename.c_str();
	// cout << argv[1] << " " << argv[2] << endl;
	mainRoutine(3, argv);
	remove(src_filename.c_str());
	fstream input(pb_filename, ios::in | ios::binary);
	unit->ParseFromIstream(&input);
	input.close();
	remove(pb_filename.c_str());
}

void process_hunk_xml(fast::Log_Commit_Diff_Hunk *hunk, std::string text, std::string ext) {
        size_t linePos;
	std::string text_old = "";
	std::string text_new = "";
	do {
	    linePos = text.find("\n");
	    if (linePos != std::string::npos) {
		std::string line = text.substr(0, linePos);
		size_t negPos = line.find("-");
		size_t posPos = line.find("+");
		bool is_special = false;
		if (negPos != std::string::npos) {
		    std::string prefix= line.substr(0, negPos);
		    if (prefix == "") {
			    is_special = true;
			    line = line.substr(negPos + 1);
			    text_old += line + "\n";
		    }
		}
		if (posPos != std::string::npos) {
		    std::string prefix= line.substr(0, posPos);
		    if (prefix == "") {
			    is_special = true;
			    line = line.substr(posPos + 1);
			    text_new += line + "\n";
		    }
		}
		if (! is_special) {
		    line = line.substr(1);
		    text_old += line + "\n";
		    text_new += line + "\n";
		}
		text = text.substr(linePos + 1);
	    }
	} while (linePos != std::string::npos);
	if (text_old != "") {
		fast::Element *unit = hunk->add_element();
		srcML(unit, text_old, ext);
		// ignore the filename field
		unit->mutable_unit()->set_filename("");
	}
	if (text_new != "") {
		fast::Element *unit = hunk->add_element();
		srcML(unit, text_new, ext);
		// ignore the filename field
		unit->mutable_unit()->set_filename("");
	}
}

void process_hunk_text(fast::Log_Commit_Diff_Hunk *hunk, std::string text) {
        size_t linePos;
	do {
	    linePos = text.find("\n");
	    if (linePos != std::string::npos) {
		std::string line = text.substr(0, linePos);
		size_t negPos = line.find("-");
		size_t posPos = line.find("+");
		bool is_special = false;
		if (negPos != std::string::npos) {
		    std::string prefix= line.substr(0, negPos);
		    if (prefix == "") {
			    is_special = true;
			    line = line.substr(negPos + 1);
			    fast::Log_Commit_Diff_Hunk_ModLine * modline = hunk->add_mod();
			    modline->set_line(line);
			    modline->set_is_add(false);
			    modline->set_is_del(true);
		    }
		}
		if (posPos != std::string::npos) {
		    std::string prefix= line.substr(0, posPos);
		    if (prefix == "") {
			    is_special = true;
			    line = line.substr(posPos + 1);
			    fast::Log_Commit_Diff_Hunk_ModLine * modline = hunk->add_mod();
			    modline->set_line(line);
			    modline->set_is_add(true);
			    modline->set_is_del(false);
		    }
		}
		if (! is_special) {
			fast::Log_Commit_Diff_Hunk_ModLine * modline = hunk->add_mod();
			modline->set_line(line);
			modline->set_is_add(false);
			modline->set_is_del(false);
		}
		text = text.substr(linePos + 1);
	    }
	} while (linePos != std::string::npos);
}

void commit(fast::Log_Commit * current_commit, std::string &diff) {
	if (diff.length() > 1000000) {
		cout << "Ignoring the diff block because it is too big: "<< diff.length() << endl;
		diff = "";
		return;
	}
	std::string file_a;
	std::string file_b;
	std::string diff_line;
	if (current_commit != NULL) {
		// cout << "." << flush;
		fast::Log_Commit_Diff_Hunk * hunk = NULL;
		fast::Log_Commit_Diff * diff_record = NULL;
		size_t linePos = std::string::npos;
		std::string hunk_text = "";
		int lineno = 0;
		do {
		    linePos = diff.find("\n");
		    if (linePos != std::string::npos) {
			diff_line = diff.substr(0, linePos);
			diff = diff.substr(linePos + 1);
			bool is_special = false;
			size_t prefixPos = diff_line.find(DIFF_PREFIX);
			if (prefixPos != std::string::npos) {
			    std::string diff_prefix= diff_line.substr(0, prefixPos);
			    if (diff_prefix == "") {
				diff_line = diff_line.substr(prefixPos + strlen(DIFF_PREFIX));
				size_t fieldPos = diff_line.find(" b/");
				file_a = diff_line.substr(2, fieldPos - 2);
				file_b = diff_line.substr(fieldPos+3);
				diff_record = current_commit->add_diff();
				diff_record->set_a(file_a);
				diff_record->set_b(file_b);
				bool is_code = false;
				size_t extPos = file_a.rfind(".");
				if (extPos != std::string::npos) {
					std::string ext = file_a.substr(extPos+1);
					if (IS_CODE(ext)) {
						is_code = true;
						diff_record->set_is_code(ext);
					}
				} 
				if (!is_code) {
					extPos = file_b.rfind(".");
					if (extPos != std::string::npos) {
						std::string ext = file_b.substr(extPos+1);
						if (IS_CODE(ext)) {
							is_code = true;
							diff_record->set_is_code(ext);
						}
					}
				}
				if (! is_code) 
					diff_record->set_is_code("");
				is_special = true;
				hunk = NULL;
				hunk_text = "";
			    }
			}
			prefixPos = diff_line.find(INDEX_PREFIX);
			if (prefixPos != std::string::npos) {
			    std::string index_prefix= diff_line.substr(0, prefixPos);
			    if (index_prefix == "") {
				    // index aa28709..05acaef 100644
				    std::string index_line = diff_line.substr(prefixPos + strlen(INDEX_PREFIX));
				    size_t from_Pos = index_line.find("..");
				    size_t mode_Pos = index_line.find(" ");
				    if (from_Pos != std::string::npos) {
					std::string from_id = index_line.substr(0, from_Pos);
					std::string to_id = "";
					std::string mode = "";
					if (mode_Pos == std::string::npos) {
					    to_id = index_line.substr(from_Pos + 2);
					} else {
					    to_id = index_line.substr(from_Pos + 2).substr(0, mode_Pos - from_Pos - 2);
					    mode = index_line.substr(mode_Pos + 1);
					}
					if (from_id == "0000000")
						diff_record->set_is_new(true);
					else
						diff_record->set_is_new(false);
					diff_record->set_index_from(from_id);
					diff_record->set_index_to(to_id);
					diff_record->set_mode(mode);
				    }
				is_special = true;
			    }
			}
			prefixPos = diff_line.find(HUNK_PREFIX);
			if (prefixPos != std::string::npos) {
			    std::string hunk_prefix= diff_line.substr(0, prefixPos);
			    if (hunk_prefix == "") {
				    if (hunk != NULL && hunk_text != "") {
					if (diff_record->is_code() != "") {
					    process_hunk_xml(hunk, hunk_text, diff_record->is_code()); 
					} else {
					    process_hunk_text(hunk, hunk_text);
					}
					hunk = NULL;
					hunk_text = "";
				    }
				    // @@ -4,10 +4,13 @@ BLABLA
				    std::string hunk_line = diff_line.substr(prefixPos + strlen(HUNK_PREFIX));
				    size_t from_Pos = hunk_line.find("-");
				    size_t to_Pos = hunk_line.find("+");
				    size_t context_Pos = hunk_line.find(HUNK_PREFIX);
				    hunk = diff_record->add_hunk();
				    if (from_Pos != std::string::npos) {
					std::string from_id = hunk_line.substr(from_Pos+1, to_Pos - 1);
					size_t col_Pos = from_id.find(",");
					std::string from_lineno = from_id.substr(0, col_Pos);
					std::string from_column = from_id.substr(col_Pos+1);
					hunk->set_from_lineno(atoi(from_lineno.c_str()));
					hunk->set_from_column(atoi(from_column.c_str()));
				    }
				    if (to_Pos != std::string::npos) {
					std::string to_id = hunk_line.substr(to_Pos+1, context_Pos - 1);
					size_t col_Pos = to_id.find(",");
					std::string to_lineno = to_id.substr(0, col_Pos);
					std::string to_column = to_id.substr(col_Pos+1);
					hunk->set_to_lineno(atoi(to_lineno.c_str()));
					hunk->set_to_column(atoi(to_column.c_str()));
				    }
				    if (context_Pos != std::string::npos) {
					std::string context = hunk_line.substr(context_Pos + strlen(HUNK_PREFIX));
					hunk->set_context(context);
				    }
				    is_special = true;
			    }
			}
			prefixPos = diff_line.find(SKIP_PREFIX1);
			if (prefixPos != std::string::npos) {
			    std::string diff_prefix= diff_line.substr(0, prefixPos);
			    if (diff_prefix == "") {
				    is_special = true;
			    }
			}
			prefixPos = diff_line.find(SKIP_PREFIX2);
			if (prefixPos != std::string::npos) {
			    std::string diff_prefix= diff_line.substr(0, prefixPos);
			    if (diff_prefix == "") {
				    is_special = true;
			    }
			}
			prefixPos = diff_line.find(SKIP_PREFIX3);
			if (prefixPos != std::string::npos) {
			    std::string diff_prefix= diff_line.substr(0, prefixPos);
			    if (diff_prefix == "") {
				    is_special = true;
			    }
			}
			prefixPos = diff_line.find(SKIP_PREFIX4);
			if (prefixPos != std::string::npos) {
			    std::string diff_prefix= diff_line.substr(0, prefixPos);
			    if (diff_prefix == "") {
				    is_special = true;
			    }
			}
			if (! is_special)
				hunk_text += diff_line + "\n";
		    }
		    /*
		    lineno ++;
		    if (lineno > 100) {
			    cout << "." << flush;
			    lineno = 0;
		    }
		    */
		} while (linePos != std::string::npos);
		diff = "";
	}
}

FILE *open_log_file(int jobs, int job, char *basename) {
	char filename[100];
	if (jobs != 1)
		sprintf(filename, "%s-%d.log", basename, job);
	else
		sprintf(filename, "%s.log", basename);
	FILE *file = fopen((char*) filename, "w");
	if (file == NULL) {
		cerr << "cannot allocate a file handle " << job << endl;
	}
	return file;
}

int main(int argc, char **argv) {
        GOOGLE_PROTOBUF_VERIFY_VERSION;
        fast::Log_Commit * current_commit = NULL;
	std::string line;
	std::string diff;
	std::string commit_id;
	std::string text;
	std::string author_name;
	std::string author_date;
	std::string author_email;
	std::string commit_name;
	std::string commit_date;
	std::string commit_email;
	std::string token;
	bool parallel = false;
	if (strcmp(argv[1], "-p") == 0) {
		parallel = true;
		argc-- ;
		argv++;
	}
	long number = 0;
	int jobs = 1;
	if (argc > 2) {
		jobs = atoi(argv[2]);
	}
	char buf[100];
	strcpy(buf, "temp.XXXXXX");
	mkstemp(buf);
	remove(buf);
	std::string fn = "/tmp/";
	fn += buf;
	remove(fn.c_str());
	strcat(buf, ".log");
	string log_filename = buf;
	FILE *input0 = fopen(log_filename.c_str(), "w");
        while(!std::cin.eof()){
		std::getline(std::cin, line);
		if (strcmp(line.c_str(), SEPARATOR)==0) {
			number++;
		}
		fprintf(input0, "%s\n", line.c_str());
	}
	fclose(input0);
	cout << "no. of records = " << number << endl;
	fstream input(log_filename.c_str(), ios::in);
	char filename[100];
	FILE *current_log_file = NULL;
        fast::Log * log = new fast::Log();
	int no = 0;
	if (parallel) {
		int job = 0;
		while(!input.eof()){
			std::getline(input, line);
			if (line == SEPARATOR) {
				if (no == (number+jobs-1)/jobs) {
					cout << "saved " << no << " records into " << argv[1] << "-" << job << ".log" << " ..." << endl;
					if (current_log_file != NULL)
						fclose(current_log_file);
					job++;
					current_log_file = open_log_file(jobs, job, argv[1]);
					no = 0;
				}
				no++;
			}
			if (current_log_file == NULL) {
				current_log_file = open_log_file(jobs, job, argv[1]);
			}
			fprintf(current_log_file, "%s\n", line.c_str());
		}
		if (no != 0) {
			cout << "saved " << no << " records into " << argv[1] << "-" << job << ".log" << " ..." << endl;
			if (current_log_file != NULL)
				fclose(current_log_file);
		}
		remove(log_filename.c_str());
		return 0;
	}
	int job = 0;
	// int lineno = 0;
        while(!input.eof()) {
		std::getline(input, line);
		if (strcmp(line.c_str(), SEPARATOR)==0) {
			// std::cout << "." << std::flush;
			if (no == (number+jobs-1)/jobs) {
				if (jobs != 1)
					sprintf(filename, "%s-%d.pb", argv[1], job);
				else
					sprintf(filename, "%s.pb", argv[1]);
				cout << "saving " << no << " records into " << filename << " ..." << endl;
				fstream output(filename, ios::out | ios::trunc | ios::binary);
				log->SerializeToOstream(&output);
				output.close();
				job++;
				log = new fast::Log();
				current_commit = NULL;
				no = 0;
			}
			no++;
			commit(current_commit, diff);
			std::getline(input, commit_id);
			std::getline(input, text);
			std::getline(input, author_name);
			std::getline(input, author_email);
			std::getline(input, author_date);
			std::getline(input, commit_name);
			std::getline(input, commit_email);
			std::getline(input, commit_date);
			fast::Log_Commit *commit = log->add_commit();
			commit->set_id(commit_id);
			commit->set_text(text);
			commit->set_author_date(author_date);
			int i = lookup_author(log, author_name, author_email);
			commit->set_author_id(i);
			i = lookup_author(log, commit_name, commit_email);
			if (i != commit->author_id()) {
				commit->mutable_committer()->set_committer_id(i);
				commit->mutable_committer()->set_commit_date(commit_date);
			}
			current_commit = commit;
		} else {
			diff += line;
			diff += "\n";
		}
	}
	remove(log_filename.c_str());
	if (jobs != 1)
		sprintf(filename, "%s-%d.pb", argv[1], job);
	else
		sprintf(filename, "%s.pb", argv[1]);
	if (no != 0) {
		commit(current_commit, diff);
		cout << "saving " << no << " records into " << filename << " ..." << endl;
		fstream output(filename, ios::out | ios::trunc | ios::binary);
		log->SerializeToOstream(&output);
		output.close();
	}
        google::protobuf::ShutdownProtobufLibrary();
	return 0;
}
