#ifndef VERO_H
#define VERO_H

#include <iostream>
#include <string>
#include <vector>
#include <cmath>
#include <map>
#include <algorithm>
#include <cstdint>
#include <tuple>
#include <fstream>
#include <sstream>

std::string run_script_cpp(std::string script);
void initialize_global_symbol_table();

#ifdef __EMSCRIPTEN__
#include <emscripten/emscripten.h>
#endif

#ifdef __EMSCRIPTEN__
extern "C" {
    EMSCRIPTEN_KEEPALIVE
    void init_global_symbol_table() {
        ::initialize_global_symbol_table();
    }
    
    const char* run_script(const char* script) {
        std::string result = run_script_cpp(std::string(script));
        // Need to return a persistent string
        char* output = (char*)malloc(result.length() + 1);
        strcpy(output, result.c_str());
        return output;
    }
}
#endif

bool is_digit(char c);
bool is_letter(char c);
bool is_letter_or_digit(char c);

// Forward declarations
class Value;
class Number;
class Function;
class RuntimeResult;
class FuncDefNode;
class Node;
class Error;

// ************POSITION************

class Position {
    public:
        int index;
        int line;
        int col;
        std::string file_name;
        std::string file_text;
        Position();
        Position(int index, int line, int col, std::string file_name, std::string file_text);
        Position copy();
        void advance(char current_char='\0');
};

// ************SYMBOL TABLE************

class SymbolTable {
    public:
        SymbolTable* parent;
        std::map<std::string, Value*> symbols;
        SymbolTable(SymbolTable* parent = nullptr);
        Value* get(std::string name);
        void set(std::string name, Value* value);
        void remove(std::string name);
};

void initialize_global_symbol_table();

// ************CONTEXT************

class Context {
    public:
        std::string display_name;
        Context* parent;
        Position parent_entry_pos;
        SymbolTable symbol_table;
        Context();
        Context(std::string display_name, Context* parent = nullptr, Position parent_entry_pos = Position());
};

// ************ERRORS************

class Error {
    public:
        Position pos_start;
        Position pos_end;
        std::string err_name;
        std::string details;
        Error(Position pos_start, Position pos_end, std::string err_name, std::string details);
        virtual std::string as_string();
        virtual ~Error() = default;
};

class IllegalCharError : public Error {
    public:
        IllegalCharError(Position pos_start, Position pos_end, std::string details);
};

class InvalidSyntaxError : public Error {
    public:
        InvalidSyntaxError(Position pos_start, Position pos_end, std::string details);
};

class ExpectedCharError : public Error {
    public:
        ExpectedCharError(Position pos_start, Position pos_end, std::string details);
};

class RuntimeError : public Error {
    public:
        Context context;
        RuntimeError(Position pos_start, Position pos_end, std::string details, Context context);
        std::string as_string() override;
        std::string generate_traceback();
};

// ************TOKENS************

class Token {
    public:
        std::string type;
        std::string value;
        Position pos_start;
        Position pos_end;
        bool matches(std::string type_, std::string value_);
        Token();
        Token(std::string type, std::string value = "", Position* pos_start = nullptr, Position* pos_end = nullptr);
        std::string repr() const;
        friend std::ostream& operator<<(std::ostream& os, const Token& obj);
};

// ************LEXER************

class Lexer {
    public:
        std::string file_name;
        std::string text;
        Lexer(std::string file_name, std::string text);
        Position pos = Position(-1, 0, -1, file_name, text);
        char current_char = '\0';
        void advance();
        std::pair<std::vector<Token>, Error*> make_tokens();
        Token make_number();
        Token make_identifier();
        Token make_string();
        std::pair<Token, Error*> make_not_equals();
        Token make_equals();
        Token make_less_than();
        Token make_greater_than();
        Token make_minus_or_arrow();
        void skip_comment();
};

// ************NODES************

class Node {
    public:
        virtual ~Node() = default;
        virtual std::string repr() const = 0;
        Position pos_start;
        Position pos_end;
};

class StringNode : public Node {
    public:
        Token token;
        StringNode();
        StringNode(Token token);
        std::string repr() const override;
};

class NumberNode : public Node {
    public:
        Token token;
        NumberNode();
        NumberNode(Token token);
        std::string repr() const override;
        friend std::ostream& operator<<(std::ostream& os, const NumberNode& obj);
};

class VarAccessNode : public Node {
    public:
        Token var_name_token;
        VarAccessNode();
        VarAccessNode(Token var_name_token);
        std::string repr() const override;
};

