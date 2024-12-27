#include "vero.h"

// ************CONSTANTS************

const std::vector<char> DIGITS = {'0', '1', '2', '3', '4', '5', '6', '7', '8', '9'};

std::string INT_TOKEN = "INT";
std::string FLOAT_TOKEN = "FLOAT";
std::string STRING_TOKEN = "STRING";
std::string PLUS_TOKEN = "PLUS";
std::string MINUS_TOKEN = "MINUS";
std::string POWER_TOKEN = "POW";
std::string MUL_TOKEN = "MUL";
std::string DIV_TOKEN = "DIV";
std::string LPAREN_TOKEN = "LPAREN";
std::string RPAREN_TOKEN = "RPAREN";
std::string LSQUARE_TOKEN = "LSQUARE";
std::string RSQUARE_TOKEN = "RSQUARE";
std::string LBRACE_TOKEN = "LBRACE";
std::string RBRACE_TOKEN = "RBRACE";
std::string EOF_TOKEN = "EOF";
std::string IDENTIFIER_TOKEN = "IDENTIFIER";
std::string EQUALS_TOKEN = "EQUALS";
std::string KEYWORD_TOKEN = "KEYWORD";
std::string EE_TOKEN = "EE";
std::string NE_TOKEN = "NE";
std::string LT_TOKEN = "LT";
std::string GT_TOKEN = "GT";
std::string LTE_TOKEN = "LTE";
std::string GTE_TOKEN = "GTE";
std::string COMMA_TOKEN = "COMMA";
std::string ARROW_TOKEN = "ARROW";
std::string NEWLINE_TOKEN = "NEWLINE";

Number* null_ = new Number(static_cast<int64_t>(0));
Number* false_ = new Number(static_cast<int64_t>(0));
Number* true_ = new Number(static_cast<int64_t>(0));
Number* pi_ = new Number(3.14159265358979323846);

bool is_digit(char c) {
    for (int i = 0; i < 10; i++) {
        if (c == DIGITS[i]) {
            return true;
        }
    }
    return false;
}

