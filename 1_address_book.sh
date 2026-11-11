#!/bin/bash
# Assignment 1: Address Book (CO1)
# Options: a/1: Create, b/2: View, c/3: Insert, d/4: Delete, e/5: Modify, f/6: Exit

file="address.txt"

while true
do
    echo -e "\na)Create  b)View  c)Insert  d)Delete  e)Modify  f)Exit"
    read -p "Enter choice: " ch

    case $ch in
    a|1) > $file
         echo "Address book created." ;;

    b|2) if [ -s "$file" ]; then
             echo -e "\nName\tPhone\tCity"
             cat $file
         else
             echo "Address book is empty."
         fi ;;

    c|3) read -p "Name: " name
         read -p "Phone: " phone
         read -p "City: " city
         echo "$name $phone $city" >> $file
         echo "Record inserted." ;;

    d|4) read -p "Enter name to delete: " name
         if grep -q "^$name " $file 2>/dev/null; then
             grep -v "^$name " $file > temp.txt && mv temp.txt $file
             echo "Record deleted."
         else
             echo "Record not found."
         fi ;;

    e|5) read -p "Enter name to modify: " name
         if grep -q "^$name " $file 2>/dev/null; then
             grep -v "^$name " $file > temp.txt && mv temp.txt $file
             read -p "New Phone: " phone
             read -p "New City: " city
             echo "$name $phone $city" >> $file
             echo "Record modified."
         else
             echo "Record not found."
         fi ;;

    f|6) echo "Exiting."
         exit 0 ;;

    *)   echo "Invalid choice." ;;
    esac
done


# chmod +x 1_address_book.sh
# ./1_address_book.sh


############################################################
# THEORY - ASSIGNMENT 1
############################################################
# B.1 What is a shell?
# --------------------
# - The shell is a command interpreter: a program that reads commands you type (or that are written in a script) and asks the kernel to run them.
# - bash (Bourne Again SHell) is the most common Linux shell.
# - A shell script is a plain text file containing shell commands. It lets you automate tasks without compiling anything.
# - The first line #!/bin/bash is the shebang: it tells the kernel which interpreter to use when you run ./1_address_book.sh.
#
# B.2 Variables in bash
# ---------------------
# - Created with name=value — no spaces around =.
# - Read with $name. Always write "$name" in double quotes if the value may contain spaces.
# - All variables are strings by default.
#
# B.3 Input and Output
# --------------------
# | Command | Meaning |
# |---|---|
# | echo "text" | Print text followed by a newline |
# | echo -e "a\tb\n" | -e enables escape sequences like \t (tab) and \n (newline) |
# | read var | Read one line from keyboard into var |
# | read -p "Prompt: " var | Print a prompt first, then read |
#
# B.4 I/O Redirection
# -------------------
# | Operator | Meaning |
# |---|---|
# | > file | Send output to file, truncating (emptying) it first. Creates it if missing |
# | >> file | Send output to file, appending to the end |
# | 2>/dev/null | Send error messages (fd 2 = stderr) to /dev/null, the "black hole" file that discards everything |
# | cmd1 && cmd2 | Run cmd2 only if cmd1 succeeded (exit status 0) |
#
# Note: A command with *only* a redirection and no command, like > address.txt, simply creates/empties the file.
#
# B.5 Exit status — how if works in bash
# --------------------------------------
# - Every command returns an exit status: 0 = success/true, non-zero = failure/false.
# - if cmd; then … fi runs the then part if cmd returned 0.
# - That's why we can write if grep -q …; then — grep returns 0 when it finds a match.
#
# B.6 Test conditions [ … ]
# -------------------------
# [ is actually a command (also called test). Common file tests:
#
# | Test | True when |
# |---|---|
# | [ -e file ] | file exists |
# | [ -f file ] | file exists and is a regular file |
# | [ -s file ] | file exists and size > 0 (not empty) |
# | [ -r file ] / -w / -x | readable / writable / executable |
#
# B.7 grep — searching text with patterns
# ---------------------------------------
# grep PATTERN file prints the lines of file that match PATTERN (a regular expression).
#
# | Option / Symbol | Meaning |
# |---|---|
# | -q | *quiet*: print nothing, only set exit status (0 = found) |
# | -v | *invert*: print lines that do NOT match |
# | ^ | regex anchor: "start of line" |
# | $ (in regex) | regex anchor: "end of line" |
#
# B.8 Loop and menu constructs
# ----------------------------
# while true          # true is a command that always returns 0 → infinite loop
# do
#     ...
# done
#
# case $ch in
#     a|1) commands ;;   # pattern "a" OR "1"; ;; ends this branch
#     *)   commands ;;   # * matches anything → default branch
# esac
############################################################


