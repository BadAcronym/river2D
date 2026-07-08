## The idea:
To write a purposeful, cross-platform 2D graphics engine to go along with my first 2D game, islescape.
Every time I found myself writing platform-specific code that could very much be re-used in the future,
i integrated it into this very engine. It's less of an engine in the traditional sense, and more a repository
that grants access to a couple different rendering backends via `.a/.so/.lib/.dll` files.

preview missing

It's not just a game engine, since I've also integrated functions to help build a UI and manage a simple
application, such as the map editor that's linked below.

## State:
- Software renderer - done. Including text loading and free stretch resizing.
- Image loader as submodule / standalone - done.
- Currently in development alongside a simple game, because writing an engine with no point of reference would be delusional.
- Alongside the game, there's also a map / tile editor in development over here: [river2D_mapedit](https://github.com/BadAcronym/river2D_mapedit/)

## Future Plans:

- sound
- openGL, vulkan & directX renderers, in that order
