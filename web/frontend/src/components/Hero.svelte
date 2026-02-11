<script lang="ts">
  interface Props {
    title?: string;
    tagline?: string;
    backgroundImage?: string;
  }

  let {
    title = 'PANOPTICORP',
    tagline = 'Building a Safer Tomorrow Through Total Awareness',
    backgroundImage = '/images/placeholderbanner.png'
  }: Props = $props();
</script>

<section class="hero" style="--bg-image: url({backgroundImage})">
  <div class="hero-overlay"></div>
  <div class="hero-content">
    <div class="logo-slot">
      <img src="/images/logo-light.svg" alt="Panopticorp" class="logo" />
    </div>
    <h1 class="hero-title">{title}</h1>
    <p class="hero-tagline">{tagline}</p>
  </div>
</section>

<style>
  .hero {
    position: relative;
    min-height: 100vh;
    display: flex;
    align-items: center;
    justify-content: center;
    background-image: var(--bg-image);
    background-size: cover;
    background-position: center;
    background-repeat: no-repeat;
  }

  .hero-overlay {
    position: absolute;
    inset: 0;
    background: linear-gradient(
      to bottom,
      rgba(0, 0, 0, 0.7) 0%,
      rgba(0, 0, 0, 0.5) 50%,
      rgba(0, 0, 0, 0.8) 100%
    );
  }

  .hero-content {
    position: relative;
    z-index: 1;
    text-align: center;
    padding: 2rem;
    max-width: 800px;
  }

  .logo-slot {
    margin-bottom: 2rem;
  }

  .logo {
    height: 8rem;
    width: auto;
  }

  .hero-title {
    font-size: clamp(2rem, 8vw, 4rem);
    font-weight: 700;
    letter-spacing: 0.3em;
    margin: 0;
    color: var(--color-primary);
    animation: crt-glow 0.1s steps(10) infinite;
  }

  @keyframes crt-glow {
    0% {
      text-shadow:
        0 0 12px rgba(255, 176, 0, 0.4),
        0 0 22px rgba(255, 176, 0, 0.25),
        -1px 0 6px rgba(255, 100, 0, 0.22),
        1px 0 6px rgba(255, 220, 0, 0.2);
    }
    50% {
      text-shadow:
        0 0 12px rgba(255, 176, 0, 0.35),
        0 0 22px rgba(255, 176, 0, 0.2),
        -1px 0 6px rgba(255, 100, 0, 0.17),
        1px 0 6px rgba(255, 220, 0, 0.15);
    }
    100% {
      text-shadow:
        0 0 12px rgba(255, 176, 0, 0.4),
        0 0 22px rgba(255, 176, 0, 0.25),
        -1px 0 6px rgba(255, 100, 0, 0.22),
        1px 0 6px rgba(255, 220, 0, 0.2);
    }

  }

  .hero-tagline {
    font-size: clamp(0.75rem, 2vw, 1rem);
    letter-spacing: 0.2em;
    text-transform: uppercase;
    opacity: 0.8;
    margin-top: 1rem;
    color: var(--color-text);
    text-shadow: 0 1px 10px rgba(0, 0, 0, 0.5);
  }
</style>
