INSTALLING
----------

Under Linux, the
  ~/.blender/scripts
should get honored, but under OSX, these are located in the Blender.app
bundle:
  <BlenderDir>/blender.app/Contents/MacOS/2.54/scripts
Since installing a new version of Blender will wipe that directory, it is
recommended to use symlinks, which are easier to recreate.

I personally use:
  ~/.blender/addons_contrib
which I link under the bundle's scripts subdirectory:
  > ln -s ${HOME}/.blender/addons_contrib  /Application/Blender.app/Contents/MacOS/*/scripts
(adjust according to platform) and that seems to work fine.

Once the add-on is installed in a place looked by Blender, you then just need
to launch Blender, and activate it in the preferences (don't forget to save
defaults if you want to keep it every time).

I guess using Blender's "Install Add-On..." (from the Preferences panel) might
work; I just never tried it (and it probably copies in the .app bundle).


References:

http://en.wikibooks.org/wiki/Blender_3D:_Noob_to_Pro/Advanced_Tutorials/Python_Scripting/Export_scripts