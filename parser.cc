
#include "parser.h"

using namespace std;

// Global data structures
vector<poly_decl_t> polynomials; //storing polys golbally its in form of dlec
map<string, int> variable_values;
vector<string> execution_order;
vector<int> tasks_to_perform;
map<string,int> inputs;
std::string warning_line;
vector<int> unused_var_line;
map<string,int> left_var;

void Parser::syntax_error() {
    cout << "SYNTAX ERROR !!!!!&%!!\n";
    exit(1);
}

Token Parser::expect(TokenType expected_type) {
    Token t = lexer.GetToken();
    
    if (t.token_type != expected_type){
        syntax_error();
    }
    return t;
}

void Parser::ConsumeAllInput() {
    parse_program();
    if ((error_n == 1) && findinTask(1)){
        cout << "Semantic Error Code 1: ";
    }
    if ((error_n == 2) && findinTask(1)){
        cout << "Semantic Error Code 2: ";
    }
    if ((error_n == 3 && findinTask(1))){
        cout << "Semantic Error Code 3: ";
    }
    if ((error_n == 4) && findinTask(1)){
        cout << "Semantic Error Code 4: ";
    }
    if (error_n == 1 || error_n == 2 || error_n == 3 || error_n == 4){
        if (findinTask(1)){std::sort(errors.begin(), errors.end());
        for (int line : errors) {
            cout << line << " ";
        }
        cout << endl;
    }
        exit(1);
    }
    execute_program();
}

void Parser::parse_program() {
    parse_tasks_section();
    parse_poly_section();
    parse_execute_section();
    parse_inputs_section();
}

bool Parser::findinTask(int task){
    auto it = std::find(tasks_to_perform.begin(),tasks_to_perform.end(),task);
    return (it!=tasks_to_perform.end());
}

void Parser::parse_tasks_section() {
    expect(TASKS);
    if (lexer.peek(1).token_type == POLY){syntax_error();}      
    while (lexer.peek(1).token_type == NUM)
        {
            int task = stoi(expect(NUM).lexeme);
            if (!(findinTask(task))){
                tasks_to_perform.push_back(task);
            }
        }
}

void Parser::parse_poly_section() {
    expect(POLY);
    if (lexer.peek(1).token_type == EXECUTE ){syntax_error();}
    parse_poly_decl_list();
}

void Parser::parse_poly_decl_list() {
    // Static map to track first occurrences of each polynomial name
    static std::map<std::string, int> poly_first_occurrence;
    static std::vector<int> duplicate_lines;
    // Parse the current polynomial declaration
    poly_decl_t decl = parse_poly_decl();
    std::string poly_name = decl.header.name.lexeme;
    int line_number = decl.header.name.line_no;

    // Check if polynomial name already exists
    if (poly_first_occurrence.find(poly_name) != poly_first_occurrence.end()) {
        // Add all occurrences of duplicates
        duplicate_lines.push_back(line_number);
    } else {
        // Store the first occurrence of this polynomial name
        poly_first_occurrence[poly_name] = line_number;
    }

    // Store the polynomial in the global list
    polynomials.push_back(decl);

    // Continue parsing if more polynomial declarations exist
    if (lexer.peek(1).token_type == ID) {
        parse_poly_decl_list();
     }

    else if (lexer.peek(1).token_type == EXECUTE) {
        if (!duplicate_lines.empty()) {
            
            if (findinTask(1)){
            error_n = 1;
            errors = duplicate_lines;
        }    
    }
    
}}


poly_decl_t Parser::parse_poly_decl() {
    // cout << "parse_poly_decl" << endl;
    poly_decl_t decl;
    decl.header = parse_poly_header();
    expect(EQUAL);
    decl.body = parse_poly_body(decl.header, false);
    expect(SEMICOLON);
    return decl;
}

