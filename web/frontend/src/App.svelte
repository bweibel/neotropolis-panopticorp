<script lang="ts">
  import { typedText } from '$lib/actions/typedText';
  import { textReveal } from '$lib/actions/textReveal';
  import Hero from './components/Hero.svelte';
  import Footer from './components/Footer.svelte';
  import ScanlinesOverlay from './components/ScanlinesOverlay.svelte';
  import CrtEffectsCanvas from './components/CrtEffectsCanvas.svelte';
  import Grid3D from './components/Grid3D.svelte';
  import CornerFrame from './components/CornerFrame.svelte';
</script>

<main>
<!-- <DecorationBox /> -->
 <CornerFrame class="hero">
   <Hero />
 </CornerFrame>
  <CornerFrame class="grid-decoration" padding=0>
    <Grid3D speed=4/>
  </CornerFrame>
   <CornerFrame class="mission"><div  use:textReveal={{ mode: 'lines', stagger: 100, duration: 1000, delay: 2000 }}>
    <p>At Panopticorp, we're building a new age of civic certainty. Our unsurpassed analytical capability takes the guessing out of Gaussian probability and the neuroses out of neural networks -- deus in machina, born of machine learning. If you want to make a secure society from scratch, you must first gather enough data.</p>
    <p>Panopticorp delivers integrated monitoring, predictive risk analysis, and operational coordination everywhere that people live, work, play, and move. We partner with law enforcement, but we don't like to think of ourselves as police – more like a big brother. We're here to protect and serve our clientele with the authority and care you need to finally take a deep breath and relax.</p>
    <p>From Portugal to Pax-Panama, the Pacific Aquicolonies to Petrozavodskgrad, the political to the personal: Panopticorp is there. Panopticorp is everywhere.</p>
    <h3>Pervasive. Persistent. Penetrative. Panopticorp.</h3>
  </div>
  </CornerFrame>
   <CornerFrame class="system-status">
     <Footer   />
   </CornerFrame>

</main>
<ScanlinesOverlay />
<CrtEffectsCanvas />