#include <iostream>
#include <string>
#include <cstdio>
#include <ctype.h>
#include <cstdlib>
#include <vector>
#include <stack>
#include <algorithm>

using namespace std;

// Enumeration for lexical types
enum type_of_lex {
    LEX_NULL,                                                                                   
    LEX_AND, LEX_BEGIN, LEX_BOOL, LEX_DO, LEX_ELSE, LEX_END, LEX_IF, LEX_FALSE, LEX_INT,       
    LEX_NOT, LEX_OR, LEX_PROGRAM, LEX_READ, LEX_THEN, LEX_TRUE, LEX_VAR, LEX_WHILE, LEX_WRITE,  
    LEX_PROCEDURE, LEX_FUNCTION, LEX_ARRAY, LEX_OF, LEX_FIN,                                    
    LEX_SEMICOLON, LEX_COMMA, LEX_COLON, LEX_ASSIGN, LEX_LPAREN, LEX_RPAREN, LEX_EQ, LEX_LSS,   
    LEX_GTR, LEX_PLUS, LEX_MINUS, LEX_TIMES, LEX_SLASH, LEX_LEQ, LEX_NEQ, LEX_GEQ,              
    LEX_LMAS, LEX_RMAS, LEX_NUM,                                                                
    LEX_ID, LEX_PID, LEX_PBOOL, LEX_PINT, LEX_FID, LEX_ARR,                                     
    POLIZ_LABEL,                                                                                
    POLIZ_ADDRESS,                                                                              
    POLIZ_GO,                                                                                   
    POLIZ_FGO                                                                                   
};

// Class representing a lexical unit
class Lex {
    type_of_lex t_lex;
    int v_lex;
public:
    Lex (type_of_lex t = LEX_NULL, int v = 0): t_lex(t), v_lex(v)  { }

    type_of_lex  get_type() const {
        return t_lex;
    }

    int get_value() const {
        return v_lex;
    }
    friend ostream & operator<< (ostream &s, Lex l);
};

// Class representing an identifier
class Ident {
    string name;
    string proc_name;
    bool declare;
    type_of_lex type;
    bool assign;
    int value;
public:
    Ident() {
        declare = false;
        assign  = false;
    }
    bool operator== (const string& s) const {
        return name == s;
    }
    Ident (const string n) {
        name = n;
        proc_name = "";
        declare = false;
        assign  = false;
    }
    string get_name() const {
        return name;
    }
    string get_proc_name() const {
        return proc_name;
    }
    bool get_declare() const {
        return declare;
    }
    void put_declare() {
        declare = true;
    }
    type_of_lex get_type() const {
        return type;
    }
    void put_type(type_of_lex t) {
        type = t;
    }
    void put_proc_name(string name) {
        proc_name  = name;
    }
    bool get_assign() const {
        return assign;
    }
    void put_assign() {
        assign = true;
    }
    int  get_value() const {
        return value;
    }
    void put_value(int v) {
        value = v;
    }
};

vector <Ident> TID; // Vector to store identifiers

// Function to add identifier to the table if not already present
int put(const string & buf) {
    vector<Ident>::iterator k;
    if (( k = find(TID.begin (), TID.end(), buf)) != TID.end() )
        return k - TID.begin();
    TID.push_back(Ident(buf));
    return TID.size() - 1;
}

// Class for lexical scanner
class Scanner {
    FILE * fp;
    char c;
    int look(const string buf, const char ** list) {
        int i = 0;
        while (list[i]) {
            if (buf == list[i])
                return i;
            ++i;
        }
        return 0;
    }
    void gc () {
        c = fgetc (fp);
    }
public:
    static const char * TW [], * TD [];

    Scanner (const char * program ) {
        if (!(fp = fopen(program, "r")))
            throw  "Can't open file" ;
    }

    Lex get_lex();
};

// List of reserved words
const char *
        Scanner::TW    [] = { "", "and", "begin", "boolean", "do", "else", "end", "if", "false", "integer", "not", "or", "program",
                              "read", "then", "true", "var", "while", "write", "procedure", "function", "array", "of", NULL };

