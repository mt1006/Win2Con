# Win2Con
Displays windows and allows you to use them in console.

# Examples

Minecraft in cmd [```-ca -tm```]

![Minecraft](screenshots/mc.png "Minecraft")

Browser in cmd [```-tm```]

![Github](screenshots/github.png "Github")

# List of basic options
```
 [none] / -i         Handle to input window (in hexadecimal).
  [handle]           When used, it will skip window selection menu.
                     Examples:
                      win2con 000100F8
 -c [mode]           Sets color mode.
  (--colors)         Default color mode is "cstd-256".
                     To get list of all avaible color modes use "win2con -h color-modes"
                     Examples:
                      win2con -c winapi-16
 -s [w] [h]          Sets width and height of the drawn image.
  (--size)           By default size of entire window.
                     Using "-s 0 0" image size will be constant
                     (will not change with the console size change).
                     Examples:
                      win2con -s 120 50
 -sm [mode] [...]    Sets scaling mode.
  (--scaling-mode)   Default scaling mode is "fill" with enabled keeping ratio.
                     To get list of all avaible scaling modes use "win2con -h scaling-modes"
                     Examples:
                      win2con -sm no-scaling
 -ca (--client-area) Gets only window client area (without title bar or menu).
                     It may not work properly in some cases!
 -tm (--top-most)    Places console window above all other windows
                     and makes it transparent to input.
 -inf(--information) Information about Win2Con.
 -v  (--version)     Information about Win2Con version.
 -h <topic>          Displays help message.
  (--help)           Topics: basic, advanced, color-modes, scaling-modes, keyboard, full
  
[To see full help use "win2con -h full"]
 ```

# Keyboard control
```
[Right Alt + ...]:
 Q - Exit from window
 X - Exit from Win2Con
 C - Switch: entire window/client area
 T - Switch: console window above all other windows
```

# Compiling

## Windows
To compile it on Windows you need only Visual Studio. Just open .sln file and everything should work (at least theoretically).