# LabSolve
A command line tool to solve labyrinths alias mazes. It provides three solution algorithms and various settings for arguments.

![ezgif-1-e5960a83bf](https://user-images.githubusercontent.com/8998518/38201188-a1efb9fc-3697-11e8-8cd2-040b371f2d33.gif)

## Usage
```LabSolve [<filename>] [<algorithm>] [<args>]```

In your file you should use a raw text document with following defintions:
- **#** Wall
- **S** Startpoint
- **X** Goal

If you have issues with reading your own file, try to verify your text encoding as UTF-8.
You can always use the mazes from the ./labs folder in the repository.

## Algorithms
- **-escape** Just finds a way out via depth first search.
- **-greedy** Takes the closest looking way step by step, returns NOT the shortest path.
- **-bfs** Breadth First Search, Classical shortest path algorithm with lowest complexity.

## Arguments
- **-s** Silent searching without unnessecary output. Default is not silent.
- **-t** <nanoseconds> Delay in nanoseconds. Accepts integer values. Default is 0.
- **-l** Linear-only. Default is diagonally.
