Rock Paper Awesome
==================

# Quickstart

See the [Awesome Assembly Instructions](https://github.com/educoder/rock_paper_awesome/blob/master/docs/Awesome%20Assembly%20Instructions.pdf?raw=true) for a detailed setup and customization guide.

### The Arduino part:

Install the RPA Arduino library:

1. `cd {YOUR ARDUINO DIRECTORY}`
    * See http://arduino.cc/en/Guide/Libraries if you don't know where to find your Arduino directory.
2. `git clone git@github.com:educoder/rock_paper_arduino.git RockPaperAwesome`
3. Open the Arduino IDE
4. File ➜ Examples ➜ RockPaperAwesome ➜ awesome_leds
5. Customize the example sketch to your heart's desire
6. Compile and upload to your Arduino

### The node.js part:

1. `cd ~` (or wherever you want to clone the git repo)
2. `git clone git@github.com:educoder/rock_paper_nodejs.git`
3. `npm install` to install all necessary dependencies
4. Make sure you have a user account set up on your XMPP server.
5. Run `node awesome.js USERNAME PASSWORD XMPPHOST`,
   where `USERNAME` and `PASSWORD` is your XMPP username and password, and `XMPPHOST` is the hostname of your XMPP server.
   For example: `node awesome.js batman gotham badger.encorelab.org`

See [the Raspberry Pi wiki page](https://github.com/educoder/rock_paper_awesome/wiki/Running-on-a-Raspberry-Pi) for additional notes about running on a Raspberry Pi.

# License

##### GNU GPLv3

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

A copy of the GNU General Public License is available at 
<http://www.gnu.org/licenses/>.