poly_header_t Parser::parse_poly_header() {
    // cout << "parse_poly_header" << endl;
    poly_header_t header;
    header.name = expect(ID);
    
    // Continue parsing parameters if present
    Token t = lexer.peek(1);
    if (t.token_type == LPAREN) {
        expect(LPAREN);
        header.id_list = parse_id_list();
        expect(RPAREN);
    } else {
        header.id_list.push_back("x");
    }

    return header;
}


vector<string> Parser::parse_id_list() {
    vector<string> id_list;
    id_list.push_back(expect(ID).lexeme);
    
    while (lexer.peek(1).token_type == COMMA) {
        expect(COMMA);
        id_list.push_back(expect(ID).lexeme);
    }
    return id_list;
}


poly_body_t Parser::parse_poly_body(poly_header_t header, bool into_task) {
    
    poly_body_t body;
    static std::vector<int> error_lines;
    int inparan = 0;
    std::vector<int> degree;
    int degree_Index = 0;
    if (into_task){
        inparan = 1;
    }
    if (!(lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM || lexer.peek(1).token_type == LPAREN)){
        syntax_error();
    }
    while (lexer.peek(1).token_type != SEMICOLON) {
        if (lexer.peek(1).token_type == EQUAL){
            syntax_error();
        }
        // Check for invalid assignments within polynomial definitions
        if (lexer.peek(1).token_type == ID && lexer.peek(2).token_type == EQUAL) {
            syntax_error();
        }

        // Track parenthesis balance
        if (lexer.peek(1).token_type == LPAREN) {
            inparan++;
        } 
        if (lexer.peek(1).token_type == RPAREN) {
            if (lexer.peek(2).token_type == ID || lexer.peek(2).token_type == NUM) {
                syntax_error();
            }
            inparan--;
        }
        
        // Ensure valid token sequence
        if (lexer.peek(1).token_type == RPAREN && (lexer.peek(2).token_type == NUM || lexer.peek(2).token_type == ID)) {
            syntax_error();
        }
        
    
        // Get the current token
        Token t = lexer.GetToken();
        
        
        // Handle implicit multiplication: e.g., "2 Y" should be "2 * Y"
        if ((t.token_type == NUM && lexer.peek(1).token_type == ID) || 
            (t.token_type == ID && lexer.peek(1).token_type == ID) || 
            (t.token_type == NUM && lexer.peek(1).token_type == LPAREN) || 
            (t.token_type == ID && lexer.peek(1).token_type == LPAREN)) {
            body.body += " " + t.lexeme + " * ";

            if (t.token_type == ID) {
                if (degree_Index >= degree.size()) {
                    degree.push_back(1);
                }else{

                    degree[degree_Index] += 1;
                }
            }    
                
            
        } else {
            if (t.token_type == PLUS) {
                if (!(lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM || lexer.peek(1).token_type == LPAREN)) {
                    syntax_error();
                }
                body.body += " + ";
                degree_Index++;
            } else if (t.token_type == MINUS) {
                if (!(lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM || lexer.peek(1).token_type == LPAREN)) {
                    syntax_error();
                }
                body.body += " - ";
                degree_Index++;
            } else if (t.token_type == POWER) {
                if (!(lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM || lexer.peek(1).token_type == LPAREN)) {
                    syntax_error();
                }
                
                int exponent = std::stoi(expect(NUM).lexeme);
                body.body += " ^ " + std::to_string(exponent);
                if (lexer.peek(1).token_type == POWER || lexer.peek(1).token_type == NUM){
                    syntax_error();
                }

                if (degree_Index >= degree.size()) {
                    degree.push_back(exponent);
                } else {
                    degree[degree_Index] += exponent - 1;
                }
                
            } else if (t.token_type == LPAREN) {
                if (!(lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM || lexer.peek(1).token_type == LPAREN)){ 
                    syntax_error();
                }
                body.body += " ( ";
                poly_body_t temp = parse_poly_body(header,true);
                if (lexer.peek(1).token_type == ID || lexer.peek(1).token_type == NUM) {
                    syntax_error();
                }
                inparan--;
                body.body += temp.body;
                
                if (degree_Index >= degree.size()) {
                    degree.push_back(0);
                }
                if (lexer.peek(1).token_type == POWER){
                    expect(POWER);
                    int exponent = std::stoi(expect(NUM).lexeme);
                    degree[degree_Index] += temp.degree * exponent;
                    body.body += " ^ " + std::to_string(exponent);
                }else{
                degree[degree_Index] += temp.degree;
                }
            } else if (t.token_type == RPAREN) {
                body.body += " ) ";
                if (into_task){
                 break;
                }
            } else if (t.token_type == ID || t.token_type == NUM) {
                
                body.body += " " + t.lexeme;
                if (t.token_type == ID) {
                    if (lexer.peek(1).token_type == NUM){
                        syntax_error();
                    }
                    if (degree_Index >= degree.size()) {
                        degree.push_back(1);  // Initialize term's degree
                    } else {
                        degree[degree_Index] += 1;
                    }
                }else{
                    if (lexer.peek(1).token_type == POWER || lexer.peek(1).token_type == NUM){
                        syntax_error();
                    }
                }
            }else{
                syntax_error();
            }
        }

        // Check if the variable is declared in the header
        if (std::find(header.id_list.begin(), header.id_list.end(), t.lexeme) == header.id_list.end() 
            && t.token_type == ID) {
            error_lines.push_back(t.line_no);
        }
    }
    // Check if parentheses are balanced
    if (inparan != 0) {
        
        syntax_error();
    }

    // Handle semantic errors (undeclared variables)
    if (!error_lines.empty() && lexer.peek(2).token_type == EXECUTE) {
        if (findinTask(1)) {
            std::sort(error_lines.begin(), error_lines.end());
            error_n = 2;
            errors = error_lines;
            
        }
    }

    // Determine highest degree among terms
    int high_power = (degree.empty()) ? 0 : degree[0];
    for (int i = 1; i < degree.size(); i++) {
        if (degree[i] > high_power) {
            high_power = degree[i];
        }
    }
    body.degree = high_power;

    cout << "returned : " << body.body << endl;
    return body;
}

