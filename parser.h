#ifndef __PARSER_H__
#define __PARSER_H__

#include <string>
#include <sstream> // Required for istringstream
#include <set> // Required for std::set
#include <algorithm>
#include <cmath>
#include <functional>
#include <vector>
#include <map>
#include <cctype>
#include <iostream>

#include "lexer.h"


struct poly_header_t {
    Token name;
    std::vector<std::string> id_list;
    
};
struct poly_body_t {
  std::string body;
  int degree;
};


struct poly_decl_t {
    poly_header_t header;
    poly_body_t body;
};

class Parser {
  public:
    void ConsumeAllInput();


  private:
    LexicalAnalyzer lexer;
    void syntax_error();
    Token expect(TokenType expected_type);
    int error_n;
    std::vector<int> errors;
    // Parsing functions
    void parse_program();
    void parse_tasks_section();
    void parse_poly_section();
    void parse_poly_decl_list();
    poly_decl_t parse_poly_decl();
    poly_header_t parse_poly_header();
    std::vector<std::string> parse_id_list();
    poly_body_t parse_poly_body(poly_header_t,bool);
    void parse_execute_section();
    void parse_statement();
    void parse_input_statement();
    void parse_output_statement();
    void parse_assignment_statement();
    poly_decl_t* findPolyByHeader(std::string name);
    std::string parse_poly_evaluation();
    void parse_argument_list();
    void parse_inputs_section();
    bool findinTask(int);
    void execute_program();
    int evaluate_polynomial(const std::string& body);
    // Additional helper functions
    
};

#endif // __PARSER_H__
