api.gpio(0, true);
api.gpio(1, false);

api.onuart = function(msg) {
  alert(msg);
};
