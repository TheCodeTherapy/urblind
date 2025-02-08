# URBlind

## A lightweight zooming tool for i3-wm on X11, built with Raylib.

URBlind is a simple tool designed for i3-wm on X11, allowing you to zoom into portions of your desktop using the mouse wheel and pan with click-and-drag. I use it with a bind key to easily inspect details on my screen.

I first saw a similar tool being used with i3 on X11 during a Twitch coding livestream by a guy named Tsoding. I wasn't sure what software he was using, and I was too shy to ask, figuring that a hundred other people had probably already asked the very same question (which can be annoying). So, I decided to build my own.

<br />
<div align="center">
  <img src="https://github.com/user-attachments/assets/6ba7c70d-6371-4964-bcda-fc1dbc899f9d">
</div>

<br />

### Q&A:

- _Can I use this on Gnome?_

  Yes, you can use it with Gnome. I listed this as an i3-wm tool because that's the environment I use it with, and I assumed desktop environments like Gnome already count with a similar tool out of the box.

- Can I use this on KDE?

  Probably, as long as you're using X11. I haven't tested, though.

- Can I use this on Wayland?

  No. I'm using `<X11/Xlib.h>` and `<X11/Xutil.h>` to capture the contents of the desktop. Also, I have no interest in Wayland as I don't appreciate giving up control over my system under the argument of security (the usual _"it's for your own safety"_ bullshit). It is a design decision in Wayland not to expose absolute window positions to clients nor to allow setting the absolute position of a window. I can't overstate enough how much I don't care for Wayland.

- Can I suggest improvements or raise a PR?

  Sure!

- Can I raise a PR to make this compatible with Windows?

  Please feel free to fork and adapt it to your operating system. Raylib allows you to do that easily. However, for this repo, I prefer keeping it simple rather than overcomplicating it to make it compatible with other operating systems, so I won't merge any code to do that.