class VarAssignNode : public Node {
    public:
        Token var_name_token;
        Node* value_node;
        VarAssignNode();
        VarAssignNode(Token var_name_token, Node* value_node);
        ~VarAssignNode();
        std::string repr() const override;
};

class BinOpNode : public Node {
    public:
        Node* left;
        Token op;
        Node* right;
        BinOpNode();
        BinOpNode(Node* left, Token token, Node* right);
        std::string repr() const override;
        ~BinOpNode();
        friend std::ostream& operator<<(std::ostream& os, const BinOpNode& obj);
};

class UnaryOpNode : public Node {
    public:
        Token op;
        Node* node;
        UnaryOpNode();
        UnaryOpNode(Token op, Node* node);
        std::string repr() const override;
        ~UnaryOpNode();
        friend std::ostream& operator<<(std::ostream& os, const UnaryOpNode& obj);
};

class IfNode : public Node {
    public:
        std::vector<std::tuple<Node*, Node*, bool>> cases;
        IfNode();
        IfNode(std::vector<std::tuple<Node*, Node*, bool>> cases);
        std::string repr() const override;
        ~IfNode();
};

class ForNode : public Node {
    public:
        Token var_name_token;
        Node* start_value_node;
        Node* end_value_node;
        Node* step_value_node;
        Node* body_node;
        bool should_return_null;
        ForNode();
        ForNode(Token var_name_token, Node* start_value_node, Node* end_value_node, Node* step_value_node, Node* body_node, bool should_return_null);
        ~ForNode();
        std::string repr() const override;
};

class WhileNode : public Node {
    public: 
        Node* condition_node;
        Node* body_node;
        bool should_return_null;
        WhileNode();
        WhileNode(Node* condition_node, Node* body_node, bool should_return_null);
        ~WhileNode();
        std::string repr() const override;
};

class FuncDefNode: public Node {
    public:
        Token var_name_token;
        std::vector<Token> arg_name_tokens;
        Node* body_node;
        bool should_auto_return;
        FuncDefNode();
        FuncDefNode(Token var_name_token, std::vector<Token> arg_name_tokens, Node* body_node, bool should_auto_return);
        ~FuncDefNode();
        std::string repr() const override;
};

class CallNode : public Node {
    public:
        Node* node_to_call;
        std::vector<Node*> args;
        CallNode();
        CallNode(Node* node_to_call, std::vector<Node*> args);
        ~CallNode();
        std::string repr() const override;
};

class ListNode : public Node {
    public:
        std::vector<Node*> elements;
        ListNode();
        ListNode(std::vector<Node*> elements, Position pos_start, Position pos_end);
        ~ListNode();
        std::string repr() const override;
};

class ReturnNode : public Node {
    public:
        Node* node_to_return;
        ReturnNode();
        ReturnNode(Node* node_to_return, Position pos_start, Position pos_end);
        ~ReturnNode();
        std::string repr() const override;
};

class ContinueNode : public Node {
    public:
        Position pos_start;
        Position pos_end;
        ContinueNode(Position pos_start, Position pos_end);
        std::string repr() const override;
};

class BreakNode : public Node {
    public:
        Position pos_start;
        Position pos_end;
        BreakNode(Position pos_start, Position pos_end);
        std::string repr() const override;
};

// ************PARSE RESULT************

class ParseResult {
    public:
        Node* ast;
        Error* error;
        int advance_count = 0;
        int to_reverse_count = 0;
        ParseResult();
        ParseResult(Node* ast, Error* error);
        Node* reg(ParseResult* result);
        void reg_advancement();
        template <typename T>
        ParseResult* success(T* n);
        ParseResult* failure(Error* error);
        Node* try_register(ParseResult* result);
};

// ************PARSER************

class Parser {
    public:
        std::vector<Token> tokens;
        Token current_token;
        int token_index;
        Parser(std::vector<Token> tokens);
        Token advance();
        ParseResult* atom();
        ParseResult* factor();
        ParseResult* power();
        ParseResult* term();
        ParseResult* statement();
        ParseResult* statements();
        ParseResult* expr();
        ParseResult* parse();
        ParseResult* comp_expr();
        ParseResult* arith_expr();
        ParseResult* if_expr();
        ParseResult* for_expr();
        ParseResult* while_expr();
        ParseResult* func_def();
        ParseResult* list_expr();
        ParseResult* call();
        ParseResult* bin_op(ParseResult* (Parser::*func1)(), std::vector<std::string> ops, ParseResult* (Parser::*func2)()=nullptr, std::vector<std::vector<std::string>> ops_with_values=std::vector<std::vector<std::string>>());
        std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> if_expr_cases(std::string case_keyword);
        std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> if_expr_b_or_c();
        std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> if_expr_b();
        Token reverse(int amount=1);
};

