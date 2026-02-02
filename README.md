## The idea:
To write a purposeful, cross-platform 2D graphics engine to go along with my first 2D game, islescape.
Every time I found myself writing platform-specific code that could very much be re-used in the future,
i integrated it into this very engine. It's less of an engine in the traditional sense, and more a repository
that grants access to a couple different rendering backends via `.a/.so/.lib/.dll` files.

## State:
- Software renderer - done.
- Text/Font loading done on the software side.
- Image loader as submodule / standalone - done.
- Currently in development alongside a simple game, because writing an engine with no goal is pointless.
- Alongside the game, there's also a map / tile editor in development over here: [river2D_mapedit](https://github.com/FlyMandi/river2D_mapedit/)

## Plans:

- Figuring out stretching/sizing behaviour, both win32 and linux
- sound
- openGL & vulkan renderers
