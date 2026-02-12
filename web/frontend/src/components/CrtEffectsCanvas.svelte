<script lang="ts">
  interface Props {
    enabled?: boolean;
    scanlines?: boolean;
    scanlineSpeed?: number;
    noise?: boolean;
    noiseIntensity?: number;
    flicker?: boolean;
    flickerIntensity?: number;
  }

  let {
    enabled = true,
    scanlines = true,
    scanlineSpeed = 0.5,
    noise = true,
    noiseIntensity = 0.03,
    flicker = true,
    flickerIntensity = 0.02
  }: Props = $props();

  let canvas: HTMLCanvasElement;
  let gl: WebGLRenderingContext | null = null;
  let program: WebGLProgram | null = null;
  let animationId: number;
  let startTime = 0;

  // Uniform locations
  let uTime: WebGLUniformLocation | null = null;
  let uResolution: WebGLUniformLocation | null = null;
  let uScanlineY: WebGLUniformLocation | null = null;
  let uNoiseIntensity: WebGLUniformLocation | null = null;
  let uFlickerIntensity: WebGLUniformLocation | null = null;
  let uEnableScanlines: WebGLUniformLocation | null = null;
  let uEnableNoise: WebGLUniformLocation | null = null;
  let uEnableFlicker: WebGLUniformLocation | null = null;

  const vertexShaderSource = `
    attribute vec2 a_position;
    varying vec2 v_texCoord;

    void main() {
      gl_Position = vec4(a_position, 0.0, 1.0);
      v_texCoord = (a_position + 1.0) * 0.5;
    }
  `;

  const fragmentShaderSource = `
    precision mediump float;

    varying vec2 v_texCoord;

    uniform float u_time;
    uniform vec2 u_resolution;
    uniform float u_scanlineY;
    uniform float u_noiseIntensity;
    uniform float u_flickerIntensity;
    uniform bool u_enableScanlines;
    uniform bool u_enableNoise;
    uniform bool u_enableFlicker;

    // Pseudo-random function
    float random(vec2 st) {
      return fract(sin(dot(st.xy, vec2(12.9898, 78.233))) * 43758.5453123);
    }

    void main() {
      vec2 uv = v_texCoord;
      vec3 color = vec3(0.0);
      float alpha = 0.0;

      // CRT orange tint color
      vec3 crtColor = vec3(0.918, 0.412, 0.11); // #EA691C

      // Scanline effect - moving bright band (MUCH more visible now)
      if (u_enableScanlines) {
        float scanlinePos = u_scanlineY / u_resolution.y;
        float dist = abs(uv.y - scanlinePos);
        float scanlineGlow = smoothstep(0.2, 0.0, dist) * 0.5;
        color += crtColor * scanlineGlow;
        alpha = max(alpha, scanlineGlow);
      }

      // Noise effect (increased intensity)
      if (u_enableNoise) {
        float noiseVal = random(uv * u_resolution + u_time * 10.0);
        if (noiseVal > (1.0 - u_noiseIntensity * 2.0)) {
          float brightness = random(uv + u_time * 0.1) * 0.5;
          color += vec3(brightness, brightness * 0.8, brightness * 0.3);
          alpha = max(alpha, brightness);
        }
      }

      // Flicker effect
      if (u_enableFlicker) {
        float flickerRand = random(vec2(floor(u_time * 10.0), 0.0));
        if (flickerRand > 0.7) {
          float flickerAmount = u_flickerIntensity * 2.0 * random(vec2(u_time));
          color += crtColor * flickerAmount;
          alpha = max(alpha, flickerAmount);
        }

        // Horizontal glitch line
        float glitchRand = random(vec2(floor(u_time * 5.0), 1.0));
        if (glitchRand > 0.8) {
          float glitchY = random(vec2(floor(u_time * 3.0), 2.0));
          float glitchDist = abs(uv.y - glitchY);
          if (glitchDist < 0.003) {
            color += crtColor * 0.4;
            alpha = max(alpha, 0.4);
          }
        }
      }

      gl_FragColor = vec4(color, alpha);
    }
  `;

  function createShader(gl: WebGLRenderingContext, type: number, source: string): WebGLShader | null {
    const shader = gl.createShader(type);
    if (!shader) return null;

    gl.shaderSource(shader, source);
    gl.compileShader(shader);

    if (!gl.getShaderParameter(shader, gl.COMPILE_STATUS)) {
      console.error('Shader compile error:', gl.getShaderInfoLog(shader));
      gl.deleteShader(shader);
      return null;
    }

    return shader;
  }

  function createProgram(gl: WebGLRenderingContext, vertexShader: WebGLShader, fragmentShader: WebGLShader): WebGLProgram | null {
    const prog = gl.createProgram();
    if (!prog) return null;

    gl.attachShader(prog, vertexShader);
    gl.attachShader(prog, fragmentShader);
    gl.linkProgram(prog);

    if (!gl.getProgramParameter(prog, gl.LINK_STATUS)) {
      console.error('Program link error:', gl.getProgramInfoLog(prog));
      gl.deleteProgram(prog);
      return null;
    }

    return prog;
  }

  function initWebGL() {
    if (!canvas) return false;

    gl = canvas.getContext('webgl', { alpha: true, premultipliedAlpha: false });
    if (!gl) {
      console.warn('WebGL not supported, falling back');
      return false;
    }

    const vertexShader = createShader(gl, gl.VERTEX_SHADER, vertexShaderSource);
    const fragmentShader = createShader(gl, gl.FRAGMENT_SHADER, fragmentShaderSource);

    if (!vertexShader || !fragmentShader) return false;

    program = createProgram(gl, vertexShader, fragmentShader);
    if (!program) return false;

    // Create fullscreen quad
    const positionBuffer = gl.createBuffer();
    gl.bindBuffer(gl.ARRAY_BUFFER, positionBuffer);
    gl.bufferData(gl.ARRAY_BUFFER, new Float32Array([
      -1, -1,
       1, -1,
      -1,  1,
      -1,  1,
       1, -1,
       1,  1,
    ]), gl.STATIC_DRAW);

    const positionLocation = gl.getAttribLocation(program, 'a_position');
    gl.enableVertexAttribArray(positionLocation);
    gl.vertexAttribPointer(positionLocation, 2, gl.FLOAT, false, 0, 0);

    // Get uniform locations
    uTime = gl.getUniformLocation(program, 'u_time');
    uResolution = gl.getUniformLocation(program, 'u_resolution');
    uScanlineY = gl.getUniformLocation(program, 'u_scanlineY');
    uNoiseIntensity = gl.getUniformLocation(program, 'u_noiseIntensity');
    uFlickerIntensity = gl.getUniformLocation(program, 'u_flickerIntensity');
    uEnableScanlines = gl.getUniformLocation(program, 'u_enableScanlines');
    uEnableNoise = gl.getUniformLocation(program, 'u_enableNoise');
    uEnableFlicker = gl.getUniformLocation(program, 'u_enableFlicker');

    // Enable blending for transparency
    gl.enable(gl.BLEND);
    gl.blendFunc(gl.SRC_ALPHA, gl.ONE_MINUS_SRC_ALPHA);

    return true;
  }

  function resize() {
    if (!canvas || !gl) return;
    const dpr = window.devicePixelRatio || 1;
    canvas.width = window.innerWidth * dpr;
    canvas.height = window.innerHeight * dpr;
    gl.viewport(0, 0, canvas.width, canvas.height);
  }

  let scanlineY = 0;

  function render(timestamp: number) {
    if (!gl || !program || !enabled) {
      animationId = requestAnimationFrame(render);
      return;
    }

    if (document.hidden) {
      animationId = requestAnimationFrame(render);
      return;
    }

    const time = (timestamp - startTime) * 0.001;

    // Update scanline position
    scanlineY = (scanlineY + scanlineSpeed) % canvas.height;

    gl.clear(gl.COLOR_BUFFER_BIT);
    gl.useProgram(program);

    // Set uniforms
    gl.uniform1f(uTime, time);
    gl.uniform2f(uResolution, canvas.width, canvas.height);
    gl.uniform1f(uScanlineY, scanlineY);
    gl.uniform1f(uNoiseIntensity, noiseIntensity);
    gl.uniform1f(uFlickerIntensity, flickerIntensity);
    gl.uniform1i(uEnableScanlines, scanlines ? 1 : 0);
    gl.uniform1i(uEnableNoise, noise ? 1 : 0);
    gl.uniform1i(uEnableFlicker, flicker ? 1 : 0);

    // Draw fullscreen quad
    gl.drawArrays(gl.TRIANGLES, 0, 6);

    animationId = requestAnimationFrame(render);
  }

  $effect(() => {
    if (!canvas) return;

    if (!initWebGL()) {
      console.error('CrtEffectsCanvas: Failed to initialize WebGL');
      return;
    }

    resize();
    startTime = performance.now();

    window.addEventListener('resize', resize);
    animationId = requestAnimationFrame(render);

    return () => {
      window.removeEventListener('resize', resize);
      cancelAnimationFrame(animationId);
    };
  });
</script>

<canvas
  bind:this={canvas}
  class="crt-effects-canvas"
  aria-hidden="true"
></canvas>

<style>
.crt-effects-canvas {
    position: fixed;
    top: 0;
    left: 0;
    width: 100vw;
    height: 100vh;
    pointer-events: none;
    z-index: 9999;
}
</style>
