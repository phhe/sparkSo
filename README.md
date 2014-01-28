sonosspark
==========

Control Sonos via Spark (spark.io)



This is a little projekt to get my Spark control a Sonos speaker in the local network.


There is no real error handling so far and it is not too well tested, but seems to work ;)




## Progress

### What is possible at the moment ?

#### Get Values

- Get Volume (Values from 0 to 100)
- Get Mute - Status (1 or 0)


#### Set / Actions

- Set the Volume (Values from 0 to 100)


### Planned / work in progress

#### Near future

- Toggle Mute
- Parse the Topology (returned by GET /status/topology)
- Find the group Master and then:
  - Play / Pause
  - Previous / Next
  - ...



#### Farer away

- Shuffle through the favorite Radio stations
