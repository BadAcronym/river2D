## The idea:
To write a purposeful, simple 2D game engine to go along with my first 2D game, islescape.
Every time I found myself writing platform-specific code that could very much be re-used in the future,
i integrated it into this very engine. It's less of an engine in the traditional sense, and more a repository
that grants access to a couple different rendering backends via `.a/.so/.lib/.dll` files, as well as a binary
that serves as a map editor.

## State:
- WIP: the Linux (X11) software renderer.
- Currently in development alongside a simple game, because writing an engine with no goal is pointless.
- Once the project is in an MVP state, I will port the platform-specific code to windows.

## Plans:

- level editor
- sound
- dialogue system
- platform porting
- openGL & vulkan renderers
- maybe: image loader for software renderers (load BGRA from RGBA, etc)