bool is_letter(char c) {
    return (c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_';
}

bool is_letter_or_digit(char c) {
    return is_letter(c) || is_digit(c);
}

std::vector<std::string> KEYWORDS = {
    "let", "and", "or", 
    "not", "if", "then", 
    "elif", "else", "for", 
    "to", "step", "while", 
    "func", "null", "true", 
    "false", "print", "input", 
    "input_int", "clear", "is_number", 
    "is_string", "is_list", "is_function", 
    "append", "pop", "extend", 
    "pi", "end", "return",
    "continue", "break", "len", "run"
}; 
bool is_keyword(std::string id) {
    for (int i = 0; i < KEYWORDS.size(); i++) {
        if (id == KEYWORDS[i]) {
            return true;
        }
    }
    return false;
}

// ************TOKENS************

Token::Token() : type(""), value(""), pos_start(Position()), pos_end(Position()) {}

Token::Token(std::string type, std::string value, Position* pos_start, Position* pos_end) {
    this->type = type;
    this->value = value;
    if (pos_start != nullptr) {
        this->pos_start = pos_start->copy();
        this->pos_end = pos_start->copy();
        this->pos_end.advance();
    }
    if (pos_end != nullptr) {
        this->pos_end = pos_end->copy();
    }
}

bool Token::matches(std::string type_, std::string value_) {
    return type == type_ && value == value_;
}

std::string Token::repr() const {
    if (value != "") {
        return type + ":" + value;
    } else {
        return type;
    }
}

std::ostream& operator<<(std::ostream& os, const Token& obj) {
    os << obj.repr();
    return os;
}

// ************LEXER************

Lexer::Lexer(std::string file_name, std::string text) : file_name(file_name), text(text), pos(Position(-1, 0, -1, file_name, text)) {
    this->file_name = file_name;
    this->text = text;
    advance();
}

void Lexer::advance() {
    pos.advance(current_char);
    if (pos.index < text.length()) {
        current_char = text[pos.index];
    } else {
        current_char = '\0';
    }
}

std::pair<std::vector<Token>, Error*> Lexer::make_tokens() { // TOKENIZATION FUNCTION
    std::vector<Token> tokens;

    while (current_char != '\0') {
        if (current_char == ' ' || current_char == '\t') {
            advance();
        } else if (current_char == '#') {
            skip_comment();
            if (current_char == '\n') {
                tokens.push_back(Token(NEWLINE_TOKEN, "", &pos));
                advance();
            }
        } else if (current_char == ';' || current_char == '\n') {
            tokens.push_back(Token(NEWLINE_TOKEN, "", &pos));
            advance();
        } else if (is_digit(current_char)) {
            tokens.push_back(make_number());
        } else if (is_letter(current_char)) {
            tokens.push_back(make_identifier());
        } else if (current_char == '+') {
            tokens.push_back(Token(PLUS_TOKEN, "", &pos));
            advance();
        } else if (current_char == '"') {
            tokens.push_back(make_string());
        } else if (current_char == '-') {
            tokens.push_back(make_minus_or_arrow());
        } else if (current_char == '*') {
            tokens.push_back(Token(MUL_TOKEN, "", &pos));
            advance();
        } else if (current_char == '/') {
            tokens.push_back(Token(DIV_TOKEN, "", &pos));
            advance();
        } else if (current_char == '^') {
            tokens.push_back(Token(POWER_TOKEN, "", &pos));
            advance();
        } else if (current_char == '(') {
            tokens.push_back(Token(LPAREN_TOKEN, "", &pos));
            advance();
        } else if (current_char == ')') {
            tokens.push_back(Token(RPAREN_TOKEN, "", &pos));
            advance();
        } else if (current_char == '[') {
            tokens.push_back(Token(LSQUARE_TOKEN, "", &pos));
            advance();
        } else if (current_char == ']') {
            tokens.push_back(Token(RSQUARE_TOKEN, "", &pos));
            advance();
        } else if (current_char == '{') {
            tokens.push_back(Token(LBRACE_TOKEN, "", &pos));
            advance();
        } else if (current_char == '}') {
            tokens.push_back(Token(RBRACE_TOKEN, "", &pos));
            advance();
        } else if (current_char == '!') {
            std::pair<Token, Error*> res_of_mne = make_not_equals();
            if (res_of_mne.second->err_name != "") {
                return std::pair<std::vector<Token>, Error*>(std::vector<Token>(), res_of_mne.second);
            }
            tokens.push_back(res_of_mne.first);
        } else if (current_char == '=') {
            tokens.push_back(make_equals());
        } else if (current_char == '>') {
            tokens.push_back(make_greater_than());
        } else if (current_char == '<') {
            tokens.push_back(make_less_than());
        } else if (current_char == ',') {
            tokens.push_back(Token(COMMA_TOKEN, "", &pos));
            advance();
        } else {
            Position start = pos.copy();
            char c = current_char;
            advance();
            return std::pair<std::vector<Token>, Error*>(std::vector<Token>(), new IllegalCharError(start, pos, "'" + std::string(1, c) + "'"));
        }
    }

    tokens.push_back(Token(EOF_TOKEN, "", &pos));
    return std::pair<std::vector<Token>, Error*>(tokens, new Error(Position(-1, -1, -1, file_name, text), Position(-1, -1, -1, file_name, text), EOF_TOKEN, ""));
}

Token Lexer::make_minus_or_arrow() {
    Position pos_start = pos.copy();
    advance();
    if (current_char == '>') {
        advance();
        return Token(ARROW_TOKEN, "", &pos_start, &pos);
    } else {
        return Token(MINUS_TOKEN, "", &pos_start, &pos);
    }
}

Token Lexer::make_number() { // NUMBER TOKENIZATION FUNCTION
    std::string num = "";
    int dot_count = 0;
    Position pos_start = pos.copy();

    while (current_char != '\0' && (is_digit(current_char) || current_char == '.')) {
        if (current_char == '.') {
            if (dot_count == 1) {
                break;
            }
            dot_count++;
            num = num + ".";
        } else {
            num = num + current_char;
        }
        advance();
    }

    if (dot_count == 0) {
        return Token(INT_TOKEN, num, &pos_start, &pos);
    } else {
        return Token(FLOAT_TOKEN, num, &pos_start, &pos);
    }
}

Token Lexer::make_identifier() { // IDENTIFIER TOKENIZATION FUNCTION
    std::string id = "";
    Position pos_start = pos.copy();

    while (current_char != '\0' && is_letter_or_digit(current_char)) {
        id = id + current_char;
        advance();
    }
    std::string type = ""; 
    if (is_keyword(id)) {
        type = KEYWORD_TOKEN;
    } else {
        type = IDENTIFIER_TOKEN;
    }

    return Token(type, id, &pos_start, &pos);
}

Token Lexer::make_string() { // STRING TOKENIZATION FUNCTION
    std::string str = "";
    Position pos_start = pos.copy();
    bool escape_character = false;
    advance();

    std::map<char, char> escape_characters = {
        {'n', '\n'},
        {'t', '\t'}
    };

    while (current_char != '\0' && (current_char != '"' or escape_character)) {
        if (escape_character) {
            if (escape_characters.find(current_char) != escape_characters.end()) {
                str += escape_characters[current_char];
            } else {
                str += current_char;
            }
        } else {
            if (current_char=='\\') {
                escape_character = true;
            } else {
                str += current_char;
            }
        }
        advance(); 
        escape_character = false;
    }

    advance();

    return Token(STRING_TOKEN, str, &pos_start, &pos);
}

std::pair<Token, Error*> Lexer::make_not_equals() {
    Position pos_start = pos.copy();
    advance();
    if (current_char == '=') {
        advance();
        return std::pair<Token, Error*>(Token(NE_TOKEN, "", &pos_start, &pos), new Error(Position(), Position(), "", ""));
    } else {
        Position start = pos.copy();
        advance();
        return std::pair<Token, Error*>(Token(), new ExpectedCharError(pos_start, pos, "'=' (after '!')"));
    }
}

Token Lexer::make_equals() {
    Position pos_start = pos.copy();
    advance();
    if (current_char == '=') {
        advance();
        return Token(EE_TOKEN, "", &pos_start, &pos);
    } else {
        return Token(EQUALS_TOKEN, "", &pos_start, &pos);
    }
}

Token Lexer::make_greater_than() {
    Position pos_start = pos.copy();
    advance();
    if (current_char == '=') {
        advance();
        return Token(GTE_TOKEN, "", &pos_start, &pos);
    } else {
        return Token(GT_TOKEN, "", &pos_start, &pos);
    }
}

Token Lexer::make_less_than() {
    Position pos_start = pos.copy();
    advance();
    if (current_char == '=') {
        advance();
        return Token(LTE_TOKEN, "", &pos_start, &pos);
    } else {
        return Token(LT_TOKEN, "", &pos_start, &pos);
    }
}

void Lexer::skip_comment() {
    advance();
    while (current_char != '\n' && current_char != '\0') {
        advance();
    }
}

// ************POSITION************

Position::Position() : index(0), line(0), col(0), file_name(""), file_text("") {}

Position::Position(int index, int line, int col, std::string file_name, std::string file_text) {
    this->index = index;
    this->line = line;
    this->col = col;
    this->file_name = file_name;
    this->file_text = file_text;
}

void Position::advance(char current_char) {
    index++;
    col++;

    if (current_char == '\n') {
        line++;
        col = 0;
    }
}

Position Position::copy() {
    return Position(index, line, col, file_name, file_text);
}

// ************ERRORS************

Error::Error(Position pos_start, Position pos_end, std::string err_name, std::string details) {
    this->pos_start = pos_start;
    this->pos_end = pos_end;
    this->err_name = err_name;
    this->details = details;
}

std::string Error::as_string() {
    std::string result = err_name + ": " + details;
    // add the file name and line number
    result = result + "\nFile:" + "'" + pos_start.file_name + "'" + ", Line:" + std::to_string(pos_start.line + 1);

    // Extract just the error line from file_text
    std::string error_line;
    std::istringstream stream(pos_start.file_text);
    std::string line;
    int current_line = 0;
    while (std::getline(stream, line)) {
        if (current_line == pos_start.line) {
            error_line = line;
            break;
        }
        current_line++;
    }

    result = result + "\n\n" + error_line + "\n" + std::string(pos_start.col, ' ') + "^";
    return result;
}

IllegalCharError::IllegalCharError(Position pos_start, Position pos_end, std::string details) 
    : Error(pos_start, pos_end, "Illegal Character", details) {}

InvalidSyntaxError::InvalidSyntaxError(Position pos_start, Position pos_end, std::string details) 
    : Error(pos_start, pos_end, "Invalid Syntax", details) {}

ExpectedCharError::ExpectedCharError(Position pos_start, Position pos_end, std::string details) 
    : Error(pos_start, pos_end, "Expected Character", details) {}

RuntimeError::RuntimeError(Position pos_start, Position pos_end, std::string details, Context context) 
    : Error(pos_start, pos_end, "Runtime Error", details) {
    this->context = context;
    this->context.parent_entry_pos = pos_start;
    this->pos_start = pos_start;
    this->pos_end = pos_end;
    this->context.parent_entry_pos.file_name = pos_start.file_name;
    this->context.parent_entry_pos.file_text = pos_start.file_text;     
}

std::string RuntimeError::as_string() {
    std::string result = generate_traceback();
    result = result + err_name + ": " + details;

    // Extract just the error line from file_text
    std::string error_line;
    std::istringstream stream(pos_start.file_text);
    std::string line;
    int current_line = 0;
    while (std::getline(stream, line)) {
        if (current_line == pos_start.line) {
            error_line = line;
            break;
        }
        current_line++;
    }

    // add arrows to point to the error
    result = result + "\n\n" + error_line + "\n" + std::string(pos_start.col, ' ') + "^";
    return result;
}

std::string RuntimeError::generate_traceback() {
    std::string result = "";
    Position pos = pos_start;
    Context* ctx = &context;
    while (ctx != nullptr) {
        result = result + "  File " + "'" + pos.file_name + "'" + ", line " + std::to_string(pos.line + 1) + ", in " + ctx->display_name + "\n";
        pos = ctx->parent_entry_pos;
        ctx = ctx->parent;
    }
    return "Traceback (most recent call last):\n" + result;
}

// ************RUN************

SymbolTable global_symbol_table = SymbolTable();

void initialize_global_symbol_table() {
    global_symbol_table.set("null", null_);
    global_symbol_table.set("true", true_);
    global_symbol_table.set("false", false_);
    global_symbol_table.set("print", new BuiltinFunction(Token(KEYWORD_TOKEN, "print")));
    global_symbol_table.set("input", new BuiltinFunction(Token(KEYWORD_TOKEN, "input")));
    global_symbol_table.set("input_int", new BuiltinFunction(Token(KEYWORD_TOKEN, "input_int")));
    global_symbol_table.set("clear", new BuiltinFunction(Token(KEYWORD_TOKEN, "clear")));
    global_symbol_table.set("is_number", new BuiltinFunction(Token(KEYWORD_TOKEN, "is_number")));
    global_symbol_table.set("is_string", new BuiltinFunction(Token(KEYWORD_TOKEN, "is_string")));
    global_symbol_table.set("is_list", new BuiltinFunction(Token(KEYWORD_TOKEN, "is_list")));
    global_symbol_table.set("is_function", new BuiltinFunction(Token(KEYWORD_TOKEN, "is_function")));
    global_symbol_table.set("append", new BuiltinFunction(Token(KEYWORD_TOKEN, "append")));
    global_symbol_table.set("pop", new BuiltinFunction(Token(KEYWORD_TOKEN, "pop")));
    global_symbol_table.set("extend", new BuiltinFunction(Token(KEYWORD_TOKEN, "extend")));
    global_symbol_table.set("len", new BuiltinFunction(Token(KEYWORD_TOKEN, "len")));
    global_symbol_table.set("run", new BuiltinFunction(Token(KEYWORD_TOKEN, "run")));
    global_symbol_table.set("pi", pi_);
}

std::pair<Value*, Error*> run(std::string file_name, std::string text) {
    // Tokenization : Generate Tokens
    Lexer lexer = Lexer(file_name, text);
    std::pair<std::vector<Token>, Error*> result = lexer.make_tokens();

    if (result.second->err_name != "EOF") {
        return std::pair<Value*, Error*>(nullptr, result.second);
    }

    // Parsing : Generate Abstract Syntax Tree
    Parser parser = Parser(result.first);
    ParseResult* ast = parser.parse();

    if (ast->error->err_name != "") {
        return std::pair<Value*, Error*>(nullptr, ast->error);
    }

    // Interpretation : Evaluate the AST
    Interpreter interpreter = Interpreter(ast);
    Context context = Context("<program>");
    context.parent_entry_pos = Position(0, 0, 0, file_name, text);
    context.symbol_table = global_symbol_table;
    RuntimeResult output = interpreter.visit(ast->ast, context);
    global_symbol_table = context.symbol_table;

    return std::pair<Value*, Error*>(output.value, output.error);
}

// ************NODES************

VarAccessNode::VarAccessNode() : var_name_token(Token()) {}

VarAccessNode::VarAccessNode(Token var_name_token) {
    this->var_name_token = var_name_token;
    this->pos_start = var_name_token.pos_start;
    this->pos_end = var_name_token.pos_end;
}

std::string VarAccessNode::repr() const {
    return var_name_token.repr();
}

VarAssignNode::VarAssignNode() : var_name_token(Token()), value_node(nullptr) {}

VarAssignNode::VarAssignNode(Token var_name_token, Node* value_node) {
    this->var_name_token = var_name_token;
    this->value_node = value_node;
    this->pos_start = var_name_token.pos_start;
    this->pos_end = value_node->pos_end;
}

VarAssignNode::~VarAssignNode() {
    delete value_node;
}

std::string VarAssignNode::repr() const {
    return "(" + var_name_token.repr() + ", " + value_node->repr() + ")";
} 

StringNode::StringNode() : token(Token()) {}

StringNode::StringNode(Token token) {
    this->token = token;
    this->pos_start = token.pos_start;
    this->pos_end = token.pos_end;
}

std::string StringNode::repr() const {
    return token.repr();
}

NumberNode::NumberNode() : token(Token()) {}

NumberNode::NumberNode(Token token) {
    this->token = token;
    this->pos_start = token.pos_start;
    this->pos_end = token.pos_end;
}

std::string NumberNode::repr() const {
    return token.repr();
}

std::ostream& operator<<(std::ostream& os, const NumberNode& obj) {
    os << obj.repr();
    return os;
}

BinOpNode::BinOpNode() : left(nullptr), op(Token()), right(nullptr) {}

BinOpNode::BinOpNode(Node* left, Token op, Node* right) {
    this->left = left;
    this->op = op;
    this->right = right;
    this->pos_start = left->pos_start;
    this->pos_end = right->pos_end;
}

BinOpNode::~BinOpNode() {
    delete left;
    delete right;
}

std::string BinOpNode::repr() const {
    return "(" + left->repr() + ", " + op.repr() + ", " + right->repr() + ")";
}

std::ostream& operator<<(std::ostream& os, const BinOpNode& obj) {
    os << obj.repr();
    return os;
}

UnaryOpNode::UnaryOpNode() : op(Token()), node(nullptr) {}

UnaryOpNode::UnaryOpNode(Token op, Node* node) {
    this->op = op;
    this->node = node;
    this->pos_start = op.pos_start;
    this->pos_end = node->pos_end;
}

UnaryOpNode::~UnaryOpNode() {
    delete node;
}

std::string UnaryOpNode::repr() const {
    return "(" + op.repr() + ", " + node->repr() + ")";
}

std::ostream& operator<<(std::ostream& os, const UnaryOpNode& obj) {
    os << obj.repr();
    return os;
}

IfNode::IfNode() : cases(std::vector<std::tuple<Node*, Node*, bool>>{std::make_tuple(nullptr, nullptr, false)}) {}

IfNode::IfNode(std::vector<std::tuple<Node*, Node*, bool>> cases) {
    this->cases = cases;
    this->pos_start = std::get<0>(cases[0])->pos_start;
    this->pos_end = std::get<0>(cases[cases.size() - 1])->pos_end;
}

IfNode::~IfNode() {
    for (int i = 0; i < cases.size(); i++) {
        delete std::get<0>(cases[i]);
        delete std::get<1>(cases[i]);
    }
}

std::string IfNode::repr() const {
    std::string result = "";
    for (int i = 0; i < cases.size(); i++) {
        result = result + "if " + std::get<0>(cases[i])->repr() + " then " + std::get<1>(cases[i])->repr() + "\n";
    }
    return result;
}

ForNode::ForNode() : var_name_token(Token()), start_value_node(nullptr), end_value_node(nullptr), step_value_node(nullptr), body_node(nullptr), should_return_null(false) {}

ForNode::ForNode(Token var_name_token, Node* start_value_node, Node* end_value_node, Node* step_value_node, Node* body_node, bool should_return_null) {
    this->var_name_token = var_name_token;
    this->start_value_node = start_value_node;
    this->end_value_node = end_value_node;
    this->step_value_node = step_value_node;
    this->body_node = body_node;
    this->pos_start = var_name_token.pos_start;
    this->pos_end = body_node->pos_end;
    this->should_return_null = should_return_null;
}

ForNode::~ForNode() {
    delete start_value_node;
    delete end_value_node;
    delete step_value_node;
    delete body_node;
}

std::string ForNode::repr() const {
    return "for " + var_name_token.repr() + " = " + start_value_node->repr() + " to " + end_value_node->repr() + " step " + step_value_node->repr() + " " + body_node->repr();
}

WhileNode::WhileNode() : condition_node(nullptr), body_node(nullptr), should_return_null(false) {}

WhileNode::WhileNode(Node* condition_node, Node* body_node, bool should_return_null) {
    this->condition_node = condition_node;
    this->body_node = body_node;
    this->pos_start = condition_node->pos_start;
    this->pos_end = body_node->pos_end;
    this->should_return_null = should_return_null;
}

WhileNode::~WhileNode() {
    delete condition_node;
    delete body_node;
}

std::string WhileNode::repr() const {
    return "while " + condition_node->repr() + " " + body_node->repr();
}

FuncDefNode::FuncDefNode() : var_name_token(Token()), arg_name_tokens(std::vector<Token>{Token()}), body_node(nullptr), should_auto_return(false) {}

FuncDefNode::FuncDefNode(Token var_name_token, std::vector<Token> arg_name_tokens, Node* body_node, bool should_auto_return) {
    this->var_name_token = var_name_token;
    this->arg_name_tokens = arg_name_tokens;
    this->body_node = body_node;
    this->should_auto_return = should_auto_return; 

    if (this->var_name_token.value != "") {
        this->pos_start = var_name_token.pos_start;
    } else if (this->arg_name_tokens.size() > 0) {
        this->pos_start = arg_name_tokens[0].pos_start;
    } else {
        this->pos_start = this->body_node->pos_start;
    }

    this->pos_end = this->body_node->pos_end;
}

FuncDefNode::~FuncDefNode() {
    delete body_node;
}

std::string FuncDefNode::repr() const {
    std::string result = "func " + var_name_token.repr() + "(";
    for (int i = 0; i < arg_name_tokens.size(); i++) {
        result = result + arg_name_tokens[i].repr();
        if (i != arg_name_tokens.size() - 1) {
            result = result + ", ";
        }
    }
    result = result + ") " + body_node->repr();
    return result;
}

CallNode::CallNode() : node_to_call(nullptr), args(std::vector<Node*>{new NumberNode(Token())}) {}

CallNode::CallNode(Node* node_to_call, std::vector<Node*> args) {
    this->node_to_call = node_to_call;
    this->args = args;
    this->pos_start = node_to_call->pos_start;
    if (args.size() > 0) {
        this->pos_end = args[args.size() - 1]->pos_end;
    } else {
        this->pos_end = node_to_call->pos_end;
    }
}

CallNode::~CallNode() {
    delete node_to_call;
    for (int i = 0; i < args.size(); i++) {
        delete args[i];
    }
}

std::string CallNode::repr() const {
    std::string result = node_to_call->repr() + "(";
    for (int i = 0; i < args.size(); i++) {
        result = result + args[i]->repr();
        if (i != args.size() - 1) {
            result = result + ", ";
        }
    }
    result = result + ")";
    return result;
}

ListNode::ListNode(std::vector<Node*> elements, Position pos_start, Position pos_end) {
    this->elements = elements;
    this->pos_start = pos_start;
    this->pos_end = pos_end;
}

ListNode::~ListNode() {
    for (int i = 0; i < elements.size(); i++) {
        delete elements[i];
    }
}

std::string ListNode::repr() const {
    std::string result = "[";
    for (int i = 0; i < elements.size(); i++) {
        result += elements[i]->repr();
        if (i < elements.size() - 1) {
            result += ", ";
        }
    }
    result += "]";
    return result;
}

ReturnNode::ReturnNode() : node_to_return(nullptr) {}

ReturnNode::ReturnNode(Node* node_to_return, Position pos_start, Position pos_end) {
    this->node_to_return = node_to_return;
    this->pos_start = pos_start;
    this->pos_end = pos_end;
}

ReturnNode::~ReturnNode() {
    delete node_to_return;
}

std::string ReturnNode::repr() const {
    return "return " + node_to_return->repr();
}

ContinueNode::ContinueNode(Position pos_start, Position pos_end) {
    this->pos_start = pos_start;
    this->pos_end = pos_end;
}

std::string ContinueNode::repr() const {
    return "continue";
}

BreakNode::BreakNode(Position pos_start, Position pos_end) {
    this->pos_start = pos_start;
    this->pos_end = pos_end;
}

std::string BreakNode::repr() const {
    return "break";
}

// ************PARSE RESULT************

ParseResult::ParseResult() : ast(nullptr), error(new Error(Position(-1, -1, -1, "", ""), Position(-1, -1, -1, "", ""), "", "")), advance_count(0), to_reverse_count(0) {}

ParseResult::ParseResult(Node* ast, Error* error) : ast(ast), error(error), advance_count(0), to_reverse_count(0) {}

Node* ParseResult::reg(ParseResult* result) {
    advance_count += result->advance_count;
    if (result->error->err_name != "") {
        this->error = result->error;
    }
    return result->ast;
}

void ParseResult::reg_advancement() {
    advance_count++;
}

template <typename T>
ParseResult* ParseResult::success(T* n) {
    if constexpr (std::is_same_v<T, std::vector<std::tuple<Node*, Node*, bool>>>) {
        if (n->size()==0) {
            return this;
        }
        this->ast = new IfNode(*n);
        return this;
    } else {
        Node* node = dynamic_cast<Node*>(n);
        this->ast = node;   
    }
    return this;
}

ParseResult* ParseResult::failure(Error* error) {
    if (this->error->err_name == "" || advance_count == 0) {
        this->error = error;
    }
    return this;
}

Node* ParseResult::try_register(ParseResult* res) {
    if (res->error->err_name != "") {
        this->to_reverse_count = res->advance_count;
        return nullptr;
    }
    return reg(res);
}

// ************PARSER************

Parser::Parser(std::vector<Token> tokens) : tokens(tokens), token_index(-1) {
    advance();
}

Token Parser::advance() {
    token_index++;
    if (token_index>=0 && token_index < tokens.size()) {
        current_token = tokens[token_index];
    }
    return current_token;
}

Token Parser::reverse(int amount) {
    token_index-=amount;
    if (token_index>=0 && token_index < tokens.size()) {
        current_token = tokens[token_index];
    }
    return current_token;
}

ParseResult* Parser::call() {
    ParseResult* res = new ParseResult();
    Node* atom_res = res->reg(atom());
    if (res->error->err_name != "") {
        return res;
    }

    if (current_token.type == LPAREN_TOKEN) {
        res->reg_advancement();
        advance();
        std::vector<Node*> args;
        if (current_token.type == RPAREN_TOKEN) {
            res->reg_advancement();
            advance();
        } else {
            args.push_back(res->reg(expr()));
            if (res->error->err_name != "") {
                return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected ')', 'var', 'if', 'for', 'while', 'func', INT, FLOAT, IDENTIFIER, '+', '-' '[' or '('"));
            }
            while (current_token.type == COMMA_TOKEN) {
                res->reg_advancement();
                advance();
                args.push_back(res->reg(expr()));
                if (res->error->err_name != "") {
                    return res;
                }
            }
            if (current_token.type != RPAREN_TOKEN) {
                return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected ',' or ')'"));
            }
            res->reg_advancement();
            advance();
        }
        return res->success(new CallNode(atom_res, args));
    }
    return res->success(atom_res);
}

ParseResult* Parser::atom() {
    ParseResult* res = new ParseResult();
    Token token = current_token;
    if (token.type == INT_TOKEN || token.type == FLOAT_TOKEN) {
        res->reg_advancement();
        advance();
        return res->success(new NumberNode((token)));
    } else if (token.type == STRING_TOKEN) {
        res->reg_advancement();
        advance();
        return res->success(new StringNode(token));
    } else if (token.type == IDENTIFIER_TOKEN) {
        res->reg_advancement();
        advance();
        return res->success(new VarAccessNode(token));
    } else if (token.type == LPAREN_TOKEN) {
        res->reg_advancement();
        advance();
        ParseResult* expr_res = expr();
        if (expr_res->error->err_name != "") {
            delete res;
            return expr_res;
        }
        if (current_token.type == RPAREN_TOKEN) {
            res->reg_advancement();
            advance();
            Node* expr_node = expr_res->ast;
            expr_res->ast = nullptr; // to prevent double deletion
            delete expr_res;
            return res->success(expr_node);
        } else {
            delete expr_res;
            return res->failure(new InvalidSyntaxError(token.pos_start, token.pos_end, "Expected ')'"));
        }
    } else if (token.matches(KEYWORD_TOKEN, "if")) {
        Node* if_expression = res->reg(if_expr());
        if (res->error->err_name != "") {
            return res;
        }
        return res->success(if_expression);
    } else if (token.matches(KEYWORD_TOKEN, "for")) {
        Node* for_expression = res->reg(for_expr());
        if (res->error->err_name != "") {
            return res;
        }
        return res->success(for_expression);
    } else if (token.matches(KEYWORD_TOKEN, "while")) {
        Node* while_expression = res->reg(while_expr());
        if (res->error->err_name != "") {
            return res;
        }
        return res->success(while_expression);
    } else if (token.matches(KEYWORD_TOKEN, "func")) {
        Node* func_definition = res->reg(func_def());
        if (res->error->err_name != "") {
            return res;
        }
        return res->success(func_definition);
    } else if (token.type == LSQUARE_TOKEN) {
        Node* list_expr_res = res->reg(list_expr());
        if (res->error->err_name != "") {
            return res;
        }
        return res->success(list_expr_res);
    } else if (token.matches(KEYWORD_TOKEN, "print") || token.matches(KEYWORD_TOKEN, "input") || token.matches(KEYWORD_TOKEN, "input_int") || token.matches(KEYWORD_TOKEN, "clear") || token.matches(KEYWORD_TOKEN, "is_number") || token.matches(KEYWORD_TOKEN, "is_string") || token.matches(KEYWORD_TOKEN, "is_list") || token.matches(KEYWORD_TOKEN, "is_function") || token.matches(KEYWORD_TOKEN, "append") || token.matches(KEYWORD_TOKEN, "pop") || token.matches(KEYWORD_TOKEN, "extend") || token.matches(KEYWORD_TOKEN, "len") || token.matches(KEYWORD_TOKEN, "run")) {
        res->reg_advancement();
        advance();
        if (current_token.type == LPAREN_TOKEN) {
            res->reg_advancement();
            advance();
            std::vector<Node*> args;
            if (current_token.type != RPAREN_TOKEN) {
                args.push_back(res->reg(expr()));
                if (res->error->err_name != "") {
                    return res;
                }
                while (current_token.type == COMMA_TOKEN) {
                    res->reg_advancement();
                    advance();
                    args.push_back(res->reg(expr()));
                    if (res->error->err_name != "") {
                        return res;
                    }
                }
                if (current_token.type != RPAREN_TOKEN) {
                    return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected ',' or ')'"));
                }
            }
            res->reg_advancement();
            advance();
            return res->success(new CallNode(new VarAccessNode(token), args));
        } else {
            return res->failure(new InvalidSyntaxError(token.pos_start, token.pos_end, "Expected '('"));
        }
    }
    return res->failure(new InvalidSyntaxError(token.pos_start, token.pos_end, "Expected INT, FLOAT, IDENTIFIER, 'if', 'while', 'func', '+', '-', '(' or '['"));
}

std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> Parser::if_expr_cases(std::string case_keyword) {
    ParseResult* res = new ParseResult();
    std::vector<std::tuple<Node*, Node*, bool>> cases = std::vector<std::tuple<Node*, Node*, bool>>{};

    if (!current_token.matches(KEYWORD_TOKEN, case_keyword)) {
        return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '" + case_keyword + "'")));
    }

    res->reg_advancement();
    advance();

    Node* condition = res->reg(this->expr());
    if (res->error->err_name != "") {
        return std::pair(cases, res);
    }

    if (current_token.type != LBRACE_TOKEN) {
        return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '{'")));
    }

    res->reg_advancement();
    advance();

    if (current_token.type==NEWLINE_TOKEN) {
        res->reg_advancement();
        advance();

        Node* statements = res->reg(this->statements());
        if (res->error->err_name != "") {
            return std::pair(cases, res);
        }
        cases.push_back(std::make_tuple(condition, statements, true));

        if (current_token.type == RBRACE_TOKEN) {
            res->reg_advancement();
            advance();
        } else {
            return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'")));
        }
        
        if (current_token.matches(KEYWORD_TOKEN, "elif")) {
            std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> if_expr_boc;
            if_expr_boc = this->if_expr_b_or_c();
            res->reg(if_expr_boc.second);
            if (res->error->err_name != "") {
                return std::pair(cases, res);
            }
            std::vector<std::tuple<Node*, Node*, bool>> all_cases = if_expr_boc.first;
            cases.insert(cases.end(), all_cases.begin(), all_cases.end());
        } else if (current_token.matches(KEYWORD_TOKEN, "else")) {
            // Treat else case as elif with condition as true
            res->reg_advancement();
            advance();
            
            if (current_token.type != LBRACE_TOKEN) {
                return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '{'")));
            } else {
                res->reg_advancement();
                advance();
            }

            Node* statements = res->reg(this->statements());
            if (res->error->err_name != "") {
                return std::pair(cases, res);
            }
            std::tuple<Node*, Node*, bool> new_case = std::make_tuple(new NumberNode(Token(INT_TOKEN, "1")), statements, true);
            cases.push_back(new_case);

            if (current_token.type == RBRACE_TOKEN) {
                res->reg_advancement();
                advance();
            } else {
                return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'")));
            }
        }
    } else {
        Node* expr = res->reg(this->statement());
        if (res->error->err_name != "") {
            return std::pair(cases, res);
        }
        cases.push_back(std::make_tuple(condition, expr, false));

        if (current_token.type != RBRACE_TOKEN) {
            return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'"))); 
        }

        res->reg_advancement();
        advance();

        std::vector<std::tuple<Node*, Node*, bool>> all_cases;
        std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> if_expr_boc;
        
        if_expr_boc = this->if_expr_b_or_c();

        // check if there are more cases
        if (if_expr_boc.first.size() > 0) {
            res->reg(if_expr_boc.second);
            if (res->error->err_name != "") {
                return std::pair(cases, res);
            }
            all_cases = if_expr_boc.first;
            cases.insert(cases.end(), all_cases.begin(), all_cases.end());
        }
    }

    return std::pair(cases, res->success(&cases));
}

ParseResult* Parser::if_expr() {
    ParseResult* res = new ParseResult();
    std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> if_exp;
    if_exp = if_expr_cases("if");
    res->reg(if_exp.second);
    if (res->error->err_name != "") {
        return res;
    }
    std::vector<std::tuple<Node*, Node*, bool>> cases = if_exp.first;

    return res->success(new IfNode(cases));
}

std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> Parser::if_expr_b() {
    return if_expr_cases("elif");
}

std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> Parser::if_expr_b_or_c() {
    ParseResult* res = new ParseResult();
    std::vector<std::tuple<Node*, Node*, bool>> cases = std::vector<std::tuple<Node*, Node*, bool>>{};

    if (current_token.matches(KEYWORD_TOKEN, "elif")) {
        std::pair<std::vector<std::tuple<Node*, Node*, bool>>, ParseResult*> if_exprb;
        if_exprb = this->if_expr_b();
        res->reg(if_exprb.second);
        if (res->error->err_name != "") {
            return std::pair(cases, res);
        }
        std::vector<std::tuple<Node*, Node*, bool>> all_cases = if_exprb.first;
        cases = all_cases;
    } else if (current_token.matches(KEYWORD_TOKEN, "else")) {
        // Treat else case as elif with condition as true
        res->reg_advancement();
        advance();
        
        if (current_token.type != LBRACE_TOKEN) {
            return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '{'")));
        } else {
            res->reg_advancement();
            advance();
        }

        Node* statements = res->reg(this->statements());
        if (res->error->err_name != "") {
            return std::pair(cases, res);
        }
        std::tuple<Node*, Node*, bool> new_case = std::make_tuple(new NumberNode(Token(INT_TOKEN, "1")), statements, true);
        cases.push_back(new_case);

        if (current_token.type == RBRACE_TOKEN) {
            res->reg_advancement();
            advance();
        } else {
            return std::pair(cases, res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'")));
        }
    } 

    return std::pair(cases, res->success(&cases));
}

ParseResult* Parser::for_expr() {
    ParseResult* res = new ParseResult();

    if (!current_token.matches(KEYWORD_TOKEN, "for")) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected 'for'"));
    }

    res->reg_advancement();
    advance();

    if (current_token.type != IDENTIFIER_TOKEN) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected Identifier"));
    }

    Token var_name = current_token;
    res->reg_advancement();
    advance();

    if (current_token.type != EQUALS_TOKEN) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '='"));
    }

    res->reg_advancement();
    advance();

    Node* start_value = res->reg(expr());
    if (res->error->err_name != "") {
        return res;
    }

    if (!current_token.matches(KEYWORD_TOKEN, "to")) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected 'to'"));
    }

    res->reg_advancement();
    advance();

    Node* end_value = res->reg(expr());
    if (res->error->err_name != "") {
        return res;
    }

    Node* step_value = nullptr;
    if(current_token.matches(KEYWORD_TOKEN, "step")) {
        res->reg_advancement();
        advance();
        step_value = res->reg(expr());
        if (res->error->err_name != "") {
            return res;
        }
    } else {
        step_value = new NumberNode(Token(INT_TOKEN, "1", &current_token.pos_start, &current_token.pos_end));
    }

    if (current_token.type != LBRACE_TOKEN) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '{'"));
    }

    res->reg_advancement();
    advance();

    if (current_token.type == NEWLINE_TOKEN) {
        res->reg_advancement();
        advance();

        Node* body = res->reg(statements());
        if (res->error->err_name != "") {
            return res;
        }

        if (current_token.type != RBRACE_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'"));
        }

        res->reg_advancement();
        advance();

        return res->success(new ForNode(var_name, start_value, end_value, step_value, body, true));
    }

    Node* body = res->reg(statement());
    if (res->error->err_name != "") {
        return res;
    }

    if (current_token.type != RBRACE_TOKEN) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'"));
    }

    res->reg_advancement();
    advance();

    return res->success(new ForNode(var_name, start_value, end_value, step_value, body, false));
}

