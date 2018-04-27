#include <iostream>
#include <boost/numeric/ublas/matrix_sparse.hpp>
#include <boost/numeric/ublas/io.hpp>
#include <boost/numeric/ublas/storage.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/archive/binary_iarchive.hpp>
#include <fstream>


namespace ublas = boost::numeric::ublas;
namespace po = boost::program_options;
using namespace std;

struct Options {
    int dim = 2;
    string input;
    string matrix;
    bool print_chains = false;
} options;

struct Stats {
    int words_number = 0;           // Nubmer of all words in the analysed text
    int unique_words_number = 0;    // Nubmer of unique words in the analysed text
    int max_number = 0;             // Max number of one unique word found in the analysed text
    std::map<int, int> sentence_len;       // number of words in sentences depending of sentence length
} stats;

typedef std::map<string, int> Dictionary;
typedef ublas::compressed_matrix<int> Matrix;




int parse_command_line(int ac, char* av[], Options & options) {

    try {

        po::options_description desc("Allowed options");
        desc.add_options()
                ("help", "produce help message")
                ("dim", po::value<int>(), "set dimension of the matrix")
                ("input", po::value<string>(), "set input file for learning")
                ("matrix", po::value<string>(), "set output file for saving of the matrix")
                ("print_matrix", "print the matrix (for dimension = 2 only)")
                ("print_chains", "print the chains")
                ;

        po::variables_map vm;
        po::store(po::parse_command_line(ac, av, desc), vm);
        po::notify(vm);

        if ((ac == 1) || vm.count("help")) {
            cout << desc << endl;
            exit(0);
        }

        if (vm.count("print_chains")) {
            options.print_chains = true;
        }


        if (vm.count("input")) {
            options.input = vm["input"].as<string>();
        }

        if (vm.count("dim")) {
            options.dim = vm["dim"].as<int>();
        }

        cout << "Dimension of the matrix = " << options.dim << endl;
    }
    catch(exception& e) {
        cerr << "error: " << e.what() << endl;
        exit(1);
    }
    catch(...) {
        cerr << "Exception of unknown type!\n";
    }
}

const std::string & read_file(const std::string & file_name, std::string & out) {
    std::ifstream ifs(file_name);
    if( ! ifs.is_open()) {
        std::cerr << "can't open input file: " << file_name << endl;
    }
    out.assign((std::istreambuf_iterator<char>(ifs)),
               std::istreambuf_iterator<char>());
    return out;
}

bool remove_predicate(const unsigned char c) {
    // needed to avoid undefined behavior
    // static_cast<unsigned char>(ch);

    return ! (isalpha(c) && isblank(c) && (c == '.') );

}

const std::string & clean_text(std::string & str) {
    str.erase(std::remove_if(str.begin(), str.end(), remove_predicate),
              str.end());
    return str;
}

void print_dictionary(const Dictionary & dictionary) {
    int word_num = 0;
    for(const auto & p : dictionary) {
        cout << p.first << " " << p.second << endl;
        word_num += p.second;
    }
    cout << "Unique words: " << dictionary.size() << endl;
    cout << "Total words: " << word_num << endl;
}

void fill_dictionary(const std::string & str, Dictionary & dictionary) {

    int number_of_words = 0;
    typedef boost::tokenizer<> tokenizer;
    tokenizer tok{str};

    for (const auto &t : tok) {
        number_of_words++;
//        std::cout << t << '\n';
        // add a new pair <word, number of this word in the text>
        auto ret = dictionary.insert({t,1});
        // if the word is already in the dictionary increase its number
        if(ret.second == false) {
            ret.first->second++;
        }
    }

    if(number_of_words > 0) {
        stats.words_number += number_of_words;
        // add a new pair <number of words in sentence, number of sentences with this number of words>
        auto ret = stats.sentence_len.insert({number_of_words, 1});
        // if this number of words is already in the dictionary increase the number sentences
        if (ret.second == false) {
            ret.first->second++;
        }
    }
}

void print_stats() {

    /*
     *     int words_number = 0;           // Number of all words in the analysed text
    int unique_words_number = 0;    // Number of all unique words in the analysed text
    int max_number = 0;             // Max number of one unique word found in the analysed text
    std::map<int, int> sentence_len;       // number of words in sentences depending of sentence length
     */
    cout << "Statistics:" << endl
         << stats.words_number << ": Number of all words in the text" << endl
         << stats.unique_words_number << ": Number of unique words in the text" << endl
         << stats.max_number << ": Max number of one unique word found in the analysed text" << endl << endl
         << "Number of words in sentences depending of sentence length: <words : sentences>" << endl;
    for (const auto & t : stats.sentence_len) {
        std::cout << t.first << " : "
                  << t.second << endl;
    }
}

void save_matrix(const Matrix & m, const string & file_name) {
    std::ofstream ofs(file_name);
    boost::archive::binary_oarchive oarch(ofs);
    oarch << m;
    // m.serialize(oarch, 1);
}

void load_matrix(Matrix & m, const string & file_name) {
    std::ifstream ifs(file_name);
    boost::archive::binary_iarchive iarch(ifs);
    iarch >> m;
}

void show_array(Matrix::value_array_type & a)  {

    for(const auto &element : a ) {
        std::cout << element << ' ';
    }
    cout << endl;
}


int main(int ac, char* av[]) {

//    setlocale(LC_CTYPE, "Russian_Russia.1251");

    Options options;
    Dictionary dictionary;

    parse_command_line(ac, av, options);

    if(options.input.empty()) {
        cout << "set --input file" << endl;
    }

    std::string the_text;
    read_file(options.input, the_text);

    typedef boost::tokenizer<boost::char_separator<char>> tokenizer;
    boost::char_separator<char> sep(".?!;");
    tokenizer tok{the_text, sep};
    for (const auto &t : tok) {
//        std::cout << t << '\n';
        fill_dictionary(t, dictionary);
    }

    stats.unique_words_number = dictionary.size();

    for(const auto &pair : dictionary ) {
        stats.max_number = std::max(stats.max_number, pair.second);
    }



//    print_dictionary(dictionary);

    print_stats();


/*

    clean_text(text);

    //convert the text to lower case
    std::transform(text.begin(), text.end(), text.begin(), ::tolower);

    cout << text;
*/

    Matrix m (10, 10);
    Matrix m2;

    m(0, 5) = 1; // underlying array is {1, 0, 0, 0, ...}
    show_array(m.value_data());
    m(0, 6) = 2; // underlying array is {1, 2, 0, 0, ...}
    show_array(m.value_data());
    m(0, 4) = 3;  // underlying array is {3, 1, 2, 0, ...}
    show_array(m.value_data());
    m(0, 4) = 7;  // underlying array is {7, 1, 2, 0, ...}
    show_array(m.value_data());

    std::cout << m << std::endl;

    std::cout << "Save matrix" << std::endl;

    save_matrix(m, "matrix.bin");

    std::cout << "Load matrix" << std::endl;

    load_matrix(m2, "matrix.bin");

    std::cout << m2 << std::endl;

    return 0;
}
