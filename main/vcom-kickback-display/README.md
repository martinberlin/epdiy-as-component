Same program for VCOM-Reading but with display
==============================================

Needs an edit in the file:
components/epdiy/src/board/epd_board_v7.c

Just remove the ERROR_CHECK wrappers from Line 123 to 125:

```C
    i2c_param_config(EPDIY_I2C_PORT, &conf);

    i2c_driver_install(EPDIY_I2C_PORT, I2C_MODE_MASTER, 0, 0, 0);
```

This is because I2C was already initialized by the OLED display.
So if those ERROR_CHECKs are there then it will simply restart with an error. Without them it will simply ignore it and keep on.

If you are lazy to edit this file, I also left a modified copy, just do in the root folder:

      cp patch/epd_board_v7.c components/epdiy/src/board

And voila! Then it will be fixed.
Probably there is a smarter way to do multiple I2C instantiation, but that will require to initialize it in a single place, and then pass the I2C dev struct over all the other components :P