ParseResult* Parser::while_expr() {
    ParseResult* res = new ParseResult();

    if (!current_token.matches(KEYWORD_TOKEN, "while")) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected 'while'"));
    }

    res->reg_advancement();
    advance();

    ParseResult* condition_res = expr();
    Node* condition = res->reg(condition_res);

    if (res->error->err_name != "") {
        return res;
    }

    if (current_token.type != LBRACE_TOKEN) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '{'"));
    }

    res->reg_advancement();
    advance();

    if (current_token.type == NEWLINE_TOKEN) {
        res->reg_advancement();
        advance();

        Node* body = res->reg(statements());
        if (res->error->err_name != "") {
            return res;
        }

        if (current_token.type != RBRACE_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'"));
        }

        res->reg_advancement();
        advance();

        return res->success(new WhileNode(condition, body, true));
    }

    Node* body = res->reg(statement());
    if (res->error->err_name != "") {
        return res;
    }

    if (current_token.type != RBRACE_TOKEN) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'"));
    }

    res->reg_advancement();
    advance();

    return res->success(new WhileNode(condition, body, false));
}

ParseResult* Parser::func_def() {
    ParseResult* res = new ParseResult();

    if (!current_token.matches(KEYWORD_TOKEN, "func")) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected 'func'"));
    }

    res->reg_advancement();
    advance();

    Token var_name = current_token;
    if (current_token.type == IDENTIFIER_TOKEN) {
        var_name  = current_token;
        res->reg_advancement();
        advance();
        if (current_token.type != LPAREN_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '('"));
        }
    } else {
        var_name = Token();
        if (current_token.type != LPAREN_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected IDENTIFIER or '('"));
        }
    }

    res->reg_advancement();
    advance();

    std::vector<Token> arg_name_tokens;
    if (current_token.type == IDENTIFIER_TOKEN) {
        arg_name_tokens.push_back(current_token);
        res->reg_advancement();
        advance();

        while (current_token.type == COMMA_TOKEN) {
            res->reg_advancement();
            advance();

            if (current_token.type != IDENTIFIER_TOKEN) {
                return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected Identifier"));
            }

            arg_name_tokens.push_back(current_token);
            res->reg_advancement();
            advance();
        }

        if (current_token.type != RPAREN_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected ')' or ','"));
        }
    } else {
        if (current_token.type != RPAREN_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected IDENTIFIER or ')'"));
        }
    }

    res->reg_advancement();
    advance();

    if (current_token.type == LBRACE_TOKEN) {
        res->reg_advancement();
        advance();

        if (current_token.type == NEWLINE_TOKEN) {
            res->reg_advancement();
            advance();

            Node* body = res->reg(statements());
            if (res->error->err_name != "") {
                return res;
            }

            if (current_token.type != RBRACE_TOKEN) {
                return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'"));
            }

            res->reg_advancement();
            advance();

            return res->success(new FuncDefNode(var_name, arg_name_tokens, body, true));
        } else {
            Node* body = res->reg(expr());
            if (res->error->err_name != "") {
                return res;
            }

            if (current_token.type != RBRACE_TOKEN) {
                return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '}'"));
            }

            res->reg_advancement();
            advance();

            return res->success(new FuncDefNode(var_name, arg_name_tokens, body, true));
        }
    }

    return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '{'"));
}

