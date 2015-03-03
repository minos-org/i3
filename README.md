## About

This is an auto-generated mirror for upstream projects who either don't use git or include a debian/ directory in their master branch (removed by autobot). The former is quite problematic for launchpad autobuilds due to limitations in bzr dailydeb (LP#[617653](https://bugs.launchpad.net/bzr-builder/+bug/617653), LP #[1390746](https://bugs.launchpad.net/bzr-builder/+bug/1390746)).

This repository is used as a base to build the i3 version shipped with minos. In some cases additional patches are applied on top to generate the final result. See https://github.com/minos-org/i3-deb/tree/master/debian/patches (if applicable) to understand them.

Most of the time packages will follow upstream name conventions, in that case, the following instructions should help to install this program with minos sauce in your system.

## Quick start

### Only LTS versions

1. Set up the minos archive:

   ```
   $ sudo add-apt-repository ppa:minos-archive/main
   ```

2. Install:

   ```
   $ sudo apt-get update && sudo apt-get install i3
   ```

3. Enjoy ☺!

## Feedback

Please drop me an [email](mailto:j@minos.io) with your suggestions or open [an issue](https://github.com/minos-org/i3-deb/issues) with your comments.