<script lang="ts">
  import { typedText } from '$lib/actions/typedText';
    import Wordmark from './Wordmark.svelte';
  
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
    <Wordmark />
    <div class="hero-taglines">
      <h2 class="hero-tagline" use:typedText>{tagline}</h2>
      <h2 class="hero-tagline" use:typedText>{tagline2}</h2>

    </div>

  </div>
</section>

<style>

  .hero-content {
    padding-block: 1rem;
    display: flex;
    flex-wrap: wrap;
    gap: 1rem;
    flex-basis: content;
  }

  .hero-title {
    font-size: clamp(2rem, 8vw, 4rem);
    letter-spacing: 0.3em;
    margin: 0;
  }
  



.hero-taglines {
    flex-basis: 50%;
    min-width: fit-content;
    height: 50%;
        align-self: center;
    font-size: clamp(.8rem, 1.5vw, 1.5rem);
}

  .hero-tagline {
    line-height: 1;
    text-transform: uppercase;
    margin: 0rem;
  }
</style>