void Parser::parse_execute_section() {
    
    expect(EXECUTE);
    execution_order.clear();
    if (lexer.peek(1).token_type == INPUTS){syntax_error();}
    cout << "parse_execute_section" << endl;
    while (lexer.peek(1).token_type != INPUTS)
        {
            parse_statement();
            
        }
    
}

void Parser::parse_statement() {
    //cout << "parse_statement" << endl;
    Token t = lexer.peek(1);
    if (t.token_type == INPUT)
        parse_input_statement();
    else if (t.token_type == OUTPUT)
        parse_output_statement();
    else if (t.token_type == ID)
        parse_assignment_statement();
    else{
        syntax_error(); }
}

void Parser::parse_input_statement() {
    //cout << "parse_input_statement" << endl;
    expect(INPUT);
    Token var = expect(ID);
    if (variable_values.find(var.lexeme) == variable_values.end()) {
        variable_values[var.lexeme] = 0;
    }
    if (inputs.find(var.lexeme) == inputs.end()){
        inputs[var.lexeme] = 0;
    }

    expect(SEMICOLON);

    if(left_var.find(var.lexeme) != left_var.end()){
        unused_var_line.push_back(left_var[var.lexeme]);
        }
        
        // left_var[var.lexeme] = var.line_no;
        left_var.erase(var.lexeme);

    execution_order.push_back("INPUT " + var.lexeme + ";");
}


void Parser::parse_output_statement() {
    //cout << "parse_output_statement" << endl;
    expect(OUTPUT);
    Token var = expect(ID);
    expect(SEMICOLON);
    left_var.erase(var.lexeme);
    execution_order.push_back("OUTPUT " + var.lexeme + ";");
}


