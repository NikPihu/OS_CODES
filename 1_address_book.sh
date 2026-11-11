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
