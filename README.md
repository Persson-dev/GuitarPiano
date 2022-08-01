# GuitarPiano
GuitarPiano is a french software to learn guitar from piano. 
It uses : 
- [OpenGL](https://www.opengl.org) (to draw the instruments)
- [GLEW](http://http://glew.sourceforge.net) (to load opengl functions)
- [GLFW](https://www.glfw.org) (to help managing the window)
- [ImGui](https://github.com/ocornut/imgui) (for the gui)

English translations will be added if really needed. For now, this is just a small project (the code is still a mess).

Also, feel free to contribute to the project !

# Screenshots

![Capture d’écran de 2022-07-30 11-41-27](https://user-images.githubusercontent.com/66266021/181904833-8ee95007-10b0-4937-9701-48d8d676104e.png)
![Capture d’écran de 2022-07-30 11-41-33](https://user-images.githubusercontent.com/66266021/181904805-676fffb9-c00e-414f-bdd5-19fb9a0f0f52.png)

# Build
You will need a recent compiler supporting c++17 and [xmake](https://xmake.io/#/getting_started).
Just cd into the project folder and type :
```
xmake
```
xmake should download all the depencencies for you.

# Run
```
xmake run -w test
```

# Install
Currently, there is no install script so you should just copy the binary.

The binary should be located in the build folder.
