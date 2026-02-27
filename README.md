# Vimktor

### What is it ?
Vimktor is simple Vim-like text editor written in c++. Vimktor provides fast, memory efficient and efective text editing enviorment \
This is semi-official windows fork.

## How to build ?
### Dependencies
- cmake (linux)
- curses (linux)
- [pcl and pac libraries](https://github.com/pawelelel/pcl) (windows)


### Linux
run these  commands:
```sh
cd Vimktor  
cmake -B build 
cmake --build build 
cd build 
./Vimktor [file you want to edit]
```


### Keymaps
"W" -> Jump to another word \
"$" -> Jump to end of line \
"0" -> Jump to first char \
"x" -> delete current char \
":" -> use command \
"dd" -> delete whole line

### Commands
"w" -> save file \
"q" -> quit

### Known bugs and issues
So basically, it compiles and runs, but nothing else more works.