void Parser::parse_assignment_statement() {
    
    Token var = expect(ID);
    expect(EQUAL);
    std::string fullbody = var.lexeme + " = ";
    // Ensure variable exists in the symbol table before assignment
    
    if (lexer.peek(1).token_type == ID && lexer.peek(2).token_type == LPAREN) {
        fullbody += " " + parse_poly_evaluation();
    }


    expect(SEMICOLON);
    if (variable_values.find(var.lexeme) == variable_values.end()) {
        variable_values[var.lexeme] = 0;
    }

    if(left_var.find(var.lexeme) != left_var.end()){
    unused_var_line.push_back(left_var[var.lexeme]);
    }
    
    left_var[var.lexeme] = var.line_no;
    cout <<" body:"<< fullbody << endl;
    execution_order.push_back(fullbody + ";");
}

// Returns a pointer to the matching poly_decl_t if found, or nullptr otherwise.
poly_decl_t* Parser::findPolyByHeader(std::string name) {
    auto it = std::find_if(polynomials.begin(), polynomials.end(),[&](const poly_decl_t &p) {
                               return p.header.name.lexeme == name;
                           });
    return (it != polynomials.end()) ? &(*it) : nullptr;
}


std::string Parser::parse_poly_evaluation() {
    Token fpoly = expect(ID);
    expect(LPAREN);
    std::vector<std::string> args;
    std::string exbody;

    // Find the function declaration (poly) by header.
    poly_decl_t* foundpoly = findPolyByHeader(fpoly.lexeme);    
    
    if (foundpoly == nullptr) {
        error_n = 3;
        errors.push_back(fpoly.line_no);
    }
    
    // Parse first argument: it can be a function call or a simple ID/NUM.
    if (lexer.peek(1).token_type == ID && lexer.peek(2).token_type == LPAREN) {
        // Recursively evaluate the function call and add its result.
        args.push_back(parse_poly_evaluation());
    } else {
        if (lexer.peek(1).token_type == ID) {
            Token var = expect(ID);
            if (variable_values.find(var.lexeme) == variable_values.end()){
                warning_line += " " + std::to_string(var.line_no);
            }else{
                left_var.erase(var.lexeme);
            }
            args.push_back(var.lexeme);
            
        } else {
            args.push_back(expect(NUM).lexeme);
        }
    }
    
    // Continue parsing additional arguments separated by commas.
    while (lexer.peek(1).token_type == COMMA) {
        expect(COMMA);
        if (lexer.peek(1).token_type == ID && lexer.peek(2).token_type == LPAREN) {
            args.push_back(parse_poly_evaluation());
        } else {
            if (lexer.peek(1).token_type == ID) {
                Token var = expect(ID);
            if (variable_values.find(var.lexeme) == variable_values.end()){
                warning_line += " " + std::to_string(var.line_no);
            }else{
                left_var.erase(var.lexeme);
            }
            args.push_back(var.lexeme);
            } else {
                args.push_back(expect(NUM).lexeme);
            }
        }
    }
    
    // Check if the number of arguments matches the function declaration.
    if (foundpoly != nullptr && args.size() != foundpoly->header.id_list.size() && error_n != 3) {
        error_n = 4;
        errors.push_back(fpoly.line_no);
    } 
    
    expect(RPAREN);
    
    // Build the output string representation.
    exbody = fpoly.lexeme + "(";
    for (size_t i = 0; i < args.size(); i++) {
        exbody += args[i];
        if (i != args.size() - 1) {
            exbody += ", ";
        }
    }
    exbody += ")";
    cout << "exbody: " << exbody<< endl;
    return exbody;
}



