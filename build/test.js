var Module = require("./mandelbrot.js");
renderHunByHun = Module.cwrap('renderHunByHun');

Module.onRuntimeInitialized = () => {
  console.log(renderHunByHun())
}