#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#define MAX_IDENTIFIER_SIZE 10
#define MAX_INT_SIZE 8
#define MAX_STRING_SIZE 256

typedef enum {
    Identifier,
    IntConst,
    Operator,
    LeftCurlyBracket,
    RightCurlyBracket,
    String,
    Keyword,
    EndOfLine,
    Comma
} TokenType;

typedef struct {
    TokenType type;
    char value[MAX_STRING_SIZE];
} Token;

int is_keyword(const char *str) {
    const char *keywords[] = {"int", "text", "is", "loop", "times", "read", "write", "newLine"};
    const int num_keywords = sizeof(keywords) / sizeof(keywords[0]);
    for (int i = 0; i < num_keywords; ++i) {
        if (strcmp(str, keywords[i]) == 0) {
            return 1;
        }
    }
    return 0;
}

Token* tokenize_line(const char* line, int* token_count) {
    Token* tokens = malloc(MAX_STRING_SIZE * sizeof(Token));
    *token_count = 0;

    char buffer[MAX_STRING_SIZE];
    int buffer_index = 0;
    int in_comment = 0;

    for (int i = 0; line[i] != '\0'; ++i) {
        if (in_comment) {
            if (line[i] == '*' && line[i+1] == '/') {
                in_comment = 0;
                i++;
            }
            continue;
        }

        if (line[i] == '/' && line[i+1] == '*') {
            in_comment = 1;
            i++;
            continue;
        }

        
        if (in_comment) {
            continue;
        }

        
        
        if (isspace(line[i])) {
            continue;
        }

        if (line[i] == '.') {
            tokens[*token_count].type = EndOfLine;
            strcpy(tokens[*token_count].value, ".");
            (*token_count)++;
            break;
        }

        if (line[i] == '{') {
            tokens[*token_count].type = LeftCurlyBracket;
            strcpy(tokens[*token_count].value, "{");
            (*token_count)++;
            continue;
        }

        if (line[i] == '}') {
            tokens[*token_count].type = RightCurlyBracket;
            strcpy(tokens[*token_count].value, "}");
            (*token_count)++;
            continue;
        }

        if (line[i] == '+' || line[i] == '*' || line[i] == '/') {
            tokens[*token_count].type = Operator;
            tokens[*token_count].value[0] = line[i];
            tokens[*token_count].value[1] = '\0';
            (*token_count)++;
            continue;
        }

        if (line[i] == ',') {
            tokens[*token_count].type = Comma;
            strcpy(tokens[*token_count].value, ",");
            (*token_count)++;
            continue;
        }

        if (line[i] == '"') {
            buffer[buffer_index++] = '"';
            ++i;
            while (line[i] != '"' && line[i] != '\0') {
                buffer[buffer_index++] = line[i++];
            }
            if (line[i] == '\0') {
                printf("Error: String constant not terminated before end of line!\n");
                exit(EXIT_FAILURE);
            }
            buffer[buffer_index++] = '"';
            buffer[buffer_index] = '\0';
            tokens[*token_count].type = String;
            strcpy(tokens[*token_count].value, buffer);
            (*token_count)++;
            buffer_index = 0;
            continue;
        }

        if (isalpha(line[i])) {
            buffer[buffer_index++] = line[i];
            ++i;
            while (isalnum(line[i]) || line[i] == '_') {
                if (buffer_index < MAX_IDENTIFIER_SIZE) {
                    buffer[buffer_index++] = line[i++];
                } else {
                    printf("Error: Identifier size exceeds maximum limit!\n");
                    exit(EXIT_FAILURE);
                }
            }
            buffer[buffer_index] = '\0';
            if (is_keyword(buffer)) {
                tokens[*token_count].type = Keyword;
            } else {
                tokens[*token_count].type = Identifier;
            }
            strcpy(tokens[*token_count].value, buffer);
            (*token_count)++;
            buffer_index = 0;
            --i;
            continue;
        }

        if (isdigit(line[i]) || (line[i] == '-' && isdigit(line[i+1]))) {
            if (line[i] == '-' && *token_count > 0) {
                int prev_token_type = tokens[*token_count - 1].type;
                if (prev_token_type == Identifier || prev_token_type == IntConst) {
                    tokens[*token_count].type = Operator;
                    tokens[*token_count].value[0] = line[i];
                    tokens[*token_count].value[1] = '\0';
                    (*token_count)++;
                    continue;
                }
            }
            if (line[i] == '-' && isdigit(line[i+1])) {
                buffer[buffer_index++] = line[i];
                ++i;
                while (isdigit(line[i])) {
                    buffer[buffer_index++] = line[i++];
                }
                buffer[buffer_index] = '\0';
                strcpy(buffer, "0");
                tokens[*token_count].type = IntConst;
                strcpy(tokens[*token_count].value, buffer);
                (*token_count)++;
                buffer_index = 0;
                --i;
                continue;
            }
            int start = i;
            if (line[i] == '-') {
                buffer[buffer_index++] = line[i++];
            }
            int isNegative = line[start] == '-';
            while (isdigit(line[i])) {
                if (buffer_index < MAX_INT_SIZE) {
                    buffer[buffer_index++] = line[i++];
                } else {
                    printf("Error: Integer size exceeds maximum limit!\n");
                    exit(EXIT_FAILURE);
                }
            }
            buffer[buffer_index] = '\0';
            if (start == i) {
                
                strcpy(buffer, "0"); 
            } else if (isNegative) {
                strcpy(buffer, "0"); 
            }
            tokens[*token_count].type = IntConst;
            strcpy(tokens[*token_count].value, buffer);
            (*token_count)++;
            buffer_index = 0;
            --i;
            continue;
        }

        printf("Error: Invalid character encountered!\n");
        exit(EXIT_FAILURE);
    }

    
    if (in_comment) {
        printf("Error: Comment block not terminated before end of line!\n");
        exit(EXIT_FAILURE);
    }

   
    tokens = realloc(tokens, *token_count * sizeof(Token));
    return tokens;
}


