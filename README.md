# Compilation
```
gcc main.c -lncursesw -lm
```

# Usage
```
export TERM=xterm-direct
ffmpeg -i video_file "%04d.jpg"   # ffmpeg convert video to frames (.jpgs only for now)
./a.out DELAY_MS .                # right now it can only be used if on same directory
```

# Demo

![Demo of Terminal Gif Player](https://media.giphy.com/media/VwLItHfeydKJiThuWQ/giphy.gif)
