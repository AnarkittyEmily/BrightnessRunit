# BrightnessRunit

## Config 

Make a file in ```/etc/brightness_runit/brightness_runit.conf```

Add your: 

Driver, you can find this in your ```/sys/class/backlight/``` folder 
save_on_shutdown, true or 1 if you want to have your last brightness used saved onto a file and then restored on next boot, if you don't want this, just don't add it. 
Brightness, add the brightness level you want it to set at boot, not required if you have save_on_shutdown=true


## Adding the Service

```Artix Linux
ln -s scripts /run/runit/services```

```Void Linux
ln -s scripts /etc/runit/runsvdir/default/```
