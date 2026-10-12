module.exports = [
  { "type": "heading", "defaultValue": "Eco Stopwatch" },
  { "type": "section", "items": [
    { "type": "heading", "defaultValue": "General" },
    { "type": "select", "messageKey": "RefreshInterval", "label": "Screen Refresh Interval",
      "defaultValue": "10",
      "description": "How often the running timer redraws. Longer intervals save battery. Presses always update the screen immediately.",
      "options": [
        { "label": "1 second",   "value": "1" },  { "label": "2 seconds",  "value": "2" },
        { "label": "5 seconds",  "value": "5" },  { "label": "10 seconds", "value": "10" },
        { "label": "15 seconds", "value": "15" }, { "label": "20 seconds", "value": "20" },
        { "label": "30 seconds", "value": "30" }, { "label": "60 seconds", "value": "60" } ] },
    { "type": "select", "messageKey": "Theme", "label": "Color Theme", "defaultValue": "2",
      "capabilities": ["COLOR"],
      "options": [ { "label": "White on Black", "value": "0" },
                   { "label": "Black on White", "value": "1" },
                   { "label": "Color", "value": "2" } ] },
    { "type": "select", "messageKey": "Theme", "label": "Color Theme", "defaultValue": "0",
      "capabilities": ["BW"],
      "options": [ { "label": "White on Black", "value": "0" },
                   { "label": "Black on White", "value": "1" } ] }
  ]},
  { "type": "submit", "defaultValue": "Save Settings" }
];
