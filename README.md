# mtp
An assignment for [CS344 - Operating Systems] to get familiar with the use of threads, mutual exclusion, and condition variables.

## Description
A program that creates (4x) threads to process input from standard input.
1. **Input Thread:** Reads in lines of characters from the standard input.
2. **Line Separator Thread:** Replaces every line separator '\n' in the input with a space.
3. **Plus Sign Thread:** Replaces every pair of plus signs '++' with a '^'.
4. **Output Thread:** Writes the processed data to standard output as lines of exactly 80 characters.

These threads will communicate with each other using the Producer-Consumer approach.

### Input
- A "line of input" is defined as a sequence of the allowed characters that does not include a line separator, followed by a line separator.
- Allowed characters are ASCII characters from space (decimal 32) to tilde (decimal 126), also known as printable characters.
- Stop-processing line: The program must process input lines until it receives an input line that contains **only** the characters "STOP" followed immediately by the line separator. Any additional lines beyond this stop-processing line must not be processed.
- Input will not contain any empty lines (i.e. lines with only space characters or no characters except the line separator).
- Input line will never be longer than 1000 characters including the line separator.
- Input for the program will never have more than 49 lines before the stop-processing line.
- The program does not need to check the input for correctness.

### Output
- The "80 character line" to be written to standard output is defined as 80 non-line separator characters, plus a line separator.
- The program must **not** wait to produce the output only when the stop-processing line is received. Whenever there is sufficient data for an output line, the output line must be produced.
- After the program receives the stop-processing line and before it terminates, the program must produce all 80 character lines it can still produce based on the input lines which were received *before* the stop-processing line, and have not yet been processed.
- Program must only output lines with 80 characters with a line separator after.

### Multi-Threading Requirements
![image](https://github.com/wleejess/mtp/assets/29618012/33f297a9-4989-4494-94f1-f71bb4842774)

