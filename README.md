Wayland Wall
=========================

Wayland Wall is a collection of protocols, called bricks, to create a complete desktop experience for Wayland.

This project will try to provide a compositor implementation for plugin-based compositors.


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

* Weston
    * notification-area
* Orbment
    * notification-area


Client implementations
----------------------

* notification-area:
    * [eventd](https://www.eventd.org/)