ParseResult* Parser::factor() {
    ParseResult* res = new ParseResult();
    Token token = current_token;
    if (token.type == PLUS_TOKEN || token.type == MINUS_TOKEN) {
        res->reg_advancement();
        advance();
        ParseResult* factor_res = factor();
        if (factor_res->error->err_name != "") {
            delete res;
            return factor_res;
        }
        Node* factor_node = factor_res->ast;
        factor_res->ast = nullptr; // to prevent double deletion
        delete factor_res;
        return res->success(new UnaryOpNode(token, factor_node));
    } 
    return power();
}

ParseResult* Parser::power() {
    return bin_op(&Parser::call, {POWER_TOKEN}, &Parser::factor);
}

ParseResult* Parser::term() {
    return bin_op(&Parser::factor, {MUL_TOKEN, DIV_TOKEN});
}

ParseResult* Parser::statement() {
    ParseResult* res = new ParseResult();
    Position pos_start = current_token.pos_start.copy();

    if (current_token.matches(KEYWORD_TOKEN, "return")) {
        res->reg_advancement();
        advance();

        Node* expr = res->try_register(this->expr());
        if (expr == nullptr) {
            reverse(res->to_reverse_count);
        }
        return res->success(new ReturnNode(expr, pos_start, current_token.pos_end.copy()));
    }

    if (current_token.matches(KEYWORD_TOKEN, "continue")) {
        res->reg_advancement();
        advance();
        return res->success(new ContinueNode(pos_start, current_token.pos_end.copy()));
    }

    if (current_token.matches(KEYWORD_TOKEN, "break")) {
        res->reg_advancement();
        advance();
        return res->success(new BreakNode(pos_start, current_token.pos_end.copy()));
    }

    Node* expr = res->reg(this->expr());
    if (res->error->err_name != "") {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected 'let', 'if', 'for', 'while', 'func', 'return', 'continue', 'break', INT, FLOAT, IDENTIFIER, '+', '-', '[', '(', 'not' or 'print'"));
    }

    return res->success(expr);
}

ParseResult* Parser::statements() {
    ParseResult* res = new ParseResult();
    std::vector<Node*> statements;
    Position pos_start = current_token.pos_start.copy();

    while (current_token.type == NEWLINE_TOKEN) {
        res->reg_advancement();
        advance();
    }

    Node* statement = res->reg(this->statement());
    if (res->error->err_name != "") {
        return res;
    }
    statements.push_back(statement);

    bool more_statements = true;
    while (true) {
        int newlines = 0;
        while (current_token.type == NEWLINE_TOKEN) {
            res->reg_advancement();
            advance();
            newlines++;
        }
        if (newlines == 0) {
            more_statements = false;
        }
        if (!more_statements) {
            break;
        }
        statement = res->try_register(this->statement());
        if (statement == nullptr) {
            reverse(res->to_reverse_count);
            more_statements = false;
            continue;
        }
        statements.push_back(statement);
    }

    return res->success(new ListNode(statements, pos_start, current_token.pos_end.copy()));
}