// List of special symbols
const char *
        Scanner::TD    [] = {".", ";", ",", ":", ":=", "(", ")", "=", "<", ">", "+", "-", "*", "/", "<=", "!=", ">=", "[", "]", NULL };


// Lexical analysis function
Lex Scanner::get_lex() {
    // Enumeration for lexical analysis states
    enum state { H, IDENT, NUMB, COM, ALE, NEQ };
    int d, j;
    string buf;  // Buffer to store lexemes
    state CS = H;  // Initial state

    // Loop to iterate through characters and perform lexical analysis
    do {
        gc();  // Get next character from input

        switch (CS) {
            // Initial state
            case H:
                // Ignore whitespace characters
                if (c == ' ' || c == '\n' || c == '\r' || c == '\t');
                // If the character is alphabetic, start parsing an identifier
                else if (isalpha(c)) {
                    buf.push_back(c);
                    CS = IDENT;
                }
                // If the character is a digit, start parsing a number
                else if (isdigit(c)) {
                    d = c - '0';  // Initialize the number
                    CS = NUMB;
                }
                // If encountering '{', start parsing a comment
                else if (c == '{') {
                    CS = COM;
                }
                // If encountering ':', '<', or '>', continue parsing for operators
                else if (c == ':' || c == '<' || c == '>') {
                    buf.push_back(c);
                    CS = ALE;
                }
                // If encountering '.', return end of program token
                else if (c == '.')
                    return Lex(LEX_FIN);
                // If encountering '!', start parsing for not equal operator
                else if (c == '!') {
                    buf.push_back(c);
                    CS = NEQ;
                }
                // For other characters, check if they are special symbols or operators
                else {
                    buf.push_back(c);
                    if ((j = look(buf, TD))) {
                        // If found, return the corresponding token
                        return Lex((type_of_lex)(j + (int)LEX_FIN), j);
                    } else
                        throw c;  // If not found, throw an error
                }
                break;

            // Parsing identifier
            case IDENT:
                if (isalpha(c) || isdigit(c)) {
                    buf.push_back(c);  // Continue appending characters to the buffer
                } else {
                    ungetc(c, fp);  // Put back the non-alphanumeric character
                    // Check if the identifier is a reserved word
                    if ((j = look(buf, TW))) {
                        return Lex((type_of_lex)j, j);  // Return the corresponding token
                    } else {
                        // If not a reserved word, it's an identifier
                        j = put(buf);  // Add the identifier to the symbol table
                        return Lex(LEX_ID, j);  // Return the identifier token
                    }
                }
                break;

            // Parsing number
            case NUMB:
                if (isdigit(c)) {
                    d = d * 10 + (c - '0');  // Append digits to the number
                } else {
                    ungetc(c, fp);  // Put back the non-digit character
                    return Lex(LEX_NUM, d);  // Return the number token
                }
                break;

            // Parsing comment
            case COM:
                if (c == '}') {
                    CS = H;  // If '}' is encountered, go back to the initial state
                } else if (c == '.' || c == '{')
                    throw c;  // Throw an error if '.' or '{' is encountered inside a comment
                break;

            // Parsing operators
            case ALE:
                if (c == '=') {
                    buf.push_back(c);
                    j = look(buf, TD);
                    return Lex((type_of_lex)(j + (int)LEX_FIN), j);  // Return the corresponding token
                } else {
                    ungetc(c, fp);  // Put back the non-'=' character
                    j = look(buf, TD);
                    return Lex((type_of_lex)(j + (int)LEX_FIN), j);  // Return the corresponding token
                }
                break;

            // Parsing not equal operator
            case NEQ:
                if (c == '=') {
                    buf.push_back(c);
                    j = look(buf, TD);
                    return Lex(LEX_NEQ, j);  // Return the not equal token
                } else
                    throw '!';  // Throw an error if '=' is not encountered after '!'
                break;
        }
    } while (true);  // Infinite loop until a token is returned or an error is thrown
}


