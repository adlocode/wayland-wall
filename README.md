Wayland Wall
=========================

Wayland Wall is a collection of protocols, called bricks, to create a complete desktop experience for Wayland.

The idea is copied from [wayland-protocols](https://cgit.freedesktop.org/wayland/wayland-protocols/) but for protocols less likely to make it to the major DEs.


Releases
--------

You can find releases tarballs [here](https://www.eventd.org/download/wayland-wall/).


Protocols
---------

We currently have this list of protocols:

*   notification-area
    This protocol will allow notification daemons to display their notifications using Wayland.


Compositor implementations
--------------------------

For now, these compositors are supported, with these protocols:

* Weston, in [weston-wall](https://github.com/wayland-wall/weston-wall)
    * notification-area
* Orbment, in [orbment-wall](https://github.com/wayland-wall/orbment-wall)
    * notification-area


Client implementations
----------------------

* notification-area:
    * [eventd](https://www.eventd.org/)
