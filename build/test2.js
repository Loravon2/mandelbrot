WebAssembly.instantiateStreaming(fetch("mandelbrot.wasm")).then(
  (obj) => obj.instance.renderHunByHun()
)

// var WasmInstance = new WebAssembly.Instance(WasmModule);

// var offset = WasmInstance.exports.renderHunByHun();