ParseResult* Parser::expr() {
    ParseResult* res = new ParseResult();
    if (current_token.matches(KEYWORD_TOKEN, "let")) {
        res->reg_advancement();
        advance();
        if (current_token.type != IDENTIFIER_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected Identifier"));
        }
        Token var_name = current_token;
        res->reg_advancement();
        advance();
        if (current_token.type != EQUALS_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '='"));
        }
        res->reg_advancement();
        advance();
        ParseResult* expr_res = expr();
        Node* expr = res->reg(expr_res);
        if (res->error->err_name != "") {
            return res;
        }
        return res->success(new VarAssignNode(var_name, expr));
    } else {
        Node* node = res->reg(bin_op(&Parser::comp_expr, {}, nullptr, {{KEYWORD_TOKEN, "and"}, {KEYWORD_TOKEN, "or"}}));
        if (res->error->err_name != "") {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected 'let', INT, FLOAT, IDENTIFIER, 'if', 'while', 'func', '+', '-', '[' or '('"));
        }
        return res->success(node);
    }
}

ParseResult* Parser::parse() {
    ParseResult* result = statements();
    if (result->error->err_name == "" && current_token.type != EOF_TOKEN) {
        return result->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '+', '-', '*', or '/'"));
    }
    return result;
}

ParseResult* Parser::bin_op(ParseResult* (Parser::*func1)(), std::vector<std::string> ops, ParseResult* (Parser::*func2)(), std::vector<std::vector<std::string>> ops_with_values) {
    if (func2==nullptr) {
        func2 = func1;
    }

    ParseResult* res = new ParseResult();
    Node* left = res->reg((this->*func1)());
    if (res->error->err_name != "") {
        return res;
    }

    if (ops_with_values.empty()) { // if ops_with_values is empty, then value of operator does not matter
        while (std::find(ops.begin(), ops.end(), current_token.type) != ops.end()) {
            Token op = current_token;
            res->reg_advancement();
            advance();
            Node* right = res->reg((this->*func2)());
            if (res->error->err_name != "") {
                return res;
            }
            left = new BinOpNode(left, op, right);
        }

        return res->success(left);
    } else { // if ops_with_values is not empty, then value of operator matters and is checked
        while (std::find_if(ops_with_values.begin(), ops_with_values.end(), [&](const std::vector<std::string>& op) { return op[0] == current_token.type && op[1] == current_token.value; }) != ops_with_values.end()) {
            Token op = current_token;
            res->reg_advancement();
            advance();
            Node* right = res->reg((this->*func2)());
            if (res->error->err_name != "") {
                return res;
            }
            left = new BinOpNode(left, op, right);
        }

        return res->success(left);
    }
}

ParseResult* Parser::comp_expr() {
    ParseResult* res = new ParseResult();

    if (current_token.matches(KEYWORD_TOKEN, "not")) {
        Token op = current_token;
        res->reg_advancement();
        advance();
        Node* node = res->reg(comp_expr());
        if (res->error->err_name != "") {
            return res;
        }
        return res->success(new UnaryOpNode(op, node));
    } else {
        Node* node = res->reg(bin_op(&Parser::arith_expr, {EE_TOKEN, NE_TOKEN, LT_TOKEN, GT_TOKEN, LTE_TOKEN, GTE_TOKEN}));
        if (res->error->err_name != "") {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected 'not', INT, FLOAT, IDENTIFIER, '+', '-', '[' or '('"));
        }
        return res->success(node);
    }
}

ParseResult* Parser::arith_expr() {
    return bin_op(&Parser::term, {PLUS_TOKEN, MINUS_TOKEN});
}

ParseResult* Parser::list_expr() {
    ParseResult* res = new ParseResult();
    std::vector<Node*> elements;
    Position pos_start = current_token.pos_start.copy();
    if (current_token.type != LSQUARE_TOKEN) {
        return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected '['"));
    }

    res->reg_advancement();
    advance();

    if (current_token.type == RSQUARE_TOKEN) {
        res->reg_advancement();
        advance();
    } else {
        elements.push_back(res->reg(expr()));
        if (res->error->err_name != "") {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected ']', 'var', 'if', 'for', 'while', 'func', INT, FLOAT, IDENTIFIER, '+', '-', '[' or '('"));
        }
        while (current_token.type == COMMA_TOKEN) {
            res->reg_advancement();
            advance();
            elements.push_back(res->reg(expr()));
            if (res->error->err_name != "") {
                return res;
            }
        }
        if (current_token.type != RSQUARE_TOKEN) {
            return res->failure(new InvalidSyntaxError(current_token.pos_start, current_token.pos_end, "Expected ',' or ']'"));
        }
        res->reg_advancement();
        advance();
    }
    return res->success(new ListNode(elements, pos_start, current_token.pos_end.copy()));
}

// ************CONTEXT************

Context::Context(std::string display_name, Context* parent, Position parent_entry_pos) 
    : display_name(display_name), parent(parent), parent_entry_pos(parent_entry_pos), symbol_table(SymbolTable()) {}

Context::Context() : display_name(""), parent(nullptr), parent_entry_pos(Position()), symbol_table(SymbolTable()) {}

// ************SYMBOL TABLE************

SymbolTable::SymbolTable(SymbolTable* parent) : parent(parent), symbols(std::map<std::string, Value*>()) {}

Value* SymbolTable::get(std::string name) {
    if (symbols.find(name) != symbols.end()) {
        return symbols[name];
    }
    if (parent != nullptr) {
        return parent->get(name);
    }
    return nullptr;
}

void SymbolTable::set(std::string name, Value* value) {
    symbols[name] = value;
}

void SymbolTable::remove(std::string name) {
    symbols.erase(name);
}

// ************INTERPRETER************

Interpreter::Interpreter(ParseResult* parse_result) : parse_result(parse_result) {}

template <typename T>
RuntimeResult Interpreter::visit(T* node, Context& context) {
    if (auto num = dynamic_cast<NumberNode*>(node)) {
        return visit_NumberNode(num, context);
    } else if (auto bin = dynamic_cast<BinOpNode*>(node)) {
        return visit_BinOpNode(bin, context);
    } else if (auto un = dynamic_cast<UnaryOpNode*>(node)) {
        return visit_UnaryOpNode(un, context);
    } else if (auto var = dynamic_cast<VarAccessNode*>(node)) {
        return visit_VarAccessNode(var, context);
    } else if (auto var = dynamic_cast<VarAssignNode*>(node)) {
        return visit_VarAssignNode(var, context);
    } else if (auto if_node = dynamic_cast<IfNode*>(node)) {
        return visit_IfNode(if_node, context);
    } else if (auto for_node = dynamic_cast<ForNode*>(node)) {
        return visit_ForNode(for_node, context);
    } else if (auto while_node = dynamic_cast<WhileNode*>(node)) {
        return visit_WhileNode(while_node, context);
    } else if (auto func_def = dynamic_cast<FuncDefNode*>(node)) {
        return visit_FuncDefNode(func_def, context);
    } else if (auto call = dynamic_cast<CallNode*>(node)) {
        return visit_CallNode(call, context);
    } else if (auto str = dynamic_cast<StringNode*>(node)) {
        return visit_StringNode(str, context);
    } else if (auto list = dynamic_cast<ListNode*>(node)) {
        return visit_ListNode(list, context);
    } else if (auto continue_ = dynamic_cast<ContinueNode*>(node)) {
        return visit_ContinueNode(continue_, context);
    } else if (auto break_ = dynamic_cast<BreakNode*>(node)) {
        return visit_BreakNode(break_, context);
    } else if (auto return_node = dynamic_cast<ReturnNode*>(node)) {
        return visit_ReturnNode(return_node, context);
    } else {
        return visit_Undefined(node, context);
    }
}

RuntimeResult Interpreter::visit_VarAccessNode(VarAccessNode* node, Context& context) {
    RuntimeResult rt_result = RuntimeResult();
    std::string var_name = node->var_name_token.value;
    Value* value = context.symbol_table.get(var_name);
    if (value==nullptr) {
        return *rt_result.failure(new RuntimeError(node->pos_start, node->pos_end, "Variable '" + var_name + "' is not defined", context));
    }
    value = value->copy();
    value->set_pos(node->pos_start, node->pos_end);
    value->set_context(context);
    return *rt_result.success(value);
}

RuntimeResult Interpreter::visit_VarAssignNode(VarAssignNode* node, Context& context) {
    RuntimeResult rt_result = RuntimeResult();
    std::string var_name = node->var_name_token.value;
    Value* value = rt_result.reg(visit(node->value_node, context));
    if (rt_result.should_return()) {
        return rt_result;
    }
    value->set_context(context);
    context.symbol_table.set(var_name, value);
    return *rt_result.success(value);
}

RuntimeResult Interpreter::visit_Undefined(Node* node, Context& context) {
    throw std::runtime_error("Undefined Node Type");
    return RuntimeResult();
}

RuntimeResult Interpreter::visit_StringNode(StringNode* node, Context& context) {
    String* str = new String(node->token.value);
    str->set_context(context);
    str->set_pos(node->pos_start, node->pos_end);
    RuntimeResult rt_result = RuntimeResult();
    rt_result.success(str);
    return rt_result;
}

RuntimeResult Interpreter::visit_NumberNode(NumberNode* node, Context& context) {
    // check which type of number 
    Value* no;
    if (node->token.type == INT_TOKEN) {
        no = new Number(std::stoll(node->token.value));
    } else {
        no = new Number(std::stod(node->token.value));
    }
    no->set_context(context);
    no->set_pos(node->pos_start, node->pos_end);
    RuntimeResult rt_result = RuntimeResult();
    rt_result.success(no);
    return rt_result;
}

RuntimeResult Interpreter::visit_BinOpNode(BinOpNode* node, Context& context) {
    RuntimeResult rt_result;
    Value* left = rt_result.reg(visit(node->left, context));
    if (rt_result.should_return()) {
        return rt_result;
    }
    Value* right = rt_result.reg(visit(node->right, context));
    if (rt_result.should_return()) {
        return rt_result;
    }
    std::pair<Value*, Error*> result(new Number(), nullptr);
    if (node->op.type == PLUS_TOKEN) {
        result = left->added_to(right);
    } else if (node->op.type == MINUS_TOKEN) {
        result = left->subtracted_by(right);
    } else if (node->op.type == MUL_TOKEN) {
        result = left->multiplied_by(right);
    } else if (node->op.type == DIV_TOKEN) {
        result = left->divided_by(right);
    } else if (node->op.type == POWER_TOKEN) {
        result = left->raised_to(right);
    } else if (node->op.type == EE_TOKEN) {
        result = left->get_comparison_eq(right);
    } else if (node->op.type == NE_TOKEN) {
        result = left->get_comparison_ne(right);
    } else if (node->op.type == LT_TOKEN) {
        result = left->get_comparison_lt(right);
    } else if (node->op.type == GT_TOKEN) {
        result = left->get_comparison_gt(right);
    } else if (node->op.type == LTE_TOKEN) {
        result = left->get_comparison_lte(right);
    } else if (node->op.type == GTE_TOKEN) {
        result = left->get_comparison_gte(right);
    } else if (node->op.matches(KEYWORD_TOKEN, "and")) {
        result = left->anded_with(right);
    } else if (node->op.matches(KEYWORD_TOKEN, "or")) {
        result = left->ored_with(right);
    }
    if (result.second->err_name != "") {
        return *rt_result.failure(result.second);
    }
    result.first->set_pos(node->pos_start, node->pos_end);
    return *rt_result.success(result.first);
}