void Parser::parse_argument_list() {
    std::vector<std::string> args;
    args.push_back(expect(ID).lexeme);

    while (lexer.peek(1).token_type == COMMA) {
        expect(COMMA);
        args.push_back(expect(ID).lexeme);
    }

    // Verify argument count matches declaration
    Token poly = lexer.peek(-2);
    for (const auto& p : polynomials) {
        if (p.header.name.lexeme == poly.lexeme) {
            error_n = 4;
            errors.push_back(poly.line_no);
        }
    }
}

void Parser::parse_inputs_section() {
    expect(INPUTS);
    for (auto& x : inputs){
        x.second = std::stoi(expect(NUM).lexeme);
    }
    while (lexer.peek(1).token_type == NUM)
        {
            expect(NUM);
        }
        
    expect(END_OF_FILE);
}
// Helper to remove spaces from a string (for left-hand side, etc.)
static inline std::string removeSpaces(const std::string &s) {
    std::string result;
    std::copy_if(s.begin(), s.end(), std::back_inserter(result), [](char c){ return !std::isspace(c); });
    return result;
}

void Parser::execute_program() {
    for (const auto &stmt_raw : execution_order) {
        // For debugging, you can print the statement:
        std::cout << "Processing: " << stmt_raw << std::endl;
        // Check if the statement is an INPUT statement.
        cout << "stmt_raw: " << stmt_raw.find("=") << endl;
        
        if (stmt_raw.find("INPUT") == 0) {
            // Expected format: "INPUT a;"
            size_t spacePos = stmt_raw.find(" ");
            if (spacePos == std::string::npos) continue;
            std::string var = stmt_raw.substr(spacePos + 1);
            // Remove the trailing semicolon.
            if (!var.empty() && var.back() == ';')
                var.pop_back();
            // Assign the next input value.
            if (!inputs.empty()) {
                variable_values[var] = inputs.begin()->second;
                inputs.erase(inputs.begin());
            } else {
                variable_values[var] = inputs.find(var)->second;
            }
            // cout << "input " << var << " " << variable_values[var] << endl;
        }
        // Check if the statement is an OUTPUT statement.
        else if ((stmt_raw.find("OUTPUT") == 0) && findinTask(2)) {
            // Expected format: "OUTPUT w;"
            size_t spacePos = stmt_raw.find(" ");
            if (spacePos == std::string::npos) continue;
            std::string var = stmt_raw.substr(spacePos + 1);
            if (!var.empty() && var.back() == ';')
                var.pop_back();
            std::cout << variable_values[var] << std::endl;
        }
        // Otherwise, assume it’s an assignment statement.
        else if (stmt_raw.find("=") != std::string::npos) {
            cout << " = " << endl;
            // Expected format: "w = F(a);"
            size_t eqPos = stmt_raw.find("=");
            std::string leftVar = stmt_raw.substr(0, eqPos);
            leftVar = removeSpaces(leftVar);
            std::string rhs = removeSpaces(stmt_raw.substr(eqPos + 1));
            // Remove trailing semicolon.
            cout << "rhs: " << rhs << endl;
            if (!rhs.empty() && rhs.back() == ';') {
                rhs.pop_back();
            }
            cout << ": " << rhs << endl;
            // Now,  rhs should be "F(a)"
            // Assume rhs is something like "F(a, 3, b)" and removeSpaces is defined.
            int numberoff = 0;
            std::string sub ="(";
            size_t pos = rhs.find(sub, 0);
            cout << "4" << endl;
            while (pos != std::string::npos) {
                numberoff++;
                pos = rhs.find(sub, pos + sub.length());
            }
            cout << "5number : " << numberoff << endl;

            int i = 0;
            
            while (i < numberoff){
                cout << "i: " << i << endl;
                size_t parenPos = rhs.find_last_of("(");
                cout << "6" << endl;
                if (parenPos == std::string::npos) break;
                
                // cout << rhs << endl;
                size_t last_comma = -1;
                if (i != numberoff-1){
                    last_comma = rhs.find_last_of(",");
                }
                cout << "7" << endl;
                std::string polyName = rhs.substr(last_comma+1, parenPos-last_comma-1);
                polyName = removeSpaces(polyName);
                cout << "8" << endl;
                size_t endParen = rhs.find(")", parenPos);
                if (endParen == std::string::npos) continue;

                std::string argStr = rhs.substr(parenPos + 1, endParen - parenPos - 1);
                argStr = removeSpaces(argStr);

                std::vector<int> argValues;

            // Use a loop to split the argument string by commas.
            cout << "9" << endl;
            size_t start = 0;
            size_t pos = argStr.find(",");
            cout << "10" << endl;
            while (pos != std::string::npos) {
                // Extract the token from the current segment.
                cout << "12" << endl;
                std::string token = argStr.substr(start, pos - start);
                token = removeSpaces(token);
                if (!token.empty()){
                    try {
                        int value = std::stoi(token);
                        argValues.push_back(value);
                    } catch (...) {
                        argValues.push_back(variable_values[token]);
                    }
                }
                start = pos + 1;
                pos = argStr.find(",", start);
            }

            // Process the last token (or the whole string if there were no commas).
            std::string token = argStr.substr(start);
            token = removeSpaces(token);
            if (!token.empty()){
                try {
                    int value = std::stoi(token);
                    argValues.push_back(value);
                } catch (...) {
                    argValues.push_back(variable_values[token]);
                }
            }

            
            // Use the dummy token to find the polynomial.
            poly_decl_t* foundedPoly = findPolyByHeader(polyName);
            if (foundedPoly == nullptr) {
                i++;
                continue;
            };
            // Evaluate the polynomial.
            std::string body = foundedPoly->body.body;
            // Replace the variable with the argument value.
            
            for (size_t i = 0; i < foundedPoly->header.id_list.size(); ++i) {
                const std::string &header = foundedPoly->header.id_list[i];
                int currentArgValue = argValues[i]; // assuming argValues has the same number of elements
                size_t pos = 0;
                while ((pos = body.find(header, pos)) != std::string::npos) {
                    body.replace(pos, header.size(), std::to_string(currentArgValue));
                    pos += std::to_string(currentArgValue).size();
                }
            }
            // Evaluate the polynomial.
            cout << body << endl;
            int result = evaluate_polynomial(body);
            // Assign the result to the left-hand side variable.
            variable_values[leftVar] = result;
            rhs.replace(last_comma + 1, endParen - last_comma, std::to_string(result));
            i++;
        }
    }
    cout << " 2 " << endl;
    if (&stmt_raw == &execution_order.back()) {
        
        if ((warning_line.size() != 0) && (findinTask(3))){
            cout << "Warning Code 1:" << warning_line<<endl;
        }

        if ((unused_var_line.size() != 0 ) && (findinTask(4))){
            for ( auto &pair : left_var) {            
                unused_var_line.push_back(pair.second);
            }
            std::sort(unused_var_line.begin(),unused_var_line.end());
            cout << "Warning Code 2:";
            for (int i = 0; i < unused_var_line.size();i++){
                cout << " " << unused_var_line[i];
            }
            cout << endl;
        }
        if (findinTask(5)){

            for (auto &poly : polynomials) {
                cout << poly.header.name.lexeme << " : " << poly.body.degree << endl;
            }
        }
    }
}
}

