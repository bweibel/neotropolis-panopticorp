<script lang="ts">
  import type { Snippet } from 'svelte';
  import Hero from './Hero.svelte';
  import Footer from './Footer.svelte';
  import ScanlinesOverlay from './ScanlinesOverlay.svelte';
  import CrtEffectsCanvas from './CrtEffectsCanvas.svelte';
  import Grid3D from './Grid3D.svelte';
  import CornerFrame from './CornerFrame.svelte';

  interface Props {
    children: Snippet;
    showGrid?: boolean;
    showHero?: boolean;
    showFooter?: boolean;
    gridSpeed?: number;
  }

  let {
    children,
    showGrid = true,
    showHero = true,
    showFooter = true,
    gridSpeed = 4
  }: Props = $props();
</script>

<main>
  {#if showHero}
    <CornerFrame class="hero">
      <Hero />
    </CornerFrame>
  {/if}

  {#if showGrid}
    <CornerFrame class="grid-decoration" padding={0}>
      <Grid3D speed={gridSpeed} />
    </CornerFrame>
  {/if}

  <CornerFrame class="content">
    {@render children()}
  </CornerFrame>

  {#if showFooter}
    <CornerFrame class="system-status">
      <Footer />
    </CornerFrame>
  {/if}
</main>

<ScanlinesOverlay />
<CrtEffectsCanvas />

<style>
  main {
    display: grid;
    grid-template-columns: 5rem auto 5rem;
    gap: 1rem;
    padding: 1rem;
    min-height: 100%;
    grid-template-areas:
      "hero hero hero"
      "content content content"
      "footer footer footer";
  }

  @media screen and (width >= 30rem) {
    main {
      grid-template-columns: repeat(6, minmax(5rem, 10rem));
      gap: 1rem;
      padding: 1rem;
      grid-template-areas:
        "hero hero hero hero hero hero"
        "decoration-1 content content content content content"
        "decoration-1 footer footer footer footer footer";
    }
  }

  @media screen and (width >= 72rem) {
    main {
      grid-template-columns: 1fr repeat(12, minmax(5rem, 10rem)) 1fr;
      grid-template-rows: 10rem auto auto 10rem;
      gap: 1rem;
      padding: 1rem;
      grid-template-areas:
        ". hero hero hero hero hero hero hero hero hero hero hero hero ."
        ". decoration-1 decoration-1 content content content content content content content content . . ."
        ". decoration-1 decoration-1 . . . . . . . . . . ."
        ". decoration-1 decoration-1 . . . . . . footer footer footer footer .";
    }
  }

  :global(.hero) {
    grid-area: hero;
    align-self: center;
  }

  :global(.content) {
    grid-area: content;
  }

  :global(.grid-decoration) {
    grid-column: 1/-1;
    grid-row: 1/-1;
    width: 100%;
    opacity: 0.3;
  }

  @media screen and (width >= 30rem) {
    :global(.grid-decoration) {
      grid-area: decoration-1;
      width: 100%;
      opacity: 1;
    }
  }

  @media screen and (width >= 72rem) {
    :global(.grid-decoration) {
      grid-area: decoration-1;
      width: 100%;
      opacity: 1;
    }
  }

  :global(.system-status) {
    grid-area: footer;
  }
</style>
