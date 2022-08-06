/**
 * @file token.h
 * @brief A tokenizer that will split given string to tokens
 * @version 1.0
 * @date 2022-07-25
 * 
 * 
 */

#define MAX_NUM_TOKENS  100
#define tokenSeparators " \t\n"    // characters that separate tokens

/**
 * @brief  breaks up an array of chars by whitespace characters into individual tokens.
 * 
 * @param line a string that will be split into tokens
 * @param token array that will house the tokens
 * @return int  >=0: largest index of token array
 *              -1: on failure
 */
int tokenise (char line[], char *token[]);

