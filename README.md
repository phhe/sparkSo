sparkSo
==========

Control Sonos with a Spark (spark.io)

This is a little projekt to get my Spark control a Sonos speaker in the local network.
There is no real error handling so far and it is not too well tested, but seems to work ;)


### The Project

The idea is to control sonos with switches mounted on the wall like light switches. A friend of mine is working on the hardware, below pictures of its current progress. As you can see this module includes some LEDs which can be used to indicate the mute status of a speaker or something somiliar.


[Picture 1](https://github.com/phhe/sparkSo/blob/master/img/IMG_5257.jpeg?raw=true)

[Picture 2](https://github.com/phhe/sparkSo/blob/master/img/IMG_7468.jpeg?raw=true)


## Progress

### What is possible at the moment ?

#### Get Values

- Get Volume (Values from 0 to 100)
- Get Mute - Status (1 or 0)


#### Set / Actions

- Set the Volume (Values from 0 to 100)
- Set Mute (0 or 1)


### Planned / work in progress

#### Near future

- Parse the Topology (returned by GET /status/topology)
- Find the group Master and then:
  - Play / Pause
  - Previous / Next
  - ...



#### Farer away

- Shuffle through the favorite Radio stations


## Usage

To debug the functionality the Application will open a serial console and listen for keystrokes.

Here what will/shall happen:

  * A: set the ip to Sonos A 
  * B: set the ip to Sonos B
  * M: Toggle Mute for the selected Sonos
  * U: Increase Volume for the selected Sonos
  * J: Decrease Volume for the selected Sonos


### Linux / Mac serial connection

The following little shell script keeps a screen ready as soon as /dev/tty.usbmodemXXXXX becomes available. 

    while true; do [ -e /dev/tty.usbmodemfa2121 ] && screen -h 500 /dev/tty.usbmodemfa2121 ; sleep 5; reset; echo "No tty available"; done