ostream & operator<< (ostream &s, Lex l) {
    string t;
    if (l.t_lex <= LEX_WRITE)
        t = Scanner::TW[l.t_lex];
    else if (l.t_lex >= LEX_FIN && l.t_lex <= LEX_GEQ)
        t = Scanner::TD[l.t_lex - LEX_FIN];
    else if (l.t_lex == LEX_NUM)
        t = "NUMB";
    else if (l.t_lex == LEX_ID)
        t = TID[l.v_lex].get_name();
    else if (l.t_lex == POLIZ_LABEL)
        t = "Label";
    else if (l.t_lex == POLIZ_ADDRESS)
        t = "Addr";
    else if (l.t_lex == POLIZ_GO)
        t = "!";
    else if (l.t_lex == POLIZ_FGO)
        t = "!F";
    else
        throw l;
    s << '(' << t << ',' << l.v_lex << ");" << endl;
    return s;
}


template <class T, class T_EL>
void from_st(T &st, T_EL &i) {
    i = st.top();
    st.pop();
}

class Parser {
    Lex curr_lex;
    type_of_lex c_type;
    int c_val;
    string curr_proc_name;
    Scanner scan;
    stack<int> st_int;
    stack<type_of_lex> st_lex;

    // Declaration of parsing functions
    void P();
    void D1();
    void OD();
    void OP();
    void OF();
    void DP();
    void D();
    void B();
    void S();
    void E();
    void E1();
    void T();
    void F();

    // Declaration of semantic checking functions
    void dec(type_of_lex type);
    void proc_dec(type_of_lex type);
    void check_id();
    void check_op();
    void check_not();
    void eq_type();
    void eq_bool();
    void check_id_in_read();

    // Lexical analysis helper function
    void gl() {
        curr_lex = scan.get_lex();
        c_type = curr_lex.get_type();
        c_val = curr_lex.get_value();
    }

public:
    vector<Lex> poliz;
    Parser(const char *program) : scan(program) {}

    // Main analysis function
    void analyze();
};

// Method to start the syntax analysis
void Parser::analyze() {
    gl(); // Get the first lexical token
    P();  // Start parsing the program
    // Ensure that the program ends correctly
    if (c_type != LEX_FIN)
        throw curr_lex;

    // Output the resulting Poliz and Symbol Table
    int i = 0;
    cout << "POLIZ" << endl;
    for (Lex l : poliz)
        cout << i++ << "| " << l;
    cout << "TID" << endl;
    for (int j = 0; j < TID.size(); j++) {
        cout << j << "| " << TID[j].get_name() << " " << TID[j].get_type() << " " << TID[j].get_value() << " " << TID[j].get_proc_name() << endl;
    }
    cout << endl << "Yes!!!" << endl;
}

// Parsing the program
void Parser::P() {
    int beg_proc;

    if (c_type == LEX_PROGRAM) {
        gl();
    } else {
        throw curr_lex;
    }

    // Generate Poliz code for the program
    beg_proc = poliz.size();
    poliz.push_back(Lex());
    poliz.push_back(Lex(POLIZ_GO));

    D1(); // Parse variable declarations or procedure/function declarations
    if (c_type == LEX_SEMICOLON)
        gl();
    else
        throw curr_lex;

    // Set Poliz label for the beginning of the main block
    poliz[beg_proc] = Lex(POLIZ_LABEL, poliz.size());

    B(); // Parse the main block
}

// Parsing variable or procedure/function declarations
void Parser::D1() {
    if (c_type == LEX_VAR) {
        OD();
    } else if (c_type == LEX_PROCEDURE) {
        OP();
        while (c_type == LEX_COMMA) {
            gl();
            D1();
        }
    } else if (c_type == LEX_FUNCTION) {
        OF();
        while (c_type == LEX_COMMA) {
            gl();
            D1();
        }
    } else if (c_type == LEX_SEMICOLON);
    else
        throw curr_lex;
}

