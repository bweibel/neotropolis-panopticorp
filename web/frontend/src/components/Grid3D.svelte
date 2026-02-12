<script lang="ts">
  interface Props {
    horizonY?: number;      // Where the horizon sits (0-1, default 0.3)
    gridLines?: number;     // Number of horizontal lines
    verticalLines?: number; // Number of vertical lines
    color?: string;         // Grid line color
    animate?: boolean;      // Enable animation
    speed?: number;         // Animation speed (pixels per second)
    logoSize?: number;      // Logo size at convergence point
  }

  let {
    horizonY = 0.25,
    gridLines = 15,
    verticalLines = 12,
    color = '#EA691C',
    animate: enableAnimation = true,
    speed = 5,
    logoSize = 50
  }: Props = $props();

  let svg: SVGSVGElement;
  let horizontalLinesGroup: SVGGElement;

  // Generate vertical lines (converging to vanishing point)
  function generateVerticalLines(width: number, height: number, vanishY: number): string[] {
    const lines: string[] = [];
    const vanishX = width / 2;
    const bottomY = height;
    const spread = width * 1.5; // How wide the lines spread at bottom

    for (let i = 0; i <= verticalLines; i++) {
      const t = i / verticalLines;
      const bottomX = vanishX - spread / 2 + spread * t;
      lines.push(`M ${vanishX} ${vanishY} L ${bottomX} ${bottomY}`);
    }

    return lines;
  }

  // Generate horizontal lines (perspective scaled)
  function generateHorizontalLines(width: number, height: number, vanishY: number): { path: string; y: number }[] {
    const lines: { path: string; y: number }[] = [];
    const vanishX = width / 2;
    const spread = width * 1.5;

    for (let i = 0; i <= gridLines; i++) {
      // Exponential distribution for perspective effect
      const t = Math.pow(i / gridLines, 1.5);
      const y = vanishY + (height - vanishY) * t;

      // Calculate line width at this y position
      const perspectiveT = (y - vanishY) / (height - vanishY);
      const lineHalfWidth = (spread / 2) * perspectiveT;

      const x1 = vanishX - lineHalfWidth;
      const x2 = vanishX + lineHalfWidth;

      lines.push({
        path: `M ${x1} ${y} L ${x2} ${y}`,
        y
      });
    }

    return lines;
  }

  // SVG dimensions
  const width = 400;
  const height = 800;
  const vanishY = height * horizonY;

  const verticalPaths = generateVerticalLines(width, height, vanishY);
  const horizontalLines = generateHorizontalLines(width, height, vanishY);

  $effect(() => {
    if (!enableAnimation || !horizontalLinesGroup) return;

    // Animate the horizontal lines moving towards viewer
    const paths = horizontalLinesGroup.querySelectorAll('path');

    // Create continuous scrolling effect
    let offset = 0;
    const gridSpacing = (height - vanishY) / gridLines;

    const animationLoop = () => {
      offset = (offset + speed / 60) % gridSpacing;

      paths.forEach((path, i) => {
        const baseY = horizontalLines[i].y;
        const newY = baseY + offset;

        // Recalculate line width for new Y position
        const perspectiveT = Math.max(0, (newY - vanishY) / (height - vanishY));
        const lineHalfWidth = (width * 1.5 / 2) * perspectiveT;
        const vanishX = width / 2;

        if (newY <= height && newY >= vanishY) {
          path.setAttribute('d', `M ${vanishX - lineHalfWidth} ${newY} L ${vanishX + lineHalfWidth} ${newY}`);
          path.style.opacity = String(Math.min(1, perspectiveT * 2));
        }
      });

      requestAnimationFrame(animationLoop);
    };

    const frameId = requestAnimationFrame(animationLoop);

    return () => cancelAnimationFrame(frameId);
  });
</script>



  <svg
    bind:this={svg}
    class="grid-3d"
    viewBox="0 0 {width} {height}"
    preserveAspectRatio="xMidYMax slice"
    aria-hidden="true"
  >
    <!-- Vertical lines (static) -->
    <g class="vertical-lines">
      {#each verticalPaths as path}
        <path d={path} stroke={color} stroke-width="1" fill="none" />
      {/each}
    </g>

    <!-- Horizontal lines (animated) -->
    <g bind:this={horizontalLinesGroup} class="horizontal-lines">
      {#each horizontalLines as line}
        <path d={line.path} stroke={color} stroke-width="1" fill="none" />
      {/each}
    </g>

    <!-- Logo at convergence point -->
    <image
      href="/images/logo-primary.svg"
      x={width / 2 - logoSize / 2}
      y={vanishY - logoSize / 3}
      width={logoSize}
      height={logoSize * 0.67}
      class="convergence-logo"
    />
  </svg>

<style>
  .grid-3d {
    width: 100%;
    height: 100%;
    filter:
      drop-shadow(0 0 4px var(--color-primary, #EA691C))
      drop-shadow(0 0 8px rgba(234, 105, 28, 0.3));
  }

  .vertical-lines path,
  .horizontal-lines path {
    vector-effect: non-scaling-stroke;
  }
</style>
