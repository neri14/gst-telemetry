#include <exprtk.hpp>
#include <iostream>

// Example usage of ExprTk in your project
// This shows how to include and use the ExprTk library

int main() {
    typedef exprtk::symbol_table<double> symbol_table_t;
    typedef exprtk::expression<double>   expression_t;
    typedef exprtk::parser<double>       parser_t;

    std::string expression_string = "2 * x + 3";
    
    double x = 5.0;
    
    symbol_table_t symbol_table;
    symbol_table.add_variable("x", x);
    
    expression_t expression;
    expression.register_symbol_table(symbol_table);
    
    parser_t parser;
    parser.compile(expression_string, expression);
    
    double result = expression.value();
    std::cout << "Result: " << result << std::endl;  // Should output 13
    
    return 0;
}