// ************VALUES************

class Value {
    public:
        Position pos_start = Position();
        Position pos_end = Position();
        Context context;
        void set_pos(Position pos_start=Position(), Position pos_end=Position());
        Value* set_context(Context context=Context());
        Value();
        virtual ~Value() = default;
        virtual std::pair<Value*, Error*> added_to(Value* other);
        virtual std::pair<Value*, Error*> subtracted_by(Value* other);
        virtual std::pair<Value*, Error*> multiplied_by(Value* other);
        virtual std::pair<Value*, Error*> divided_by(Value* other);
        virtual std::pair<Value*, Error*> raised_to(Value* other);
        virtual std::pair<Value*, Error*> get_comparison_eq(Value* other);
        virtual std::pair<Value*, Error*> get_comparison_ne(Value* other);
        virtual std::pair<Value*, Error*> get_comparison_lt(Value* other);
        virtual std::pair<Value*, Error*> get_comparison_gt(Value* other);
        virtual std::pair<Value*, Error*> get_comparison_lte(Value* other);
        virtual std::pair<Value*, Error*> get_comparison_gte(Value* other);
        virtual std::pair<Value*, Error*> anded_with(Value* other);
        virtual std::pair<Value*, Error*> ored_with(Value* other);
        virtual std::pair<Value*, Error*> notted();
        virtual std::pair<RuntimeResult*, Error*> execute(std::vector<Value*> args);
        virtual Value* copy() = 0;
        virtual bool is_true();
        RuntimeError* illegal_operation(Value* other=nullptr);  
        std::string repr();  
};

class String : public Value {
    public:
        std::string value;
        Position pos_start;
        Position pos_end;
        String();
        String(std::string value);
        bool is_true() override;
        String* copy() override;
        std::pair<Value*, Error*> added_to(Value* other) override;
        std::pair<Value*, Error*> multiplied_by(Value* other) override;
        std::pair<Value*, Error*> get_comparison_eq(Value* other) override;
        std::pair<Value*, Error*> get_comparison_ne(Value* other) override;
        std::string repr();
};

class Number : public Value {
    public:
        enum Type { INTEGER, FLOAT } type;
        union {
            int64_t int_value;
            double float_value;
        };
        Position pos_start;
        Position pos_end;
        Number();
        Number(int64_t v);
        Number(double v);
        double getAsFloat() const;
        int64_t getAsInt() const;
        bool is_null = false;
        bool is_true() override;
        Error* added_to(void* other);
        std::pair<Value*, Error*> added_to(Value* other) override;
        Error* subtracted_by(void* other);
        std::pair<Value*, Error*> subtracted_by(Value* other) override;
        Error* multiplied_by(void* other);
        std::pair<Value*, Error*> multiplied_by(Value* other) override;
        Error* divided_by(void* other);
        std::pair<Value*, Error*> divided_by(Value* other) override;
        Error* raised_to(void* other);
        std::pair<Value*, Error*> raised_to(Value* other) override;
        Error* get_comparison_eq(void* other);
        std::pair<Value*, Error*> get_comparison_eq(Value* other) override;;
        Error* get_comparison_ne(void* other);
        std::pair<Value*, Error*> get_comparison_ne(Value* other) override;;
        Error* get_comparison_lt(void* other);
        std::pair<Value*, Error*> get_comparison_lt(Value* other) override;;
        Error* get_comparison_gt(void* other);
        std::pair<Value*, Error*> get_comparison_gt(Value* other) override;;
        Error* get_comparison_lte(void* other);
        std::pair<Value*, Error*> get_comparison_lte(Value* other) override;;
        Error* get_comparison_gte(void* other);
        std::pair<Value*, Error*> get_comparison_gte(Value* other) override;;
        Error* anded_with(void* other);
        std::pair<Value*, Error*> anded_with(Value* other) override;
        Error* ored_with(void* other);
        std::pair<Value*, Error*> ored_with(Value* other) override;
        std::pair<Value*, Error*> notted() override;
        Number* copy() override;
        std::string repr();
};

class BaseFunction : public Value {
    public:
        std::string name;
        std::vector<Token> arg_name_tokens;
        Node* body_node;
        BaseFunction(Token n);
        void generate_new_context(Context& new_context);
        RuntimeResult* check_args(std::vector<Value*> args, std::vector<Token> arg_name_tokens);
        void populate_args(std::vector<Value*> args, std::vector<Token> arg_name_tokens, Context& new_context);
        RuntimeResult check_and_populate_args(std::vector<Value*> args, std::vector<Token> arg_name_tokens, Context& new_context);
};

