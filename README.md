Rock Paper Awesome
==================

# Quickstart

See the [Awesome Assembly Instructions](https://github.com/educoder/rock_paper_awesome/blob/master/docs/Awesome%20Assembly%20Instructions.pdf?raw=true) for a detailed setup and customization guide.

### The Arduino part:

1. `cd {YOUR ARDUINO DIRECTORY}`
    * See http://arduino.cc/en/Guide/Libraries if you don't know where to find your Arduino directory.
2. `git clone git@github.com:educoder/rock_paper_arduino.git RockPaperAwesome`
3. Open the Arduino IDE
4. File ➜ Examples ➜ RockPaperAwesome ➜ awesome_leds
5. Customize the exmaple sketch to your heart's desire
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
This software is available under the [MIT licence](http://www.opensource.org/licenses/mit-license.php)

**All parts are copyright (c) 2012 of their respective authors**

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