// int Parser::evaluate_polynomial(const std::string& body) {

//     std::istringstream ss(body);

//     // Forward declarations for the lambda functions
//     std::function<int()> parseExpression, parseTerm, parseFactor, parsePrimary;

//     // parsePrimary: numbers or parenthesized expressions
//     parsePrimary = [&]() -> int {
//         ss >> std::ws; // Skip any whitespace
//         if (ss.peek() == '(') {
//             ss.get();  // Consume '('
//             int value = parseExpression();
//             ss >> std::ws;
//             if (ss.peek() == ')') {
//                 ss.get();  // Consume ')'
//             }
//             return value;
//         } else {
//             int number;
//             ss >> number;
//             return number;
//         }
//     };

//     // parseFactor: handles exponentiation (right-associative)
//     parseFactor = [&]() -> int {
//         int left = parsePrimary();
//         ss >> std::ws;
//         while (ss.peek() == '^') {
//             ss.get(); // Consume '^'
//             int exponent = parseFactor(); // Right-associative: parse the exponent recursively
//             left = std::pow(left, exponent);
//             ss >> std::ws;
//         }
//         return left;
//     };

//     // parseTerm: handles multiplication and division
//     parseTerm = [&]() -> int {
//         int left = parseFactor();
//         ss >> std::ws;
//         while (ss.peek() == '*') {
//             char op = ss.get();
//             int right = parseFactor();
//             if (op == '*') {
//                 left *= right;
//             }
//             ss >> std::ws;
//         }
//         return left;
//     };