int main() {
    FILE* input_file = fopen("code.sta", "r");
    FILE* output_file = fopen("code.lex", "w");

    if (input_file == NULL || output_file == NULL) {
        perror("Error opening files");
        return EXIT_FAILURE;
    }

    char line[MAX_STRING_SIZE];
    int line_number = 1;

    while (fgets(line, sizeof(line), input_file)) {
        int token_count;
        Token* tokens = tokenize_line(line, &token_count);

        for (int i = 0; i < token_count; ++i) {
            switch (tokens[i].type) {
                case Identifier:
                    fprintf(output_file, "Identifier(%s)\n", tokens[i].value);
                    break;
                case IntConst:
                    fprintf(output_file, "IntConst(%s)\n", tokens[i].value);
                    break;
                case Operator:
                    fprintf(output_file, "Operator(%s)\n", tokens[i].value);
                    break;
                case LeftCurlyBracket:
                    fprintf(output_file, "LeftCurlyBracket(%s)\n", tokens[i].value);
                    break;
                case RightCurlyBracket:
                    fprintf(output_file, "RightCurlyBracket(%s)\n", tokens[i].value);
                    break;
                case String:
                    fprintf(output_file, "String(%s)\n", tokens[i].value);
                    break;
                case Keyword:
                    fprintf(output_file, "Keyword(%s)\n", tokens[i].value);
                    break;
                case EndOfLine:
                    fprintf(output_file, "EndOfLine(%s)\n", tokens[i].value);
                    break;
                case Comma:
                    fprintf(output_file, "Comma(%s)\n", tokens[i].value);
                    break;
                default:
                    fprintf(output_file, "Unknown(%s)\n", tokens[i].value);
                    break;
            }
        }

        free(tokens); 

        ++line_number;
    }

    fclose(input_file);
    fclose(output_file);

    return EXIT_SUCCESS;
}