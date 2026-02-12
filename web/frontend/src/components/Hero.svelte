<script lang="ts">
  import { typedText } from '$lib/actions/typedText';
  
  interface Props {
    title?: string;
    tagline?: string;
    backgroundImage?: string;
  }

  let {
    title = 'PANOPTICORP',
    tagline = 'Embracing surveillance today',
    tagline2 = 'to Ensure a safer tomorrow',
    taglineAlt = '監視をいま受け入れ、より安全な明日へ',
    backgroundImage = '/images/nobackground.png'
  }: Props = $props();
</script>

<section class="hero" >
  <div class="hero-content">
    <h1 class="hero-wordmark"> <img src="/images/wordmark-primary.svg" alt="Panopticorp" class="logo" /></h1>
    <div class="hero-taglines">
      <h2 class="hero-tagline" use:typedText>{tagline}</h2>
      <h2 class="hero-tagline" use:typedText>{tagline2}</h2>

    </div>

  </div>
</section>

<style>

  .hero-content {
    display: flex;
    /* flex-wrap: wrap; */
    gap: 1rem;
    flex-basis: content;
  }

  .hero-title {
    font-size: clamp(2rem, 8vw, 4rem);
    letter-spacing: 0.3em;
    margin: 0;
  }
  
  .hero-wordmark {
    will-change: filter;
    animation: crt-glow-logo-primary .1s  infinite;
    margin: 0;
    flex-basis: 50rem;
    flex-grow: 1;
    align-self: center;
  }
  
  .hero-wordmark img {
    max-width: 100%;
    margin: 0;
  }


.hero-taglines {
    flex-basis: 50%;
    align-self: center;

  min-width: fit-content;
  font-size: clamp(1rem, 1.5vw, 2rem);
}

  .hero-tagline {
    line-height: 1;
    text-transform: uppercase;
    margin: 0rem;
  }
</style>