RuntimeResult Interpreter::visit_UnaryOpNode(UnaryOpNode* node, Context& context) {
    RuntimeResult rt_result;
    Number* no = static_cast<Number*>(rt_result.reg(visit(node->node, context)));
    if (rt_result.should_return()) {
        return rt_result;
    }
    std::pair<Value*, Error*> result;
    if (node->op.type == PLUS_TOKEN) {
        result.first = no;
    } else if (node->op.type == MINUS_TOKEN) {
        Number minus_one = Number(static_cast<int64_t>(-1));
        result = no->multiplied_by(&minus_one);
    } else if (node->op.matches(KEYWORD_TOKEN, "not")) {
        result = no->notted();
    }
    result.first->set_pos(node->pos_start, node->pos_end);
    rt_result.success(result.first);
    return rt_result;
}

RuntimeResult Interpreter::visit_IfNode(IfNode* node, Context& context) {
    RuntimeResult rt_result;
    for (int i = 0; i < node->cases.size(); i++) {
        std::tuple<Node*, Node*, bool> case_ = node->cases[i];
        Value* condition = rt_result.reg(visit(std::get<0>(case_), context));
        Node* expr = std::get<1>(case_);
        bool should_return_null = i == std::get<2>(case_);
        if (rt_result.should_return()) {
            return rt_result;
        }
        if (condition->is_true()) {
            Value* expr = rt_result.reg(visit(std::get<1>(case_), context));
            if (rt_result.should_return()) {
                return rt_result;
            }
            if (should_return_null) {
                return *rt_result.success(new Number(static_cast<int64_t>(0)));
            } else {
                return *rt_result.success(expr);
            }
        }
    }
    return *rt_result.success(new Number(static_cast<int64_t>(0)));
}

RuntimeResult Interpreter::visit_ForNode(ForNode* node, Context& context) {
    RuntimeResult rt_result;
    std::vector<Value*> elements;
    
    // start value
    Value* start_val = rt_result.reg(visit(node->start_value_node, context));
    if (rt_result.should_return()) {
        return rt_result;
    }
    const Number* start = static_cast<Number*>(start_val);

    // end value  
    Value* end_val = rt_result.reg(visit(node->end_value_node, context));
    if (rt_result.should_return()) {
        return rt_result;
    }
    const Number* end_value = static_cast<Number*>(end_val);

    // step value (default = 1)
    const Number* step = nullptr;
    if (node->step_value_node) {
        Value* step_val = rt_result.reg(visit(node->step_value_node, context));
        if (rt_result.should_return()) {
            return rt_result;
        }
        step = static_cast<Number*>(step_val);
    } else {
        step = new Number(static_cast<int64_t>(1));
    }
    SymbolTable& symbol_table = context.symbol_table;
    const std::string& var_name = node->var_name_token.value;
    Node* body_node = node->body_node;
    Value* result = new Number();
    if (start->type==Number::INTEGER && end_value->type==Number::INTEGER && step->type==Number::INTEGER) {
        int64_t start_value = start->getAsInt();
        int64_t end = end_value->getAsInt();
        int64_t step_value = step->getAsInt();
        for (int64_t i = start_value; i < end; i += step_value) {
            symbol_table.set(var_name, new Number(i));
            result = rt_result.reg(visit(body_node, context));
            if (rt_result.should_return() && rt_result.loop_should_continue==false && rt_result.loop_should_break==false) {
                return rt_result;
            }
            if (rt_result.loop_should_continue) {
                continue;
            }
            if (rt_result.loop_should_break) {
                break;
            }
            elements.push_back(result->copy());
        }
    } else {
        double start_value = start->getAsFloat();
        double end = end_value->getAsFloat();
        double step_value = step->getAsFloat();
        for (double i = start_value; i < end; i += step_value) {
            symbol_table.set(var_name, new Number(i));
            result = rt_result.reg(visit(body_node, context));
            if (rt_result.should_return() && rt_result.loop_should_continue==false && rt_result.loop_should_break==false) {
                return rt_result;
            }
            if (rt_result.loop_should_continue) {
                continue;
            }
            if (rt_result.loop_should_break) {
                break;
            }
            elements.push_back(result->copy());
        }
    }
    List* list = new List(elements);
    list->set_context(context);
    list->set_pos(node->pos_start, node->pos_end);
    if (node->should_return_null) {
        return *rt_result.success(new Number(static_cast<int64_t>(0)));
    } else {
        return *rt_result.success(list);
    }
}

RuntimeResult Interpreter::visit_WhileNode(WhileNode* node, Context& context) {
    RuntimeResult rt_result;
    std::vector<Value*> elements;
    Node* const condition_node = node->condition_node;
    Node* const body_node = node->body_node;
    Value* result;
    while (true) {
        Value* condition = rt_result.reg(visit(condition_node, context));
        if (rt_result.should_return()) {
            return rt_result;
        }
        if (!condition->is_true()) {
            break;
        }
        result = rt_result.reg(visit(body_node, context));
        if (rt_result.should_return() && rt_result.loop_should_continue==false && rt_result.loop_should_break==false) {
            return rt_result;
        }
        if (rt_result.loop_should_continue) {
            continue;
        }
        if (rt_result.loop_should_break) {
            break;
        }
        elements.push_back(result);
    }
    List* list = new List(elements);
    list->set_context(context);
    list->set_pos(node->pos_start, node->pos_end);
    if (node->should_return_null) {
        return *rt_result.success(new Number(static_cast<int64_t>(0)));
    } else {
        return *rt_result.success(list);
    }
}

RuntimeResult Interpreter::visit_FuncDefNode(FuncDefNode* node, Context& context) {
    RuntimeResult res = RuntimeResult();
    Token func_name;
    const std::string& file_name = node->pos_start.file_name;
    const std::string& file_text = node->pos_start.file_text;
    func_name = node->var_name_token;
    Function* func = new Function(func_name, node->arg_name_tokens, node->body_node, node->should_auto_return);
    func->set_context(context);
    func->set_pos(node->pos_start, node->pos_end);

    if (context.symbol_table.get(func_name.value) == nullptr) {
        context.symbol_table.set(func_name.value, func);
    }

    func->pos_start.file_name = file_name;
    func->pos_start.file_text = file_text;
    return *res.success(func);
}

RuntimeResult Interpreter::visit_ReturnNode(ReturnNode* node, Context& context) {
    RuntimeResult rt_result;
    Value* value = new Number(static_cast<int64_t>(0));
    if (node->node_to_return) {
        value = rt_result.reg(visit(node->node_to_return, context));
        if (rt_result.should_return()) {
            return rt_result;
        }
    }
    return *rt_result.success_return(value);
}

RuntimeResult Interpreter::visit_ContinueNode(ContinueNode* node, Context& context) {
    RuntimeResult rt_result;
    return *rt_result.success_continue();
}

RuntimeResult Interpreter::visit_BreakNode(BreakNode* node, Context& context) {
    RuntimeResult rt_result;
    return *rt_result.success_break();
}

RuntimeResult Interpreter::visit_CallNode(CallNode* node, Context& context) {
    RuntimeResult rt_result;
    std::vector<Value*> args;
    
    const std::string& file_name = node->pos_start.file_name;
    const std::string& file_text = node->pos_start.file_text;
    const int line_num = node->pos_start.line;
    const int col_num = node->pos_start.col;

    std::string func_to_call_name = (dynamic_cast<VarAccessNode*>(node->node_to_call))->var_name_token.value;
    if (func_to_call_name.empty()) {
        func_to_call_name = "<anonymous>";
    }

    Value* value = rt_result.reg(visit(node->node_to_call, context));
    if (rt_result.should_return()) {
        rt_result.error->pos_start.file_name = file_name;
        rt_result.error->pos_start.file_text = file_text;
        rt_result.error->pos_start.line = line_num;
        rt_result.error->pos_start.col = col_num;
        return rt_result;
    }
    
    BaseFunction* func = dynamic_cast<BaseFunction*>(value);
    if (!func) {
        return *rt_result.failure(new RuntimeError(node->pos_start, node->pos_end, "Cannot call a non-function value", context));
    }

    func = static_cast<BaseFunction*>(func->copy());
    func->set_pos(node->pos_start, node->pos_end);

    if (!func) {
        return *rt_result.failure(new RuntimeError(node->pos_start, node->pos_end, "Expected function", context));
    }

    for (Node* arg_node : node->args) {
        args.push_back(rt_result.reg(visit(arg_node, context)));
        if (rt_result.should_return()) {
            rt_result.error->pos_start.file_name = file_name;
            rt_result.error->pos_start.file_text = file_text;
            rt_result.error->pos_start.line = line_num;
            rt_result.error->pos_start.col = col_num;
            return rt_result;
        }
    }

    if (func->name=="<anonymous>") {
        func->name = "<" + func_to_call_name + ">";
    }

    std::pair result = func->execute(args);
    if (result.second && result.second->err_name != "") {
        result.second->pos_start.file_name = file_name;
        result.second->pos_start.file_text = file_text;
        result.second->pos_start.line = line_num;
        result.second->pos_start.col = col_num;
        return *rt_result.failure(result.second);
    }

    Value* return_value = result.first->value;
    if (result.first->error->err_name != "") {
        result.first->error->pos_start.file_name = file_name;
        result.first->error->pos_start.file_text = file_text; 
        result.first->error->pos_start.line = line_num;
        result.first->error->pos_start.col = col_num;
        return *rt_result.failure(result.first->error);
    }

    return_value->set_context(context);
    return_value->set_pos(node->pos_start, node->pos_end);
    return *rt_result.success(return_value->copy());
}

RuntimeResult Interpreter::visit_ListNode(ListNode* node, Context& context) {
    RuntimeResult rt_result;
    std::vector<Value*> elements;

    for (Node* element_node : node->elements) {
        RuntimeResult element_result = visit(element_node, context);
        Value* element = rt_result.reg(element_result);
        if (rt_result.should_return()) {
            return rt_result;
        }
        elements.push_back(element);
    }
    List* list = new List(elements);
    list->set_context(context);
    list->set_pos(node->pos_start, node->pos_end);
    rt_result.success(list->copy());
    return rt_result;
}

// ************RUNTIME RESULT************

RuntimeResult::RuntimeResult() : value(nullptr), error(new Error(Position(), Position(), "", "")) {}

RuntimeResult::RuntimeResult(Value* value, Error* error) : value(value), error(error) {}

void RuntimeResult::reset() {
    this->value = nullptr;
    this->error = new Error(Position(), Position(), "", "");
    this->func_return_value = nullptr;
    this->loop_should_continue = false;
    this->loop_should_break = false;
}

Value* RuntimeResult::reg(RuntimeResult result) {
    if (result.should_return()) {
        this->error = result.error;
    }
    this->func_return_value = result.func_return_value;
    this->loop_should_continue = result.loop_should_continue;
    this->loop_should_break = result.loop_should_break;
    return result.value;
}

bool RuntimeResult::should_return() {
    if (this->error->err_name != "" || this->func_return_value!=nullptr || this->loop_should_continue || this->loop_should_break) {
        return true;
    }
    return false;
}

RuntimeResult* RuntimeResult::success(Value* value) {
    reset();
    this->value = value;
    this->error = new Error(Position(), Position(), "", "");
    return this;
}

RuntimeResult* RuntimeResult::failure(Error* error) {
    reset();
    this->error = error;
    if (error->pos_start.file_name.empty()) {
        error->pos_start.file_name = error->pos_start.file_name;
    }
    return this;
}

RuntimeResult* RuntimeResult::success_return(Value* value) {
    reset();
    this->func_return_value = value;
    return this;
}

