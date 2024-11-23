ORIGINAL SOURCE -
https://github.com/StrafeTool/waloranti

components needed:

  - 2 PCs
  - Arduino Leonardo R3 (preferably) or any board that has USB capabilities / can be converted via NicoHood project + USB Cable
  - USB Host Shield 
  - Jumper wires (Female to Male / vice versa)
  - CP2102/CH340 (USB to TTL converter)
  - Arduino Sketch (you need to find your own to use to send commands but for a relatively simple colorbot, all you need is some simple movement code)
  
  - (OPTIONAL) - 
  
  - Capture card / Fuser (you can use a network based solution such as Moonlight / Parsec / OBS etc if your speeds are consistent and good)

setup hardware:

  (1) Plug USB Host Shield into Arduino
  (2) Plug USB TO TTL converter chip into 2nd PC, take the jumper wires and connect them
      - RX TO TX
      - TX TO RX
      - 5v TO 5v
      - GND TO GND
  (3) Plug your USB Cable into the Arduino to the 1st PC
  (4) Plug your mouse into the USB Host Shield

setup software:

  troubleshooting:
  there may be some obstacles such as the usb to ttl hardware being not recognized, you may need to install drivers or there is maybe some issue with your arduino not receiving power check
  the voltage with a multimeter, etc (just google and look online)  
  
  - Execute the cheat on the 2nd PC (make sure the USB TO TTL chip is set to COM3 / Baudrate 115200)
  - Try moving your mouse to test if it works and then test if the clicks register whilst the program is active on the 2nd PC (so if you click once on the 1st PC, it should also do so on the 2nd)
  - On your 1st PC, make sure that display/capture is being processed and sent to the 2nd PC
  - Test (in this case) in Valorant now, default keybinds are found in config.hpp and can be modified via Microsoft Virtual Keycodes



