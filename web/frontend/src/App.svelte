<script lang="ts">
  import Hero from './components/Hero.svelte';
  import Footer from './components/Footer.svelte';
  import CrtOverlay from './components/CrtOverlay.svelte';
    import ScanlinesOverlay from './components/ScanlinesOverlay.svelte';
</script>

<main>
  <Hero />
  <div class="mission">
    <p>At Panopticorp, we're building a new age of civic certainty. Our unsurpassed analytical capability takes the guessing out of Gaussian probability and the neuroses out of neural networks -- deus in machina, born of machine learning. If you want to make a secure society from scratch, you must first gather enough data.</p>
    <p>Panopticorp delivers integrated monitoring, predictive risk analysis, and operational coordination everywhere that people live, work, play, and move. We partner with law enforcement, but we don’t like to think of ourselves as police – more like a big brother. We’re here to protect and serve our clientele with the authority and care you need to finally take a deep breath and relax.</p>
    <p>From Portugal to Pax-Panama, the Pacific Aquicolonies to Petrozavodskgrad, the political to the personal: Panopticorp is there. Panopticorp is everywhere.</p>
    <h3>Pervasive. Persistent. Penetrative. Panopticorp.</h3>
  </div>
  <Footer />
</main>
<ScanlinesOverlay />
<CrtOverlay />

<style>
  .mission {
    max-width: 60em;
    margin: auto;
  }

  .mission h3 {
    text-align: center;
  }
</style>