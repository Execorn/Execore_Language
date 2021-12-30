//
// Created by Legion on 12.12.2021.
//
#ifdef EXECORE_AST_H
#define NODE_INFO                                                                                               \
    node_t type;                                                                                                \
                                                                                                                \
    struct source {                                                                                             \
        library* lib;                                                                                           \
        size_t current_line; size_t char_index;                                                                 \
    } source;                                                                                                   \
                                                                                                                \
    struct method {                                                                                             \
        bool is_valid;                                                                                          \
        char* name;                                                                                             \
        Array* args;                                                                                            \
    } method


#define CONTAINERS_UNION                                                                                \
    union {                                                                                             \
        struct {                                                                                        \
            Array* statements;                                                                          \
        } block;                                                                                        \
                                                                                                        \
        struct {                                                                                        \
            variable_t type;                                                                            \
            char* value;                                                                                \
        } literal;                                                                                      \
                                                                                                        \
        struct {                                                                                        \
            unary_t operator;                                                                           \
            struct node* operand;                                                                       \
        } unary;                                                                                        \
                                                                                                        \
        struct {                                                                                        \
            binary_t operator;                                                                          \
            struct node* left;                                                                          \
            struct node* right;                                                                         \
        } binary;                                                                                       \
                                                                                                        \
        struct {                                                                                        \
            Array* expressions;                                                                         \
        } commaExpression;                                                                                   \
                                                                                                        \
        struct {                                                                                        \
            Array* arguments;                                                                           \
        } args;                                                                                      \
                                                                                                        \
        struct {                                                                                        \
            struct node* seq;                                                                      \
            struct node* index;                                                                         \
        } index;                                                                                        \
                                                                                                        \
        struct {                                                                                        \
            struct node* seq;                                                                      \
            struct node* start;                                                                         \
            struct node* end;                                                                           \
        } slice;                                                                                        \
                                                                                                        \
        struct {                                                                                        \
            assign_t operator;                                                                          \
            struct node* variable;                                                                      \
            struct node* expression;                                                                    \
        } assignment;                                                                                   \
                                                                                                        \
        struct {                                                                                        \
            char* name;                                                                                 \
        } reference;                                                                                    \
                                                                                                        \
        struct {                                                                                        \
            char* name;                                                                                 \
            Array* arguments;                                                                           \
            bool is_builtin;                                                                               \
            bool is_checked;                                                                               \
        } funcCall;                                                                                \
                                                                                                        \
        struct {                                                                                        \
            struct node* expression;                                                                    \
        } exprStatement;                                                                             \
                                                                                                        \
        struct {                                                                                        \
            char* name;                                                                                 \
            bool is_nested;                                                                                \
            Array* arguments;                                                                           \
            struct node* block;                                                                         \
        } functionDecl;                                                                         \
                                                                                                        \
        struct {\
            Array* defvars;\
        } VarDecl;\
        \
        struct {\
            variable_t type;\
            char* name;\
            struct node* value;\
        } newVariable;\
        \
        struct {\
            struct node* condition;\
            struct node* if_true;\
            struct node* if_false;                                                                   \
        } ifStatement;\
        \
        struct {\
            struct node* condition;\
            struct node* block;\
        } loopStatement;\
        \
        struct {\
            char* name;\
            struct node* expression;\
            struct node* block;\
        } ForStatement;\
        \
        struct {\
            bool raw;\
            Array* expressions;\
        } PrintStatement;\
        \
        struct {\
            struct node* value;\
        } ReturnStatement;\
        \
        struct {\
            char* name;\
            struct node* code;\
        } importStatement;\
        \
        struct {\
            Array* prompts;\
            Array* identifiers;\
        } inputStatement;\
    };


#define NODE_FUNCTIONS                                                                                  \
    void (*valid_check)(struct node*);                                                                  \
    void (*dump       )(struct node*, int);                                                             \
    void (*compile    )(struct node*, Stack*)
#endif

#ifdef EXECORE_STACK_H
#define STACK_INFO                                                                                      \
    long top;                                                                                           \
    long capacity;                                                                                      \
    void** array
#endif

#ifdef EXECORE_READER_H
#define READER_INFO \
    library* source;		/* current_library from which this main_reader is reading */\
\
    token_t token;\
    token_t last_peek;\
    bool is_indent;\
    char string[BUFFER_SIZE]

#define INDENTATION_INFO \
    int total_indent; \
    int indentation[TOTAL_IND_MAX]

#define TOKEN_FUNC \
    token_t (*GetToken)();\
    token_t (*PeekToken)()


#define READERS_FUNC \
    void (*MakeReader  )(struct reader*, library*);\
    void (*CopyReader  )(struct reader*);\
    void (*UploadReader)(struct reader*)
#endif

#ifdef EXECORE_LIBRARY_H
#define LIB_INFO                                                                                    \
    struct lib*    next_lib;                                                                        \
    char*              name;                                                                        \
    char*            buffer;                                                                        \
    size_t            bytes;                                                                        \
    size_t   buf_char_index;                                                                        \
    size_t   buf_line_index;                                                                        \
    size_t  cur_line_number


#define CHAR_FUNC                                                                                 \
    int (*GetChar )(struct lib*);                                                                   \
    int (*PeekChar)(struct lib*);                                                                   \
    int (*PushChar)(struct lib*, char)

#define LIB_FUNC                                                                                    \
    struct lib *(*GetLibrary)(const char* name);                                                        \
    struct lib *(*FindLibrary)(const char* name)

#endif

#ifdef EXECORE_ID_H
#define ID_INFO \
    identifier_t type;\
	char* name;\
	struct identifier* next_id

#define OBJ_AND_NODE \
    Target* target_id;\
	struct node* node

#define ID_FUNC \
    struct identifier* (*add)(const identifier_t type, const char* name);\
	struct identifier* (*search)(const char* name);\
	void (*connect)(struct identifier* self, void* ptr);\
	void (*disconnect)(struct identifier* self)

#define SCOPE_INFO \
    struct scope* higher_scope;\
	Identifier* first_id;\
	bool is_nested


#define SCOPE_FUNC \
    void (*add_child)(bool is_nested);\
	void (*remove_child)()
#endif