// Parsing variable declarations
void Parser::OD() {
    if (c_type == LEX_VAR) {
        gl();
        D();
        while (c_type == LEX_COMMA) {
            gl();
            D();
        }
    } else
        throw curr_lex;
}

// Parsing procedure declarations
void Parser::OP() {
    if (c_type == LEX_PROCEDURE) {
        gl();
        if (c_type != LEX_ID)
            throw curr_lex;
        st_int.push(c_val);
        dec(LEX_PID);
        gl();
        if (c_type != LEX_LPAREN)
            throw curr_lex;
        gl();
        if (c_type == LEX_RPAREN);
        else if (c_type == LEX_ID) {
            DP();
            while (c_type == LEX_COMMA) {
                gl();
                DP();
            }
            if (c_type != LEX_RPAREN)
                throw curr_lex;
        } else
            throw curr_lex;
        gl();
        if (c_type != LEX_SEMICOLON)
            throw curr_lex;
        gl();
        OD();
        if (c_type != LEX_SEMICOLON)
            throw curr_lex;
        gl();
        B();

        poliz.push_back(Lex(POLIZ_GO));
    } else
        throw curr_lex;
}

// Parsing function declarations
void Parser::OF() {
    if (c_type == LEX_FUNCTION) {
        gl();
        if (c_type != LEX_ID)
            throw curr_lex;
        st_int.push(c_val);
        dec(LEX_FID);
        gl();
        if (c_type != LEX_LPAREN)
            throw curr_lex;
        gl();
        if (c_type == LEX_RPAREN);
        else if (c_type == LEX_ID) {
            DP();
            while (c_type == LEX_COMMA) {
                gl();
                DP();
            }
            if (c_type != LEX_RPAREN)
                throw curr_lex;
        } else
            throw curr_lex;
        gl();
        if (c_type != LEX_COLON)
            throw curr_lex;
        gl();
        if (c_type == LEX_INT) {
            gl();
        } else if (c_type == LEX_BOOL) {
            gl();
        } else
            throw curr_lex;
        if (c_type != LEX_SEMICOLON)
            throw curr_lex;
        gl();
        OD();
        if (c_type != LEX_SEMICOLON)
            throw curr_lex;
        gl();
        B();

        poliz.push_back(Lex(POLIZ_GO));
    } else
        throw curr_lex;
}

void Parser::DP() {
    // Function to parse declarations with variable initialization
    if (c_type != LEX_ID)
        throw curr_lex;
    else {
        // Push current token's value onto the stack
        st_int.push(c_val);
        // Get the next token
        gl();
        while (c_type == LEX_COMMA) {
            // Handle comma-separated list of identifiers
            gl();
            if (c_type != LEX_ID)
                throw curr_lex;
            else {
                st_int.push(c_val);
                gl();
            }
        }
        // Expect a colon after identifiers
        if (c_type != LEX_COLON)
            throw curr_lex;
        else {
            gl();
            // Check type and process declaration
            if (c_type == LEX_INT) {
                proc_dec(LEX_INT);
                gl();
            } else if (c_type == LEX_BOOL) {
                proc_dec(LEX_BOOL);
                gl();
            } else
                throw curr_lex;
        }
    }
}

