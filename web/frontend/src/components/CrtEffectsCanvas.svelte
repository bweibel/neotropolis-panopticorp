<script lang="ts">
  interface Props {
    enabled?: boolean;
    scanlines?: boolean;
    scanlineSpeed?: number;
    noise?: boolean;
    noiseIntensity?: number;
    flicker?: boolean;
    flickerIntensity?: number;
    tint?: string;
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
  let ctx: CanvasRenderingContext2D | null = null;
  let animationId: number;
  let time = 0;

  // Scanline position (wraps around)
  let scanlineY = 0;

  function resize() {
    if (!canvas) return;
    canvas.width = window.innerWidth;
    canvas.height = window.innerHeight;
  }

  function drawScanlines(ctx: CanvasRenderingContext2D, width: number, height: number) {
    if (!scanlines) return;

    // Moving bright scanline
    scanlineY = (scanlineY + scanlineSpeed) % height;

    const gradient = ctx.createLinearGradient(0, scanlineY - 100, 0, scanlineY + 100);
    gradient.addColorStop(0, 'rgba(234, 105, 28, 0)');
    gradient.addColorStop(0.4, 'rgba(234, 105, 28, 0.03)');
    gradient.addColorStop(0.5, 'rgba(234, 105, 28, 0.06)');
    gradient.addColorStop(0.6, 'rgba(234, 105, 28, 0.03)');
    gradient.addColorStop(1, 'rgba(234, 105, 28, 0)');

    ctx.fillStyle = gradient;
    ctx.fillRect(0, 0, width, height);
  }

  function drawNoise(ctx: CanvasRenderingContext2D, width: number, height: number) {
    if (!noise) return;

    const imageData = ctx.getImageData(0, 0, width, height);
    const data = imageData.data;

    // Sparse noise - only update some pixels for performance
    const pixelCount = width * height;
    const noisePixels = Math.floor(pixelCount * noiseIntensity);

    for (let i = 0; i < noisePixels; i++) {
      const idx = Math.floor(Math.random() * pixelCount) * 4;
      const brightness = Math.random() * 50;
      data[idx] = brightness;     // R
      data[idx + 1] = brightness * 0.8; // G (slight orange tint)
      data[idx + 2] = brightness * 0.3; // B
      data[idx + 3] = Math.random() * 100; // A
    }

    ctx.putImageData(imageData, 0, 0);
  }

  function drawFlicker(ctx: CanvasRenderingContext2D, width: number, height: number) {
    if (!flicker) return;

    // Random flicker - occasional brightness changes
    if (Math.random() < 0.1) {
      const alpha = Math.random() * flickerIntensity;
      ctx.fillStyle = `rgba(234, 105, 28, ${alpha})`;
      ctx.fillRect(0, 0, width, height);
    }

    // Occasional horizontal glitch line
    if (Math.random() < 0.02) {
      const y = Math.random() * height;
      const lineHeight = 2 + Math.random() * 4;
      ctx.fillStyle = `rgba(234, 105, 28, ${0.1 + Math.random() * 0.1})`;
      ctx.fillRect(0, y, width, lineHeight);
    }
  }

  function render() {
    if (!ctx || !enabled) {
      animationId = requestAnimationFrame(render);
      return;
    }

    const { width, height } = canvas;

    // Clear
    ctx.clearRect(0, 0, width, height);

    // Draw effects
    drawScanlines(ctx, width, height);
    drawFlicker(ctx, width, height);
    drawNoise(ctx, width, height);

    time++;
    animationId = requestAnimationFrame(render);
  }

  $effect(() => {
    if (!canvas) return;

    ctx = canvas.getContext('2d');
    resize();

    window.addEventListener('resize', resize);
    render();

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
    width: 100%;
    height: 100%;
    pointer-events: none;
    z-index: 99;
  }
</style>