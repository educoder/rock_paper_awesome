/*jshint browser: true, devel: true, strict: false, unused:false */
/*globals jQuery, _, Sail, RPA */

window.RPA = window.RPA || {};

RPA.Scoreboard = function () {
    var app = this;

    app.name = "RPA.Scoreboard";

    app.player1weapon = null;
    app.player2weapon = null;

    app.requiredConfig = {
        xmpp: {
            domain: 'string',
            port: 'number',
            url: 'string',
            username: 'string',
            password: 'string'
        }
    };

    app.init = function () {
        Sail.verifyConfig(app.config, app.requiredConfig);

        app.username = app.config.xmpp.username;
        app.password = app.config.xmpp.password;
        app.run = { name: "rock-paper-awesome" };

        Sail.modules
            // Enable multi-picker login for CommonKnowledge curnit - asking for run (must be linked to curnit)
            .load('Strophe.AutoConnector', {mode: 'pseudo-anon'})
            .load('AuthStatusWidget', {indicatorContainer: '#logout-container'})
            .thenRun(function () {
                Sail.autobindEvents(app);
                app.trigger('initialized');

                return true;
            });  
    };

    app.authenticate = function () {
        app.trigger('authenticated');
    };

    app.events = {
        initialized: function(ev) {
            app.authenticate();
        },

        'ui.initialized': function (ev) {
            console.log('ui.initialized!');
        },    

        authenticated: function (ev) {
            console.log('Authenticated...');
        },

        connected: function (ev) {
            console.log("Connected...");
        },

        'unauthenticated': function (ev) {
            app.authenticate();
        },

        sail: {
            ready: function (sev) {
                var p1 = $('#player-1');
                var p2 = $('#player-2');

                if (p1.find('.username').text() === "") {
                    p1.find('.username').text(sev.origin);
                    p1.addClass(MD5.hexdigest(sev.origin));
                } else if (p2.find('.username').text() === "") {
                    p2.find('.username').text(sev.origin);
                    p2.addClass(MD5.hexdigest(sev.origin));
                }
            },

            choice: function (sev) {
                var playerHex = MD5.hexdigest(sev.origin);
                
                var weapon = sev.payload.weapon;

                var playerDiv = $('.'+playerHex);

                playerDiv.find('.weapon').text(weapon);

                var score = playerDiv.find('.score').text();

                if (score === "")
                    score = 0;

                playerDiv.find('.score').text(score + 1);
            }
        }
    };

};

RPA.Scoreboard.prototype = new Sail.App();