void Parser::D() {
    // Function to parse declarations without initialization
    vector<string> name_id;
    string array_name;

    if (c_type != LEX_ID)
        throw curr_lex;
    else {
        st_int.push(c_val);
        name_id.push_back(TID[c_val].get_name());
        gl();
        while (c_type == LEX_COMMA) {
            gl();
            if (c_type != LEX_ID)
                throw curr_lex;
            else {
                st_int.push(c_val);
                name_id.push_back(TID[c_val].get_name());
                gl();
            }
        }
        if (c_type != LEX_COLON)
            throw curr_lex;
        else {
            gl();
            // Handle array declaration
            if (c_type == LEX_INT || c_type == LEX_BOOL) {
                dec(c_type);
                gl();
            } else if (c_type == LEX_ARRAY) {
                gl();
                // Check for left square bracket
                if (c_type != LEX_LMAS)
                    throw curr_lex;
                gl();
                // Expect array size
                if (c_type != LEX_NUM)
                    throw curr_lex;

                // Iterate over identifiers and create array entries
                for (int j = 0; j < name_id.size(); j++) {
                    for (int i = 0; i < c_val; i++) {
                        TID.push_back(Ident(name_id[j]));
                    }
                }

                gl();
                // Expect right square bracket
                if (c_type != LEX_RMAS)
                    throw curr_lex;
                gl();
                // Expect 'OF' keyword
                if (c_type != LEX_OF)
                    throw curr_lex;
                gl();
                // Check array type and process declaration
                if (c_type == LEX_INT) {
                    cout << "INT ARRAY!!!" << endl;
                    dec(LEX_ARR);
                    gl();
                } else if (c_type == LEX_BOOL) {
                    cout << "BOOL ARRAY!!!" << endl;
                    dec(LEX_ARR);
                    gl();
                } else
                    throw curr_lex;
            } else
                throw curr_lex;
        }
    }
}

void Parser::B() {
    // Function to parse a block of statements
    if (c_type == LEX_BEGIN) {
        gl();
        if (c_type == LEX_END) {
            gl();
        } else {
            S();
            while (c_type == LEX_SEMICOLON) {
                gl();
                S();
            }
            if (c_type == LEX_END) {
                gl();
            } else {
                throw curr_lex;
            }
        }
    } else
        throw curr_lex;
}


void Parser::S() {
    // Function to parse statements
    int pl0, pl1, pl2, pl3;
    int proc_val;

    if (c_type == LEX_IF) {
        // Parse if statement
        gl();
        E();
        eq_bool();
        pl2 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        if (c_type == LEX_THEN) {
            gl();
            S();
            pl3 = poliz.size();
            poliz.push_back(Lex());

            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl2] = Lex(POLIZ_LABEL, poliz.size());

            if (c_type == LEX_ELSE) {
                gl();
                S();
                poliz[pl3] = Lex(POLIZ_LABEL, poliz.size());
            } else
                throw curr_lex;
        } else
            throw curr_lex;
    } else if (c_type == LEX_WHILE) {
        // Parse while loop
        pl0 = poliz.size();
        gl();
        E();
        eq_bool();
        pl1 = poliz.size();
        poliz.push_back(Lex());
        poliz.push_back(Lex(POLIZ_FGO));
        if (c_type == LEX_DO) {
            gl();
            S();
            poliz.push_back(Lex(POLIZ_LABEL, pl0));
            poliz.push_back(Lex(POLIZ_GO));
            poliz[pl1] = Lex(POLIZ_LABEL, poliz.size());
        } else
            throw curr_lex;
    } else if (c_type == LEX_READ) {
        // Parse read statement
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            if (c_type == LEX_ID) {
                check_id_in_read();
                poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
                gl();
            } else
                throw curr_lex;
            if (c_type == LEX_RPAREN) {
                gl();
                poliz.push_back(Lex(LEX_READ));
            } else
                throw curr_lex;
        } else
            throw curr_lex;
    } else if (c_type == LEX_WRITE) {
        // Parse write statement
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            E();
            if (c_type == LEX_RPAREN) {
                gl();
                poliz.push_back(Lex(LEX_WRITE));
            } else
                throw curr_lex;
        } else
            throw curr_lex;
    } else if (c_type == LEX_ID && TID[c_val].get_type() != LEX_PID) {
        // Parse assignment statement
        check_id();
        poliz.push_back(Lex(POLIZ_ADDRESS, c_val));
        gl();
        if (c_type == LEX_ASSIGN) {
            gl();
            E();
            eq_type();
            poliz.push_back(Lex(LEX_ASSIGN));
        } else
            throw curr_lex;
    } else if (c_type == LEX_ID && TID[c_val].get_type() == LEX_PID) {
        // Parse procedure call
        poliz.push_back(Lex(POLIZ_LABEL, poliz.size() + 3));
        proc_val = c_val;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            if (c_type == LEX_RPAREN) ;
            else if (c_type == LEX_ID || c_type == LEX_NUM) {
                // Parse parameters
                if (c_type == LEX_ID)
                    poliz.push_back(Lex(LEX_ID, c_val));
                else
                    poliz.push_back(Lex(LEX_NUM, c_val));
                gl();
                while (c_type == LEX_COMMA) {
                    gl();
                    if (c_type == LEX_ID)
                        poliz.push_back(Lex(LEX_ID, c_val));
                    else
                        poliz.push_back(Lex(LEX_NUM, c_val));
                    gl();
                }
                if (c_type != LEX_RPAREN)
                    throw curr_lex;
            } else
                throw curr_lex;
            gl();
        } else
            throw curr_lex;

        poliz.push_back(Lex(POLIZ_LABEL, TID[proc_val].get_value()));
        poliz.push_back(Lex(POLIZ_GO));
    } else
        B();  // Parse block of statements
}