RuntimeResult* RuntimeResult::success_continue() {
    reset();
    this->loop_should_continue = true;
    return this;
}

RuntimeResult* RuntimeResult::success_break() {
    reset();
    this->loop_should_break = true;
    return this;
}

// ************VALUE CLASS************

Value::Value() {
    set_pos();
    set_context();
}

void Value::set_pos(Position pos_start, Position pos_end) {
    this->pos_start = pos_start;
    this->pos_end = pos_end;
}

Value* Value::set_context(Context context) {
    this->context = context;
    return this;
}

std::string Value::repr() {
    if (auto val = dynamic_cast<Number*>(this)) {
        return val->repr();
    } else if (auto val = dynamic_cast<List*>(this)) {
        return val->repr();
    } else if (auto val = dynamic_cast<Function*>(this)) {
        return val->repr();
    } else if (auto val = dynamic_cast<String*>(this)) {
        return val->repr();
    }

    return "";
}

std::pair<Value*, Error*> Value::added_to(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::subtracted_by(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::multiplied_by(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::divided_by(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::raised_to(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::get_comparison_eq(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::get_comparison_ne(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::get_comparison_lt(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::get_comparison_gt(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::get_comparison_lte(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::get_comparison_gte(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::anded_with(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::ored_with(Value* other) {
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> Value::notted() {
    return std::pair(new Number(), illegal_operation());
}

RuntimeError* Value::illegal_operation(Value* other) {
    if (other==nullptr) {
        return new RuntimeError(pos_start, pos_end, "Illegal operation", context);
    }
    return new RuntimeError(pos_start, other->pos_end, "Illegal operation", context);
}

std::pair<RuntimeResult*, Error*> Value::execute(std::vector<Value*> args) {
    return std::pair(new RuntimeResult(), illegal_operation());
}

Value* Value::copy() {
    throw std::runtime_error("No copy method defined");
}

bool Value::is_true() {
    return false;
}

// ************STRINGS************

String::String(std::string value) : value(value) {
    set_pos();
    set_context();
}

String::String() : value("") {}

String* String::copy() {
    String* copy = new String(value);
    copy->set_pos(pos_start, pos_end);
    copy->set_context(context);
    return copy;
}

bool String::is_true() {
    return value != "";
}

std::pair<Value*, Error*> String::added_to(Value* other) {
    if (auto str = dynamic_cast<String*>(other)) {
        return std::pair(new String(value + str->value), new Error(Position(), Position(), "", ""));
    }
    return std::pair(new String(), illegal_operation(other));
}

std::pair<Value*, Error*> String::multiplied_by(Value* other) {
    if (auto num = dynamic_cast<Number*>(other)) {
        std::string new_value = "";
        for (int i = 0; i < num->getAsInt(); i++) {
            new_value += value;
        }
        return std::pair(new String(new_value), new Error(Position(), Position(), "", ""));
    }
    return std::pair(new String(), illegal_operation(other));
}

std::pair<Value*, Error*> String::get_comparison_eq(Value* other) {
    if (auto str = dynamic_cast<String*>(other)) {
        return std::pair(new Number(static_cast<int64_t>(value == str->value)), new Error(Position(), Position(), "", ""));
    }
    return std::pair(new Number(), illegal_operation(other));
}

std::pair<Value*, Error*> String::get_comparison_ne(Value* other) {
    if (auto str = dynamic_cast<String*>(other)) {
        return std::pair(new Number(static_cast<int64_t>(value != str->value)), new Error(Position(), Position(), "", ""));
    }
    return std::pair(new Number(), illegal_operation(other));
}

// ************NUMBERS************

Number::Number() : type(INTEGER), int_value(0), is_null(true), pos_start(Position()), pos_end(Position()) {}

Number::Number(int64_t value) {
    this->type = INTEGER;
    this->int_value = value;
    this->is_null = false;
}

Number::Number(double value) {
    this->type = FLOAT;
    this->float_value = value;
    this->is_null = false;
}

double Number::getAsFloat() const {
    if (type == INTEGER) {
        return static_cast<double>(int_value);
    } else {
        return float_value;
    }
}

int64_t Number::getAsInt() const {
    if (type == INTEGER) {
        return int_value;
    } else {
        return static_cast<int64_t>(float_value);
    }
}

// fallback functions for when the argument is not a Number

Error* Number::added_to(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::subtracted_by(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::multiplied_by(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::divided_by(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::raised_to(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::get_comparison_eq(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::get_comparison_ne(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::get_comparison_lt(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::get_comparison_gt(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::get_comparison_lte(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::get_comparison_gte(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::anded_with(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}
Error* Number::ored_with(void* other) {
    return illegal_operation(static_cast<Number*>(other));
}

std::pair<Value*, Error*> Number::added_to(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num;
    if (type==INTEGER && other_num->type==INTEGER) {
        num = new Number(int_value + other_num->int_value);
    } else {
        num = new Number(getAsFloat() + other_num->getAsFloat());
    }
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::subtracted_by(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num;
    if (type==INTEGER && other_num->type==INTEGER) {
        num = new Number(int_value - other_num->int_value);
    } else {
        num = new Number(getAsFloat() - other_num->getAsFloat());
    }
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::multiplied_by(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num;
    if (type==INTEGER && other_num->type==INTEGER) {
        num = new Number(int_value * other_num->int_value);
    } else {
        num = new Number(getAsFloat() * other_num->getAsFloat());
    }
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::divided_by(Value* other) {
    Number* other_num = static_cast<Number*>(other);
    Number* num;
    if (type==INTEGER && other_num->type==INTEGER) {
        if (other_num->int_value == 0) {
            Position error_pos = pos_start;
            error_pos.file_name = context.parent_entry_pos.file_name;
            error_pos.file_text = context.parent_entry_pos.file_text;
            RuntimeError* error = new RuntimeError(error_pos, other->pos_end, "Division by zero", context);
            return std::pair(new Number(), error);
        }
        num = new Number(int_value / other_num->int_value);
    } else {
        if (other_num->getAsFloat() == 0) {
            Position error_pos = pos_start;
            error_pos.file_name = context.parent_entry_pos.file_name;
            error_pos.file_text = context.parent_entry_pos.file_text;
            RuntimeError* error = new RuntimeError(error_pos, other->pos_end, "Division by zero", context);
            return std::pair(new Number(), error);
        }
        num = new Number(getAsFloat() / other_num->getAsFloat());
    }
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::raised_to(Value* other) {
    Number* other_num = static_cast<Number*>(other);
    Number* num;
    if (type==INTEGER && other_num->type==INTEGER) {
        num = new Number(std::pow(int_value, other_num->int_value));
    } else {
        num = new Number(std::pow(getAsFloat(), other_num->getAsFloat()));
    }
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::get_comparison_eq(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() == other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::get_comparison_ne(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() != other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::get_comparison_lt(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() < other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::get_comparison_gt(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() > other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::get_comparison_lte(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() <= other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::get_comparison_gte(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() >= other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::anded_with(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() && other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::ored_with(Value* other) {
    Number* other_num = static_cast<Number*>(other);  
    Number* num = new Number(static_cast<int64_t>(getAsFloat() || other_num->getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> Number::notted() {
    Number* num = new Number(static_cast<int64_t>(!getAsFloat()));
    num->set_context(context);
    return std::pair(num, new Error(Position(), Position(), "", ""));
}

Number* Number::copy() {
    Number* num;
    if (type == INTEGER) {
        num = new Number(int_value);
    } else {
        num = new Number(float_value);
    }
    num->set_context(context);
    num->set_pos(pos_start, pos_end);
    return num;
}

bool Number::is_true() {
    if (type == INTEGER) {
        return int_value != 0;
    } else {
        return float_value != 0;
    }
}

// ************FUNCTIONS************

BaseFunction::BaseFunction(Token n) {
    set_pos();
    set_context();
    this->name = n.value.empty() ? "<anonymous>" : n.value;
}

void BaseFunction::generate_new_context(Context& new_context) {
    new_context = Context(name, &context, pos_start);
    new_context.parent_entry_pos = pos_start;
    new_context.parent_entry_pos.file_name = context.parent_entry_pos.file_name;
    new_context.parent_entry_pos.file_text = context.parent_entry_pos.file_text;
    new_context.symbol_table = SymbolTable(&context.symbol_table);
}

RuntimeResult* BaseFunction::check_args(std::vector<Value*> args, std::vector<Token> arg_name_tokens) {
    RuntimeResult rt_result = RuntimeResult();
    if (args.size() != arg_name_tokens.size()) {
        return rt_result.failure(new RuntimeError(pos_start, pos_end, "Expected " + std::to_string(arg_name_tokens.size()) + " arguments, but got " + std::to_string(args.size()), context));
    }
    return rt_result.success(new Number());
}

void BaseFunction::populate_args(std::vector<Value*> args, std::vector<Token> arg_name_tokens, Context& new_context) {
    for (int i = 0; i < args.size(); i++) {
        Token arg_name_token = arg_name_tokens[i];
        Value* arg_value = args[i];
        arg_value->set_context(new_context);
        new_context.symbol_table.set(arg_name_token.value, arg_value->copy());
    }
} 

RuntimeResult BaseFunction::check_and_populate_args(std::vector<Value*> args, std::vector<Token> arg_name_tokens, Context& new_context) {
    RuntimeResult rt_result = RuntimeResult();
    rt_result.reg(*this->check_args(args, arg_name_tokens));
    if (rt_result.should_return()) {
        return rt_result;
    }
    this->populate_args(args, arg_name_tokens, new_context);
    return *rt_result.success(new Number());
}

Function::Function(Token var_name_token, std::vector<Token> arg_name_tokens, Node* body_node, bool should_auto_return) 
    : BaseFunction(var_name_token) {
        this->body_node = body_node;
        this->arg_name_tokens = arg_name_tokens;
        this->name = var_name_token.value;
        this->should_auto_return = should_auto_return;
    };;

std::pair<RuntimeResult*, Error*> Function::execute(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    Interpreter interpreter = Interpreter(nullptr);

    Context exec_ctx;
    generate_new_context(exec_ctx);

    rt_result->reg(check_and_populate_args(args, arg_name_tokens, exec_ctx));
    if (rt_result->should_return()) {
        return std::pair(rt_result, new Error(Position(), Position(), "", ""));
    }

    Value* value = rt_result->reg(interpreter.visit(body_node, exec_ctx));
    if (rt_result->error->err_name != "") {
        rt_result->error->pos_start = pos_start;
        return std::pair(rt_result, rt_result->error);
    }
    
    if (rt_result->should_return() && rt_result->func_return_value==nullptr) {
        rt_result->error->pos_start = pos_start;
        return std::pair(rt_result, new Error(Position(), Position(), "", ""));
    }

    Value* ret_value;
    if (rt_result->func_return_value != nullptr) {
        ret_value = rt_result->func_return_value;
    } else if (should_auto_return) {
        ret_value = value;
    } else {
        ret_value = new Number(static_cast<int64_t>(0));
    }

    return std::pair(rt_result->success(ret_value), new Error(Position(), Position(), "", ""));
}

Function* Function::copy() {
    Function* func = new Function(this->var_name_token, this->arg_name_tokens, this->body_node, this->should_auto_return);
    func->set_context(this->context);
    func->set_pos(this->pos_start, this->pos_end);
    return func;
}

std::string Function::repr() {
    return "<function " + name + ">";
}

// ************BUILT-IN FUNCTIONS************

BuiltinFunction::BuiltinFunction(Token name) : BaseFunction(name) {
    this->name = name.value.empty() ? name.repr() : name.value;
} 

std::pair<RuntimeResult*, Error*> BuiltinFunction::execute(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    Context exec_ctx;
    generate_new_context(exec_ctx);

    Value* return_value;
    if (this->name=="print") {
        return_value = rt_result->reg(*this->func_print(args));
    } else if (this->name=="input") {
        return_value = rt_result->reg(*this->func_input(args));
    } else if (this->name=="input_int") {
        return_value = rt_result->reg(*this->func_input_int(args));
    } else if (this->name=="clear") {
        return_value = rt_result->reg(*this->func_clear(args));
    } else if (this->name=="is_number") {
        return_value = rt_result->reg(*this->func_is_number(args));
    } else if (this->name=="is_string") {
        return_value = rt_result->reg(*this->func_is_string(args));
    } else if (this->name=="is_list") {
        return_value = rt_result->reg(*this->func_is_list(args));
    } else if (this->name=="is_function") {
        return_value = rt_result->reg(*this->func_is_function(args));
    } else if (this->name=="append") {
        return_value = rt_result->reg(*this->func_append(args));
    } else if (this->name=="pop") {
        return_value = rt_result->reg(*this->func_pop(args));
    } else if (this->name=="extend") {
        return_value = rt_result->reg(*this->func_extend(args));
    } else if (this->name=="len") {
        return_value = rt_result->reg(*this->func_len(args));
    } else if (this->name=="run") {
        return_value = rt_result->reg(*this->func_run(args, exec_ctx));
    } else {
        return std::pair(rt_result->failure(new RuntimeError(pos_start, pos_end, "No built-in function with name " + this->name, context)), new Error(Position(), Position(), "", ""));
    }

    if (rt_result->should_return()) {
        return std::pair(rt_result, new Error(Position(), Position(), "", ""));
    }

    return std::pair(rt_result->success(return_value), new Error(Position(), Position(), "", ""));
}

BuiltinFunction* BuiltinFunction::copy() {
    BuiltinFunction* func = new BuiltinFunction(this->name);
    func->set_context(this->context);
    func->set_pos(this->pos_start, this->pos_end);
    return func;
}

std::string BuiltinFunction::repr() {
    return "<built-in function " + this->name + ">";
}

RuntimeResult* BuiltinFunction::func_print(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    std::string output = "";

    for (Value* arg : args) {
        if (auto arg_str = dynamic_cast<String*>(arg)) {
            output += arg_str->repr();
        } else if (auto arg_num = dynamic_cast<Number*>(arg)) {
            output += arg_num->repr();
        } else if (auto arg_list = dynamic_cast<List*>(arg)) {
            output += arg_list->repr();
        }
    }
    #ifdef __EMSCRIPTEN__
        EM_ASM({
            Module.print(UTF8ToString($0));
        }, output.c_str());
    #else
        std::cout << output << std::endl;
    #endif
    return rt_result->success(new Number());
}

RuntimeResult* BuiltinFunction::func_input(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    std::string input;
    std::getline(std::cin, input);
    return rt_result->success(new String(input));
}

RuntimeResult* BuiltinFunction::func_input_int(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    std::string input;
    std::getline(std::cin, input);
    try {
        int64_t num = std::stoi(input);
        return rt_result->success(new Number(num));
    } catch (std::invalid_argument) {
        return rt_result->failure(new RuntimeError(Position(), Position(), "Invalid input", context));
    }
}

RuntimeResult* BuiltinFunction::func_clear(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    // check os and clear screen
    #ifdef _WIN32
        system("cls");
    #else
        system("clear");
    #endif
    return rt_result->success(new Number());
}

RuntimeResult* BuiltinFunction::func_is_number(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto arg = dynamic_cast<Number*>(args[0])) {
        return rt_result->success(new Number(static_cast<int64_t>(1)));
    }
    return rt_result->success(new Number(static_cast<int64_t>(0)));
}

RuntimeResult* BuiltinFunction::func_is_string(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto arg = dynamic_cast<String*>(args[0])) {
        return rt_result->success(new Number(static_cast<int64_t>(1)));
    }
    return rt_result->success(new Number(static_cast<int64_t>(0)));
}

RuntimeResult* BuiltinFunction::func_is_list(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto arg = dynamic_cast<List*>(args[0])) {
        return rt_result->success(new Number(static_cast<int64_t>(1)));
    }
    return rt_result->success(new Number(static_cast<int64_t>(0)));
}

RuntimeResult* BuiltinFunction::func_is_function(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto arg = dynamic_cast<Function*>(args[0])) {
        return rt_result->success(new Number(static_cast<int64_t>(1)));
    }
    return rt_result->success(new Number(static_cast<int64_t>(0)));
}

RuntimeResult* BuiltinFunction::func_append(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto list = dynamic_cast<List*>(args[0])) {
        Value* value_copy = args[1]->copy();
        list->elements.push_back(value_copy);
        return rt_result->success(list);
    }
    return rt_result->failure(new RuntimeError(Position(), Position(), "First argument must be a list", context));
}

RuntimeResult* BuiltinFunction::func_pop(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto list = dynamic_cast<List*>(args[0])) {
        try {
            list->elements.erase(list->elements.begin() + static_cast<int>(static_cast<Number*>(args[1])->getAsInt()));
        } catch (std::out_of_range) {
            return rt_result->failure(new RuntimeError(Position(), Position(), "Index out of range", context));
        }
        return rt_result->success(list);
    }
    return rt_result->failure(new RuntimeError(Position(), Position(), "First argument must be a list", context));
}

RuntimeResult* BuiltinFunction::func_extend(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto list = dynamic_cast<List*>(args[0])) {
        if (auto other_list = dynamic_cast<List*>(args[1])) {
            list->elements.insert(list->elements.end(), other_list->elements.begin(), other_list->elements.end());
            return rt_result->success(list);
        }
        return rt_result->failure(new RuntimeError(Position(), Position(), "Second argument must be a list", context));
    }
    return rt_result->failure(new RuntimeError(Position(), Position(), "First argument must be a list", context));
}

RuntimeResult* BuiltinFunction::func_len(std::vector<Value*> args) {
    RuntimeResult* rt_result = new RuntimeResult();
    if (auto list = dynamic_cast<List*>(args[0])) {
        return rt_result->success(new Number(static_cast<int64_t>(list->elements.size())));
    }
    return rt_result->failure(new RuntimeError(Position(), Position(), "Argument must be a list", context));
}

RuntimeResult* BuiltinFunction::func_run(std::vector<Value*> args, Context& context) {
    RuntimeResult* rt_result = new RuntimeResult();

    // check if args is empty 
    if (args.empty()) {
        return rt_result->failure(new RuntimeError(pos_start, pos_end, "run() requires a filename argument", context));
    }

    // check if filename is a string
    if (auto file_name = dynamic_cast<String*>(args[0])) {
        std::string file_value = file_name->value;
        std::string script;

        std::ifstream file(file_value);
        if (!file.is_open()) {
            return rt_result->failure(new RuntimeError(pos_start, pos_end, "File: '" + file_value + "' not found", context));
        }

        try {
            script = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
        } catch (const std::exception& e) {
            return rt_result->failure(new RuntimeError(pos_start, pos_end, "File: '" + file_value + "' not found\n" + std::string(e.what()), context));
        }

        file.close();
        
        std::pair<Value*, Error*> out = run(file_value, script);
        Value* _ = out.first;
        Error* error = out.second;

        if (error->err_name != "") {
            return rt_result->failure(new RuntimeError(Position(), Position(), "Error while executing script: '" + file_value + "'\n" + error->as_string(), context));
        }

        return rt_result->success(new Number(static_cast<int64_t>(0)));
    } 
    return rt_result->failure(new RuntimeError(this->pos_start, this->pos_end, "Filename must be a string", context));
}

// ************LISTS************

List::List() {
    set_pos();
    set_context();
}

List::List(std::vector<Value*> elements) {
    this->elements = elements;
    set_pos();
    set_context();
}

List::~List() {
    for (Value* element : elements) {
        delete element;
    }
}

std::pair<Value*, Error*> List::added_to(Value* other) {
    List* new_list = copy();
    if (auto other_list = dynamic_cast<List*>(other)) {
        new_list->elements.insert(new_list->elements.end(), other_list->elements.begin(), other_list->elements.end());
    } else {
        return std::pair(new List(), illegal_operation(other));
    }
    return std::pair(new_list, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> List::multiplied_by(Value* other) {
    List* new_list = copy();
    if (auto other_num = dynamic_cast<Number*>(other)) {
        for (int i = 0; i < other_num->getAsInt(); i++) {
            new_list->elements.insert(new_list->elements.end(), elements.begin(), elements.end());
        }
    } else {
        return std::pair(new List(), illegal_operation(other));
    }
    return std::pair(new_list, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> List::subtracted_by(Value* other) {
    List* new_list = copy();
    if (auto index = dynamic_cast<Number*>(other)) {
        try {
            new_list->elements.erase(new_list->elements.begin() + index->getAsInt());
        } catch (std::out_of_range) {
            return std::pair(new List(), new RuntimeError(Position(), Position(), "Index out of range", context));
        }
    } else {
        return std::pair(new List(), illegal_operation(other));
    }
    return std::pair(new_list, new Error(Position(), Position(), "", ""));
}

std::pair<Value*, Error*> List::divided_by(Value* other) {
    std::pair<Value*, Error*> ret;
    if (auto index = dynamic_cast<Number*>(other)) {
        // check if it is negative, and if negative, access from end
        if (index->getAsInt() < 0) {
            index = new Number(static_cast<int64_t>(elements.size() + index->getAsInt()));
        }
        if (index->getAsInt() >= elements.size() || index->getAsInt() < 0) {
            return std::pair(new List(), new RuntimeError(pos_start, pos_end, "Index out of range", context));
        }
        ret = std::pair(elements[index->getAsInt()], new Error(Position(), Position(), "", ""));
        return ret;
    } else {
        return std::pair(new List(), illegal_operation(other));
    }
}

List* List::copy() {
    List* list = new List(elements);
    list->set_pos(pos_start, pos_end);
    list->set_context(context);
    return list;
}

// ************REPRESENTATION METHODS************

std::string Number::repr() {
    if (type == INTEGER) {
        return std::to_string(int_value);
    } else {
        return std::to_string(float_value);
    }
}

std::string String::repr() {
    return value;
}

std::string List::repr() {
    std::string str = "[";
    for (int i = 0; i < elements.size(); i++) {
        if (auto element = dynamic_cast<String*>(elements[i])) {
            str += element->repr();
        } else if (auto element = dynamic_cast<Number*>(elements[i])) {
            str += element->repr();
        } else if (auto element = dynamic_cast<List*>(elements[i])) {
            str += element->repr();
        } 
        if (i != elements.size() - 1) {
            str += ", ";
        }
    }
    str += "]";
    return str;
}

// ************RUN FUNCTION************

std::string run_script_cpp(std::string script) {
    std::pair<Value*, Error*> out = run("<stdin>", script);
    Value* value = out.first;
    Error* error = out.second;
    std::string result;

    if (error->err_name != "" && error->err_name != "EOF") {
        result = error->as_string();
    } else {
        std::string out_value = value->repr();
        // Split output by newlines
        size_t pos = 0;
        size_t last_pos = 0;
        // Only print lines before the last one (which contains the return value)
        while ((pos = out_value.find('\n', last_pos)) != std::string::npos) {
            std::string line = out_value.substr(last_pos, pos - last_pos);
            result += line + "\n";
            last_pos = pos + 1;
        }
    }
    return result;
}