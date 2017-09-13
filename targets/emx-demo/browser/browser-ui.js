window.browserUI = {};

// Hook up the UART button + text field:
var sendUartButton = document.getElementById("uart-send");
var uartRxText = document.getElementById("uart-rx");
sendUartButton.onclick = function() {
  var msg = uartRxText.value;
  Module.ccall('js_runtime_call_on_uart', null, ['string'], [msg]);
  uartRxText.value = '';
};

// Hook up the GPIO callback:
window.browserUI.gpioSet = function(pin, on) {
  var gpioDiv = document.getElementById('gpio-' + pin);
  gpioDiv.className = on ? 'gpio-on' : 'gpio-off';
};