############################################################
# FUNCTIONS & SYSTEM CALLS - ASSIGNMENT 1
############################################################
# | Command | Purpose in this script |
# |---|---|
# | echo / echo -e | Print menu, messages, table header |
# | read -p | Read menu choice and record fields |
# | > | Create / empty address.txt |
# | >> | Append a record |
# | [ -s file ] | Check whether there is anything to show |
# | cat | Print the whole file |
# | grep -q | Check whether a name exists |
# | grep -v | Copy every line *except* the matching one |
# | mv | Rename temp.txt → address.txt (replace the old file) |
# | exit 0 | Terminate the script with success status |
############################################################


############################################################
# EXECUTION FLOW (CALL CHAIN) - ASSIGNMENT 1
############################################################
# START (bash reads the file top to bottom)
#   │
#   ├─ line 5: file="address.txt"
#   │
#   └─ line 7: while true ─────────────────────────────────────────────┐
#         │                                                            │
#         ├─ line 9 : echo menu                                        │
#         ├─ line 10: read ch                                          │
#         └─ line 12: case $ch in                                      │
#               │                                                      │
#               ├─ a|1 → [> file]            → "created"         ──────┤
#               ├─ b|2 → [-s file]?                                    │
#               │          ├─ yes → echo header → cat file       ──────┤
#               │          └─ no  → "empty"                      ──────┤
#               ├─ c|3 → read name,phone,city → echo >> file     ──────┤
#               ├─ d|4 → read name → grep -q?                          │
#               │          ├─ found → grep -v > temp → mv        ──────┤
#               │          └─ not   → "not found"                ──────┤
#               ├─ e|5 → read name → grep -q?                          │
#               │          ├─ found → grep -v > temp → mv              │
#               │          │          → read phone,city → echo >> ─────┤
#               │          └─ not   → "not found"                ──────┤
#               ├─ f|6 → "Exiting." → exit 0 ──► END                   │
#               └─ *   → "Invalid choice."                       ──────┘
#                                                 (back to top of loop)
#
# Delete / Modify trick — the data flow:
# address.txt ──grep -v "^John "──► temp.txt ──mv──► address.txt
# (all lines)    (drop John's line)  (everyone       (old file replaced)
#                                     except John)
############################################################


############################################################
# DRY RUN - ASSIGNMENT 1
############################################################
# | Step | Input | address.txt after the step | Screen |
# |---|---|---|---|
# | 1 | a | *(empty file)* | Address book created. |
# | 2 | b | *(empty)* | Address book is empty. → -s is false |
# | 3 | c, John, 111, Pune | John 111 Pune | Record inserted. |
# | 4 | c, Mary, 222, Mumbai | John 111 Pune<br>Mary 222 Mumbai | Record inserted. |
# | 5 | e, John, 999, Delhi | Mary 222 Mumbai<br>John 999 Delhi | Record modified. (John moved to the end) |
# | 6 | d, Mary | John 999 Delhi | Record deleted. |
# | 7 | d, Alex | John 999 Delhi | Record not found. |
# | 8 | x | unchanged | Invalid choice. |
# | 9 | f | unchanged | Exiting. → script ends |
#
# Detail of step 6 (grep view):
# grep -q "^Mary " address.txt   → finds "Mary 222 Mumbai" → exit status 0 → then-branch
# grep -v "^Mary " address.txt   → outputs "John 999 Delhi"  → written to temp.txt
# mv temp.txt address.txt         → address.txt now has only John
############################################################
