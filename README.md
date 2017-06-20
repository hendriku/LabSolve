# LabSolve
A program to solve labyrinths. It provides three solution algorithms and various settings for arguments.

## Usage
```LabSolve [<filename>] [<algorithm>] [<args>]```

## Algorithms
- **-escape** Just finds a way out via depth first search.
- **-greedy** Takes the closest looking way step by step, returns NOT the shortest path
- **-bfs** Breadth First Search, Classical shortest path algorithm with lowest complexity

## Arguments
- **-s** Silent searching without unnessecary output. Default is not silent.
- **-t** <nanoseconds> Delay in nanoseconds. Accepts integer values. Default is 0.
- **-l** Linear-only. Default is diagonally.
