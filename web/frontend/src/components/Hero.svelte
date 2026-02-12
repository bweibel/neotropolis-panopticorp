<script lang="ts">
  import { typedText } from '$lib/actions/typedText';
  
  interface Props {
    title?: string;
    tagline?: string;
    backgroundImage?: string;
  }

  let {
    title = 'PANOPTICORP',
    tagline = 'Embracing surveillance today to Ensure a safer tomorrow',
    taglineAlt = '監視をいま受け入れ、より安全な明日へ',
    backgroundImage = '/images/nobackground.png'
  }: Props = $props();
</script>

<section class="hero" >
  <div class="hero-overlay"></div>
  <div class="hero-content">
    <h1 class="hero-wordmark"> <img src="/images/wordmark-primary.svg" alt="Panopticorp" class="logo" /></h1>
    <h2 class="hero-tagline" use:typedText>{tagline}</h2>
  </div>
</section>

<style>

  h1 img {
    max-width: 100%;
  }

  .hero-content {
    position: relative;
    z-index: 1;
    text-align: center;
    padding: 2rem;
  }

  .logo-slot {
    margin-bottom: 2rem;
  }

  .hero-title {
    font-family: "Ethnocentric";
    font-size: clamp(2rem, 8vw, 4rem);
    font-weight: 700;
    letter-spacing: 0.3em;
    margin: 0;
  }

  .hero-wordmark {
    will-change: filter;
    animation: crt-glow-logo-primary .1s  infinite;
  }

  .hero-tagline {
    text-transform: uppercase;
    margin-top: 1rem;
  }
</style>
