gcc main.c -o main
./main < image.mem > output.mem
python3 main.py view_color output.mem