void Parser::E() {
    // Expression parsing function
    E1();
    if (c_type == LEX_EQ || c_type == LEX_LSS || c_type == LEX_GTR ||
        c_type == LEX_LEQ || c_type == LEX_GEQ || c_type == LEX_NEQ) {
        st_lex.push(c_type);
        gl();
        E1();
        check_op();
    }
}

void Parser::E1() {
    // Expression parsing function for addition and subtraction
    T();
    while (c_type == LEX_PLUS || c_type == LEX_MINUS || c_type == LEX_OR) {
        st_lex.push(c_type);
        gl();
        T();
        check_op();
    }
}

void Parser::T() {
    // Expression parsing function for multiplication, division, and logical AND
    F();
    while (c_type == LEX_TIMES || c_type == LEX_SLASH || c_type == LEX_AND) {
        st_lex.push(c_type);
        gl();
        F();
        check_op();
    }
}

void Parser::F() {
    // Factor parsing function
    int proc_val;
    if (c_type == LEX_ID && TID[c_val].get_type() != LEX_FID) {
        poliz.push_back(Lex(LEX_ID, c_val));
        gl();
        if (c_type == LEX_LMAS) {
            cout << "MAS!!" << endl;
            gl();
            if (c_type != LEX_NUM)
                throw curr_lex;
            gl();
            if (c_type != LEX_RMAS)
                throw curr_lex;
            gl();
        }
    } else if (c_type == LEX_ID && TID[c_val].get_type() == LEX_FID) {
        st_lex.push(LEX_INT);
        poliz.push_back(Lex(POLIZ_LABEL, poliz.size() + 3));
        proc_val = c_val;
        gl();
        if (c_type == LEX_LPAREN) {
            gl();
            if (c_type == LEX_RPAREN);
            else if (c_type == LEX_ID || c_type == LEX_NUM) {
                if (c_type == LEX_ID)
                    poliz.push_back(Lex(LEX_ID, c_val));
                else
                    poliz.push_back(Lex(LEX_NUM, c_val));
                gl();
                while (c_type == LEX_COMMA) {
                    gl();
                    if (c_type == LEX_ID)
                        poliz.push_back(Lex(LEX_ID, c_val));
                    else
                        poliz.push_back(Lex(LEX_NUM, c_val));
                    gl();
                }
                if (c_type != LEX_RPAREN)
                    throw curr_lex;
            } else
                throw curr_lex;
            gl();
        } else
            throw curr_lex;

        poliz.push_back(Lex(POLIZ_LABEL, TID[proc_val].get_value()));
        poliz.push_back(Lex(POLIZ_GO));
    } else if (c_type == LEX_NUM) {
        st_lex.push(LEX_INT);
        poliz.push_back(curr_lex);
        gl();
    } else if (c_type == LEX_TRUE) {
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_TRUE, 1));
        gl();
    } else if (c_type == LEX_FALSE) {
        st_lex.push(LEX_BOOL);
        poliz.push_back(Lex(LEX_FALSE, 0));
        gl();
    } else if (c_type == LEX_NOT) {
        gl();
        F();
        check_not();
    } else if (c_type == LEX_LPAREN) {
        gl();
        E();
        if (c_type == LEX_RPAREN)
            gl();
        else
            throw curr_lex;
    } else
        throw curr_lex;
}

