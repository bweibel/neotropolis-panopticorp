<script lang="ts">
  import type { Snippet } from 'svelte';

  interface Props {
    children: Snippet;
    class?: string;
    cornerSize?: number;
    strokeWidth?: number;
    color?: string;
    padding?: number;
  }

  let {
    children,
    class: className = '',
    cornerSize = 20,
    strokeWidth = 2,
    color = '#EA691C',
    padding = 1
  }: Props = $props();
</script>

<div class="corner-frame {className}" style="--padding: {padding}rem;">
  <!-- Top-left corner ┌ -->
  <svg class="corner top-left" width={cornerSize} height={cornerSize} viewBox="0 0 {cornerSize} {cornerSize}">
    <path
      d="M 0 {cornerSize} L 0 0 L {cornerSize} 0"
      stroke={color}
      stroke-width={strokeWidth}
      fill="none"
      stroke-linecap="square"
    />
  </svg>

  <!-- Top-right corner ┐ -->
  <svg class="corner top-right" width={cornerSize} height={cornerSize} viewBox="0 0 {cornerSize} {cornerSize}">
    <path
      d="M 0 0 L {cornerSize} 0 L {cornerSize} {cornerSize}"
      stroke={color}
      stroke-width={strokeWidth}
      fill="none"
      stroke-linecap="square"
    />
  </svg>

  <!-- Bottom-left corner └ -->
  <svg class="corner bottom-left" width={cornerSize} height={cornerSize} viewBox="0 0 {cornerSize} {cornerSize}">
    <path
      d="M 0 0 L 0 {cornerSize} L {cornerSize} {cornerSize}"
      stroke={color}
      stroke-width={strokeWidth}
      fill="none"
      stroke-linecap="square"
    />
  </svg>

  <!-- Bottom-right corner ┘ -->
  <svg class="corner bottom-right" width={cornerSize} height={cornerSize} viewBox="0 0 {cornerSize} {cornerSize}">
    <path
      d="M {cornerSize} 0 L {cornerSize} {cornerSize} L 0 {cornerSize}"
      stroke={color}
      stroke-width={strokeWidth}
      fill="none"
      stroke-linecap="square"
    />
  </svg>

  <div class="content">
    {@render children()}
  </div>
</div>

<style>
  .corner-frame {
    position: relative;
    width: 100%;
    height: 100%;
  }

  .corner {
    position: absolute;
    pointer-events: none;
    filter:
      drop-shadow(0 0 4px var(--color-primary, #EA691C))
      drop-shadow(0 0 8px rgba(234, 105, 28, 0.4));
  }

  .corner.top-left {
    top: 0;
    left: 0;
  }

  .corner.top-right {
    top: 0;
    right: 0;
  }

  .corner.bottom-left {
    bottom: 0;
    left: 0;
  }

  .corner.bottom-right {
    bottom: 0;
    right: 0;
  }

  .content {
    width: 100%;
    height: 100%;
    padding: var(--padding);

  }
</style>
