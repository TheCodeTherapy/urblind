# URBlind

## A lightweight zooming tool for i3-wm on X11, built with Raylib.

URBlind is a simple tool designed for i3-wm on X11, allowing you to zoom into portions of your desktop using the mouse wheel and pan with click-and-drag. I use it with a bind key to easily inspect details on my screen.

I first saw a similar tool being used with i3 on X11 during a Twitch coding livestream by a guy named [Tsoding](https://www.twitch.tv/tsoding). I wasn't sure what software he was using, and I was too shy to ask, figuring that a hundred other people had probably already asked the very same question (which can be annoying). So, I decided to build my own.

<br />
<div align="center">
  <img src="https://github.com/user-attachments/assets/6ba7c70d-6371-4964-bcda-fc1dbc899f9d">
</div>

<br />

---

<br />

### 🏗️ **Building the Project**
Make sure to build the project first:

```sh
./build_posix.sh
```

Alternatively, while developing or changing the code, you may build and run with a single command like:

```sh
./build_posix.sh --run --debug 2 --debug-anchor tr
```

_(you'll find more details about the command line arguments below)_

<br />

### 🚀 **Running the Application**

```sh
./build/urblind [monitor_index] [OPTIONS]
```
- If no monitor index is provided, the **rightmost monitor** will be used by default.

<br />

### 🛠 **Options**
| Argument                      | Description |
|-------------------------------|-------------|
| `--help`                      | Show help message and exit. |
| `--debug`                     | Enable debug panel to display real-time info. |
| `--debug-anchor {tl\|tr\|bl\|br}` | Set debug panel anchor position. Options: `tl` (top-left, default), `tr` (top-right), `bl` (bottom-left), `br` (bottom-right). |

<br />

### 📌 **Usage Examples**

#### Start using the **default (rightmost) monitor**:
```sh
./build/urblind
```

#### Select **monitor index 2**:
```sh
./build/urblind 2
```

#### Check your **monitor indexes** to select the right one:
```sh
./build/urblind --help
```


#### Run on **monitor index 1** and Enable debug panel:
```sh
./build/urblind 1 --debug
```

#### Run on **monitor index 1** and Enable debug panel with **bottom-right anchor**:
```sh
./build/urblind 1 --debug --debug-anchor br
```

#### Run on **monitor index 3** with debug panel **top-right anchored**:
```sh
./build/urblind --debug-anchor tr --debug 3
```

<br />

---

<br />

### Q&A:

- _Can I use this on Gnome?_

  Yes, you can use it with Gnome. I listed this as an i3-wm tool because that's the environment I use it with, and I assumed desktop environments like Gnome already count with a similar tool out of the box.

- _Can I use this on KDE?_

  Probably, as long as you're using X11. I haven't tested, though.

- _Can I use this on Wayland?_

  No. I'm using `<X11/Xlib.h>` and `<X11/Xutil.h>` to capture the contents of the desktop. Also, I have no interest in Wayland as I don't appreciate giving up control over my system under the argument of security (the usual _"it's for your own safety"_ bullshit). It is a design decision in Wayland not to expose absolute window positions to clients nor to allow setting the absolute position of a window. I can't overstate enough how much I don't care for Wayland.

- _Can I suggest improvements or raise a PR?_

  Sure!

- _Can I raise a PR to make this compatible with Windows or MacOS?_

  Please feel free to fork and adapt it to your operating system. Raylib allows you to do that easily. However, for this repo, I prefer keeping it simple rather than overcomplicating it to make it compatible with other operating systems, so I won't merge any code to do that.

- _Can I use a different monospaced custom font for the Debug Panel?_

  Yes. Just convert any TrueType font you may want to use with `xxd`, add the generated header file to the include directory, and include your new font on `main.cpp`.

  To convert your font, use:

  ```bash
  xxd -i your_cool_font.ttf > ./include/your_cool_font.hpp
  ```

  Please keep in mind to inspect the file to find the names for the `unsigned char <name_here>_ttf[]` and the `unsigned int <name_here>_ttf_len` that you'll need to use to load the font with Raylib. See `main.cpp` for a usage reference.

<br />

---

<br />

### Enjoy my work?

Give it a Star and get in touch (and/or Tweet me memes) on X at **[@TheCodeTherapy](https://x.com/TheCodeTherapy)** 🤓
