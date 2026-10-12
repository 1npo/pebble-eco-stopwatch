var Clay = require('@rebble/clay');
var clayConfig = require('./config');
var clay = new Clay(clayConfig);

// The watch sends Halted (the safeguard in minutes) when it stops a timer.
Pebble.addEventListener('appmessage', function(e) {
  var min = e.payload.Halted;
  if (!min) return;
  var limit = min % 60 === 0 ? (min / 60) + ' h' : min + ' min';
  Pebble.showSimpleNotificationOnPebble('Eco Stopwatch',
      'Timer halted: it reached the ' + limit + ' safeguard.');
});