//     // parseExpression: handles addition and subtraction
//     parseExpression = [&]() -> int {
//         int left = parseTerm();
//         ss >> std::ws;
//         while (ss.peek() == '+' || ss.peek() == '-') {
//             char op = ss.get();
//             int right = parseTerm();
//             if (op == '+') {
//                 left += right;
//             } else { // op == '-'
//                 left -= right;
//             }
//             ss >> std::ws;
//         }
//         return left;
//     };

//     return parseExpression();
// }

int Parser::evaluate_polynomial(const std::string& body) {
    cout << "body: " << body << endl;
    std::istringstream ss(body);

    // Forward declarations
    std::function<int()> parseExpression, parseTerm, parseFactor, parsePrimary;

    // parsePrimary: numbers or parenthesized expressions
    parsePrimary = [&]() -> int {
        ss >> std::ws; // skip any whitespace
        if (ss.peek() == '(') {
            ss.get(); // consume '('
            int value = parseExpression();
            ss >> std::ws;
            if (ss.peek() == ')') {
                ss.get(); // consume ')'
            }
            return value;
        } else {
            int number;
            ss >> number;
            return number;
        }
    };

    // parseFactor: handles exponentiation (right-associative)
    parseFactor = [&]() -> int {
        int left = parsePrimary();
        ss >> std::ws;
        while (ss.peek() == '^') {
            ss.get(); // consume '^'
            int exponent = parseFactor(); // recursion for right-associative exponent
            left = static_cast<int>(std::pow(left, exponent));
            ss >> std::ws;
        }
        return left;
    };

    // parseTerm: handles multiplication (left-associative)
    parseTerm = [&]() -> int {
        int left = parseFactor();
        ss >> std::ws;
        while (ss.peek() == '*') {
            ss.get(); // consume '*'
            int right = parseFactor();
            left *= right;  // left-associative for '*'
            ss >> std::ws;
        }
        return left;
    };

    // parseExpression: handles + and - (forcing minus to be right-associative)
    //
    //  - We parse one term (left)
    //  - If the next token is '+' or '-', we parse the *entire rest* of the expression (right)
    //  - Then combine left and right according to the operator
    //
    // This approach groups the right side first for '-',
    // so something like 1 - 2 - 3 is parsed as 1 - (2 - 3) = 2.
    //
    parseExpression = [&]() -> int {
        int left = parseTerm();
        ss >> std::ws;

        // Check if next token is '+' or '-'
        if (ss.peek() == '+' || ss.peek() == '-') {
            char op = ss.get(); // consume the operator
            int right = parseExpression(); // parse everything to the right (recursively)

            if (op == '+') {
                return left + right;  // for plus, right- vs left-associative doesn’t matter
            } else { // op == '-'
                return left - right;  // minus is now right-associative
            }
        }

        // If no '+' or '-' is next, just return the single term
        return left;
    };

    return parseExpression();
}

int main() {
    Parser parser;
    parser.ConsumeAllInput();
    return 0;
}