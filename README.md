# HGR-Tree

source code for paper [A Hierarchical Framework for Top-k Location-aware Error-tolerant Keyword Search](https://ieeexplore.ieee.org/abstract/document/8731340/)

## Usage

1. build
    ```bash
    make
    ```
2. run
    ```bash
    ./topk <dataset> <queryset> <k> <max_dist> <alpha> <delta>
    ```

**Note**: both dataset and queryset should in format:
```plain
lagtitude longitude description(comma seperated)
```
e.g.
```plain
42.509548 -83.371115 Jessie,J,ruined,Queen,song,huge,screen,Freddie,Mercury
```

