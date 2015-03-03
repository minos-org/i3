## About

i3 includes a debian/ dir which difficults ppa recipes (LP:1390746). This is an automatic generated mirror of i3 minus deb/. It's used as base to build the i3 version shipped with minos. See https://github.com/minos-org/i3-deb/tree/master/debian/patches to understand the patches applied on top to generate the final packages.

## Quick start

1. Set up the minos archive:

   ```
   $ sudo add-apt-repository ppa:minos-archive/main
   ```

2. Install:

   ```
   $ sudo apt-get update && sudo apt-get install i3-wm
   ```

3. Enjoy =)!

## Feedback

Please drop me an [email](mailto:j@minos.io) with your suggestions or open [an issue](https://github.com/minos-org/i3-deb/issues) with your comments.
