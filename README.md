# Rarch Installer
This is a completely custom [GTK4](https://www.gtk.org/) Rarch Linux Installer. 
> Riley's (customized) Arch Linux

## Details
- Language: C18
- Toolkit: GTK4 [4.18.6]
- License: [MPL-2](https://github.com/RileyMeta/Rarch_Installer/blob/main/LICENSE)

## Build Instructions
Currently we provide a custom Makefile that should automate every step of the build and compilation. Assuming you've already cloned the repository, in the root, to build the project you can run:
```sh
make
```
You may also use `make clean` to clear the final and object file(s).

<details>
<summary>Fallback Commands</summary>
These are the commands used in the make file:

```sh
gcc `pkg-config --cflags gtk4 gio-2.0` main.c `pkg-config --libs gtk4 gio-2.0` -o Rarch_Installer
```
Key: compiler cflags src ldflags output

The backticks are required for bash command interpretation, but can be replaced with `$(commands)`.
</details>

## Updates
> [!NOTE]
If anything needs to be fixed / updated, please submit a pull request