void Parser::dec(type_of_lex type) {
    // Function to declare variables
    int i;
    while (!st_int.empty()) {
        from_st(st_int, i);
        if (TID[i].get_declare())
            throw "twice";
        else {
            TID[i].put_declare();
            TID[i].put_type(type);
            if (type == LEX_PID || type == LEX_FID) {
                TID[i].put_value(poliz.size());
                TID[i].put_assign();
                curr_proc_name = TID[i].get_name();
            }
        }
    }
}

void Parser::proc_dec(type_of_lex type) {
    // Function to declare procedures
    int i;
    while (!st_int.empty()) {
        from_st(st_int, i);
        if (TID[i].get_declare() && (TID[i].get_proc_name() == curr_proc_name))
            throw "twice";
        else {
            TID[i].put_declare();
            TID[i].put_type(type);
            TID[i].put_proc_name(curr_proc_name);
        }
    }
}

void Parser::check_id() {
    // Function to check if an identifier is declared
    if (TID[c_val].get_declare())
        st_lex.push(TID[c_val].get_type());
    else
        throw "not declared";
}

void Parser::check_op() {
    // Function to check if operation types match
    type_of_lex t1, t2, op, t = LEX_INT, r = LEX_BOOL;

    from_st(st_lex, t2);
    from_st(st_lex, op);
    from_st(st_lex, t1);

    if (op == LEX_PLUS || op == LEX_MINUS || op == LEX_TIMES || op == LEX_SLASH)
        r = LEX_INT;
    if (op == LEX_OR || op == LEX_AND)
        t = LEX_BOOL;
    if (t1 == t2 && t1 == t)
        st_lex.push(r);
    else
        throw "wrong types are in operation";
    poliz.push_back(Lex(op));
}

void Parser::check_not() {
    // Function to check if 'not' operation is applied to a boolean
    if (st_lex.top() != LEX_BOOL)
        throw "wrong type is in not";
    else
        poliz.push_back(Lex(LEX_NOT));
}

void Parser::eq_type() {
    // Function to check if types match in assignment
    type_of_lex t;
    from_st(st_lex, t);
    if (t != st_lex.top())
        throw "wrong types are in :=";
    st_lex.pop();
}

void Parser::eq_bool() {
    // Function to check if expression is boolean
    if (st_lex.top() != LEX_BOOL)
        throw "expression is not boolean";
    st_lex.pop();
}

void Parser::check_id_in_read() {
    // Function to check if an identifier is declared before reading
    if (!TID[c_val].get_declare())
        throw "not declared";
}


int main(int argc, char** argv) {
    try {
        // Initialize the scanner with the input file provided as a command-line argument
        Scanner scanner(argv[1]);
        Lex lex;
        type_of_lex type;
        int val;

        // Continue scanning and printing tokens until the end of file (LEX_FIN) is reached
        do {
            lex = scanner.get_lex();
            type = lex.get_type();
            val = lex.get_value();
            cout << lex << endl;
        } while(type != LEX_FIN);

        return 0; // Exit with success status
    }
    catch (char c) {
        cout << "Unexpected symbol " << c << endl;
        return 1; // Exit with error status
    }
    catch (Lex l) {
        cout << "Unexpected expression" << l << endl;
        return 1; // Exit with error status
    }
    catch (const char* c) {
        cout << c << endl;
        return 1; // Exit with error status
    }
}