class Function : public BaseFunction {
    public:
        std::string name;
        Token var_name_token;
        std::vector<Token> arg_name_tokens;
        Node* body_node;
        Position pos_start;
        Position pos_end;
        bool should_auto_return;
        Function();
        Function(Token var_name_token, std::vector<Token> arg_name_tokens, Node* body_node, bool should_auto_return);
        std::pair<RuntimeResult*, Error*> execute(std::vector<Value*> args) override;
        Function* copy() override;
        std::string repr();
};

class List : public Value {
    public:
        std::vector<Value*> elements;
        Position pos_start;
        Position pos_end;
        List();
        ~List();
        List(std::vector<Value*> elements);
        List* copy() override;
        std::pair<Value*, Error*> added_to(Value* other) override;
        std::pair<Value*, Error*> multiplied_by(Value* other) override;
        std::pair<Value*, Error*> subtracted_by(Value* other) override;
        std::pair<Value*, Error*> divided_by(Value* other) override;
        std::string repr();
};

class BuiltinFunction : public BaseFunction {
    public:
        std::string name;
        BuiltinFunction(Token name);
        std::pair<RuntimeResult*, Error*> execute(std::vector<Value*> args) override;
        BuiltinFunction* copy() override;
        std::string repr();

        // functions
        RuntimeResult* func_print(std::vector<Value*> args);
        RuntimeResult* func_input(std::vector<Value*> args);
        RuntimeResult* func_input_int(std::vector<Value*> args);
        RuntimeResult* func_clear(std::vector<Value*> args);
        RuntimeResult* func_is_number(std::vector<Value*> args);
        RuntimeResult* func_is_string(std::vector<Value*> args);
        RuntimeResult* func_is_list(std::vector<Value*> args);
        RuntimeResult* func_is_function(std::vector<Value*> args);
        RuntimeResult* func_append(std::vector<Value*> args);
        RuntimeResult* func_pop(std::vector<Value*> args);
        RuntimeResult* func_extend(std::vector<Value*> args);
        RuntimeResult* func_len(std::vector<Value*> args);
        RuntimeResult* func_run(std::vector<Value*> args, Context& context);  
};

// ************RUNTIME RESULT************

class RuntimeResult {
    public:
        Value* value = nullptr;
        Error* error = new Error(Position(), Position(), "", "");
        Value* func_return_value = nullptr;
        bool loop_should_continue = false;
        bool loop_should_break = false;
        void reset();
        RuntimeResult();
        RuntimeResult(Value* value, Error* error);
        Value* reg(RuntimeResult result);
        RuntimeResult* success(Value* value);
        RuntimeResult* failure(Error* error);
        RuntimeResult* success_return(Value* value);
        RuntimeResult* success_continue();
        RuntimeResult* success_break();
        bool should_return();
};

// ************INTERPRETER************

class Interpreter {
    public:
        ParseResult* parse_result;
        Interpreter(ParseResult* parse_result);
        template <typename T>
        RuntimeResult visit(T* node, Context& context);
        RuntimeResult visit_NumberNode(NumberNode* node, Context& context);
        RuntimeResult visit_BinOpNode(BinOpNode* node, Context& context);
        RuntimeResult visit_UnaryOpNode(UnaryOpNode* node, Context& context);
        RuntimeResult visit_Undefined(Node* node, Context& context);
        RuntimeResult visit_VarAccessNode(VarAccessNode* node, Context& context);
        RuntimeResult visit_VarAssignNode(VarAssignNode* node, Context& context);
        RuntimeResult visit_IfNode(IfNode* node, Context& context);
        RuntimeResult visit_ForNode(ForNode* node, Context& context);
        RuntimeResult visit_WhileNode(WhileNode* node, Context& context);
        RuntimeResult visit_FuncDefNode(FuncDefNode* node, Context& context);
        RuntimeResult visit_CallNode(CallNode* node, Context& context);
        RuntimeResult visit_StringNode(StringNode* node, Context& context);
        RuntimeResult visit_ListNode(ListNode* node, Context& context);
        RuntimeResult visit_ReturnNode(ReturnNode* node, Context& context);
        RuntimeResult visit_ContinueNode(ContinueNode* node, Context& context);
        RuntimeResult visit_BreakNode(BreakNode* node, Context& context);
};

// ************RUN************
std::pair<Value*, Error*> run(std::string file_name, std::string text);

std::string run_script_cpp(std::string script);

#endif