# Solitairecpp
A terminal solitaire game written in modern C++23 with superior error handling. Currently only linux is supported.

## Dependencies
```
cmake(build)
ftxui(fetchContent)
```
## Build
```sh
git clone https://github.com/0xsch1zo/solitairecpp
cd solitairecpp/
cmake . -B build
cd build
make -j
```
## The friendly manual
> [!TIP]
> The screen for choosing the difficulty is kind of broken for mouse, because of a small bug in the lib, please use keyboard instead(arrows or hjkl).

The interface should be straight forward there are butttons for most of actions. But there remain some that are keybinds.
<ul>
<li>Press Q or Ctrl+C anywhere to exit the current screen</li>
<li>Press ESC during a move operation to cancel it</li>
</ul>
Except for these keyboard navigation should be avoided as it is somewhat unstable because of some limitations.
